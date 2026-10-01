#include "ImageParser.h"
#include <QFileInfo>
#include <QtEndian>
#include <cmath>

uint16_t ImageParser::readUInt16BE(const char* data) {
    return qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data));
}
uint32_t ImageParser::readUInt32BE(const char* data) {
    return qFromBigEndian<uint32_t>(reinterpret_cast<const uchar*>(data));
}
uint16_t ImageParser::readUInt16LE(const char* data) {
    return qFromLittleEndian<uint16_t>(reinterpret_cast<const uchar*>(data));
}
uint32_t ImageParser::readUInt32LE(const char* data) {
    return qFromLittleEndian<uint32_t>(reinterpret_cast<const uchar*>(data));
}

ImageMetadata ImageParser::parseFile(const QString &filePath) {
    QFileInfo info(filePath);
    QString fileName = info.fileName();
    qint64 fileSize = info.size();

    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;

    if (fileSize < 10) {
        meta.isCorrupted = true;
        meta.errorMessage = "Файл слишком мал или пуст";
        return meta;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        meta.isCorrupted = true;
        meta.errorMessage = "Не удалось открыть файл (Ошибка I/O)";
        return meta;
    }

    char magic[12];
    qint64 bytesRead = file.peek(magic, 12);
    if (bytesRead < 4) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Недостаточно данных для сигнатуры";
        return meta;
    }

    // Проверка сигнатур (Magic Numbers)
    if (memcmp(magic, "\x89PNG\r\n\x1a\n", 8) == 0) {
        return parsePNG(file, fileSize, fileName, filePath);
    } else if (memcmp(magic, "\xFF\xD8", 2) == 0) {
        return parseJPEG(file, fileSize, fileName, filePath);
    } else if (memcmp(magic, "BM", 2) == 0) {
        return parseBMP(file, fileSize, fileName, filePath);
    } else if (memcmp(magic, "GIF87a", 6) == 0 || memcmp(magic, "GIF89a", 6) == 0) {
        return parseGIF(file, fileSize, fileName, filePath);
    } else if (memcmp(magic, "II\x2A\x00", 4) == 0 || memcmp(magic, "MM\x00\x2A", 4) == 0) {
        return parseTIFF(file, fileSize, fileName, filePath);
    } else if (static_cast<uchar>(magic[0]) == 0x0A) {
        return parsePCX(file, fileSize, fileName, filePath);
    }

    file.close();
    meta.isCorrupted = true;
    meta.errorMessage = "Неизвестный или подмененный формат файла";
    return meta;
}

// ---------------- PNG PARSER ----------------
ImageMetadata ImageParser::parsePNG(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath) {
    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;
    meta.formatType = "PNG";

    file.seek(8); // Пропускаем сигнатуру PNG
    bool hasIEND = false;

    while (!file.atEnd()) {
        QByteArray chunkHeader = file.read(8);
        if (chunkHeader.size() < 8) break;

        uint32_t length = readUInt32BE(chunkHeader.constData());
        QByteArray chunkType = chunkHeader.mid(4, 4);

        if (chunkType == "IHDR") {
            QByteArray data = file.read(13);
            if (data.size() < 13) {
                meta.isCorrupted = true;
                meta.errorMessage = "Чанк IHDR поврежден";
                file.close();
                return meta;
            }
            meta.width = readUInt32BE(data.constData());
            meta.height = readUInt32BE(data.constData() + 4);
            uint8_t bitDepth = static_cast<uint8_t>(data[8]);
            uint8_t colorType = static_cast<uint8_t>(data[9]);

            int channels = 1;
            if (colorType == 2) channels = 3;       // RGB
            else if (colorType == 4) channels = 2;  // Gray + Alpha
            else if (colorType == 6) channels = 4;  // RGBA

            meta.colorDepth = bitDepth * channels;
            meta.compression = "Deflate/Inflate (LZ77)";
            file.seek(file.pos() + length - 13 + 4); // Пропуск данных и CRC
        }
        else if (chunkType == "pHYs") {
            QByteArray data = file.read(length);
            if (data.size() >= 9) {
                uint32_t ppuX = readUInt32BE(data.constData());
                uint32_t ppuY = readUInt32BE(data.constData() + 4);
                uint8_t unit = static_cast<uint8_t>(data[8]);
                if (unit == 1) { // 1 = Пикселей на метр
                    meta.dpiX = std::round(ppuX * 0.0254);
                    meta.dpiY = std::round(ppuY * 0.0254);
                }
            }
            file.seek(file.pos() + 4); // CRC
        }
        else if (chunkType == "IEND") {
            hasIEND = true;
            break;
        } else {
            file.seek(file.pos() + length + 4); // Пропуск неопознанного чанка
        }
    }

    file.close();

    // Детекция битого файла
    if (!hasIEND) {
        meta.isCorrupted = true;
        meta.errorMessage = "Файл поврежден: отсутствует чанк IEND";
    }

    return meta;
}

// ---------------- JPEG PARSER ----------------
ImageMetadata ImageParser::parseJPEG(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath) {
    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;
    meta.formatType = "JPEG";

    // Проверка EOI маркерa FF D9 в самом конце
    file.seek(fileSize - 2);
    QByteArray eoi = file.read(2);
    if (eoi != "\xFF\xD9") {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Файл поврежден: отсутствует маркер EOI (FF D9)";
        return meta;
    }

    file.seek(2);
    bool sofFound = false;
    int dqtCount = 0;

    while (!file.atEnd()) {
        char markerBuf[2];
        if (file.read(markerBuf, 1) < 1) break;

        if (static_cast<uchar>(markerBuf[0]) != 0xFF) continue;

        if (file.read(markerBuf + 1, 1) < 1) break;
        uchar marker = static_cast<uchar>(markerBuf[1]);

        if (marker == 0xDA) break; // SOS (Start of Scan) - дальше растр

        char lenBytes[2];
        if (file.read(lenBytes, 2) < 2) break;
        uint16_t segLen = readUInt16BE(lenBytes) - 2;

        if (marker == 0xE0) { // APP0 (JFIF)
            QByteArray data = file.read(segLen);
            if (data.size() >= 12 && data.startsWith("JFIF")) {
                uint8_t units = static_cast<uint8_t>(data[7]);
                uint16_t xDensity = readUInt16BE(data.constData() + 8);
                uint16_t yDensity = readUInt16BE(data.constData() + 10);
                if (units == 1) { meta.dpiX = xDensity; meta.dpiY = yDensity; }
                else if (units == 2) { meta.dpiX = std::round(xDensity * 2.54); meta.dpiY = std::round(yDensity * 2.54); }
            }
        }
        else if (marker >= 0xC0 && marker <= 0xC2) { // SOF0, SOF1, SOF2
            QByteArray data = file.read(segLen);
            if (data.size() >= 6) {
                uint8_t precision = static_cast<uint8_t>(data[0]);
                meta.height = readUInt16BE(data.constData() + 1);
                meta.width = readUInt16BE(data.constData() + 3);
                uint8_t components = static_cast<uint8_t>(data[5]);
                meta.colorDepth = precision * components;
                sofFound = true;
            }
        }
        else if (marker == 0xDB) { // DQT (Матрицы квантования)
            file.seek(file.pos() + segLen);
            dqtCount++;
        } else {
            file.seek(file.pos() + segLen);
        }
    }

    file.close();

    if (!sofFound) {
        meta.isCorrupted = true;
        meta.errorMessage = "Файл поврежден: не найден заголовок SOF";
        return meta;
    }

    meta.compression = "JPEG (DCT Lossy)";
    meta.extraInfo["Матрицы квантования (DQT)"] = QString("Найдено таблиц: %1").arg(dqtCount);
    meta.extraInfo["Пояснение DQT"] = "Таблицы DQT задают коэффициент сжатия частотных коэффициентов DCT.";

    return meta;
}

// ---------------- BMP PARSER ----------------
ImageMetadata ImageParser::parseBMP(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath) {
    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;
    meta.formatType = "BMP";

    QByteArray fileHeader = file.read(14);
    if (fileHeader.size() < 14) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Заголовок BITMAPFILEHEADER поврежден";
        return meta;
    }

    uint32_t bfSize = readUInt32LE(fileHeader.constData() + 2);
    if (fileSize < bfSize) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Размер файла меньше указанного в заголовке BMP";
        return meta;
    }

    QByteArray infoHeader = file.read(40);
    if (infoHeader.size() < 40) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Заголовок BITMAPINFOHEADER поврежден";
        return meta;
    }

    meta.width = std::abs(static_cast<int32_t>(readUInt32LE(infoHeader.constData() + 4)));
    meta.height = std::abs(static_cast<int32_t>(readUInt32LE(infoHeader.constData() + 8)));
    meta.colorDepth = readUInt16LE(infoHeader.constData() + 14);
    uint32_t comp = readUInt32LE(infoHeader.constData() + 16);
    uint32_t xPpm = readUInt32LE(infoHeader.constData() + 24);
    uint32_t yPpm = readUInt32LE(infoHeader.constData() + 28);

    meta.dpiX = (xPpm > 0) ? std::round(xPpm * 0.0254) : 72.0;
    meta.dpiY = (yPpm > 0) ? std::round(yPpm * 0.0254) : 72.0;

    switch (comp) {
    case 0: meta.compression = "BI_RGB (Без сжатия)"; break;
    case 1: meta.compression = "BI_RLE8 (Run-Length Encoded 8-bit)"; break;
    case 2: meta.compression = "BI_RLE4 (Run-Length Encoded 4-bit)"; break;
    default: meta.compression = QString("Тип сжатия #%1").arg(comp); break;
    }

    file.close();
    return meta;
}

// ---------------- GIF PARSER ----------------
ImageMetadata ImageParser::parseGIF(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath) {
    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;
    meta.formatType = "GIF";

    file.seek(6);
    QByteArray lsd = file.read(7);
    if (lsd.size() < 7) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "GIF LSD заголовок поврежден";
        return meta;
    }

    meta.width = readUInt16LE(lsd.constData());
    meta.height = readUInt16LE(lsd.constData() + 2);
    meta.colorDepth = 8;
    meta.compression = "LZW (Lempel-Ziv-Welch)";

    uint8_t packed = static_cast<uint8_t>(lsd[4]);
    bool gctFlag = (packed & 0x80) != 0;
    int gctSize = 1 << ((packed & 0x07) + 1);

    meta.extraInfo["Палитра GIF (GCT)"] = gctFlag ? QString("%1 цветов").arg(gctSize) : "Отсутствует";

    file.close();
    return meta;
}

// ---------------- TIFF PARSER ----------------
ImageMetadata ImageParser::parseTIFF(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath) {
    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;
    meta.formatType = "TIFF";

    QByteArray header = file.read(8);
    if (header.size() < 8) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Заголовок TIFF поврежден";
        return meta;
    }

    bool isLittleEndian = (header[0] == 'I');
    auto read16 = [isLittleEndian](const char* p) { return isLittleEndian ? readUInt16LE(p) : readUInt16BE(p); };
    auto read32 = [isLittleEndian](const char* p) { return isLittleEndian ? readUInt32LE(p) : readUInt32BE(p); };

    uint32_t ifdOffset = read32(header.constData() + 4);
    file.seek(ifdOffset);

    char numEntriesBytes[2];
    if (file.read(numEntriesBytes, 2) < 2) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Ошибка чтения IFD структуры";
        return meta;
    }

    uint16_t numEntries = read16(numEntriesBytes);

    for (int i = 0; i < numEntries; ++i) {
        QByteArray entry = file.read(12);
        if (entry.size() < 12) break;

        uint16_t tag = read16(entry.constData());
        uint32_t val = read32(entry.constData() + 8);

        if (tag == 256) meta.width = val;        // ImageWidth
        else if (tag == 257) meta.height = val;   // ImageLength
        else if (tag == 258) meta.colorDepth = val * 3; // BitsPerSample
        else if (tag == 259) {                   // Compression
            if (val == 1) meta.compression = "None";
            else if (val == 5) meta.compression = "LZW";
            else if (val == 7) meta.compression = "JPEG";
            else meta.compression = QString("Code %1").arg(val);
        }
    }

    file.close();
    return meta;
}

// ---------------- PCX PARSER ----------------
ImageMetadata ImageParser::parsePCX(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath) {
    ImageMetadata meta;
    meta.filename = fileName;
    meta.filepath = filePath;
    meta.formatType = "PCX";

    QByteArray header = file.read(128);
    if (header.size() < 128) {
        file.close();
        meta.isCorrupted = true;
        meta.errorMessage = "Заголовок PCX поврежден";
        return meta;
    }

    uint8_t encoding = static_cast<uint8_t>(header[2]);
    uint8_t bpp = static_cast<uint8_t>(header[3]);
    uint16_t xmin = readUInt16LE(header.constData() + 4);
    uint16_t ymin = readUInt16LE(header.constData() + 6);
    uint16_t xmax = readUInt16LE(header.constData() + 8);
    uint16_t ymax = readUInt16LE(header.constData() + 10);
    uint16_t hdpi = readUInt16LE(header.constData() + 12);
    uint16_t vdpi = readUInt16LE(header.constData() + 14);
    uint8_t planes = static_cast<uint8_t>(header[65]);

    meta.width = xmax - xmin + 1;
    meta.height = ymax - ymin + 1;
    meta.dpiX = hdpi > 0 ? hdpi : 72;
    meta.dpiY = vdpi > 0 ? vdpi : 72;
    meta.colorDepth = bpp * planes;
    meta.compression = (encoding == 1) ? "RLE (Run-Length Encoding)" : "Uncompressed";

    file.close();
    return meta;
}