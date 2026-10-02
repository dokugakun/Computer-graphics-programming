#ifndef IMAGEMETADATA_H
#define IMAGEMETADATA_H

#include <QString>
#include <QMap>

struct ImageMetadata {
    QString filename;
    QString filepath;
    QString formatType = "Unknown";
    uint32_t width = 0;
    uint32_t height = 0;
    double dpiX = 72.0;
    double dpiY = 72.0;
    int colorDepth = 0;
    QString compression = "Unknown";
    bool isCorrupted = false;
    QString errorMessage = "OK";
    QMap<QString, QString> extraInfo;
};

#include <QMetaType>
Q_DECLARE_METATYPE(ImageMetadata)

#endif // IMAGEMETADATA_H