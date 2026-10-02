#ifndef IMAGEPARSER_H
#define IMAGEPARSER_H

#include "ImageMetadata.h"
#include <QFile>

class ImageParser {
public:
    static ImageMetadata parseFile(const QString &filePath);

private:
    static ImageMetadata parsePNG(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath);
    static ImageMetadata parseJPEG(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath);
    static ImageMetadata parseBMP(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath);
    static ImageMetadata parseGIF(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath);
    static ImageMetadata parseTIFF(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath);
    static ImageMetadata parsePCX(QFile &file, qint64 fileSize, const QString &fileName, const QString &filePath);


    static uint16_t readUInt16BE(const char* data);
    static uint32_t readUInt32BE(const char* data);
    static uint16_t readUInt16LE(const char* data);
    static uint32_t readUInt32LE(const char* data);
};

#endif // IMAGEPARSER_H