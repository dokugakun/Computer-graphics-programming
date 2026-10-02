#include "DirectoryScannerThread.h"
#include "ImageParser.h"
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>

DirectoryScannerThread::DirectoryScannerThread(const QString &folderPath, QObject *parent)
    : QThread(parent), m_folderPath(folderPath) {}

void DirectoryScannerThread::stop() {
    m_isRunning = false;
}

void DirectoryScannerThread::run() {
    QStringList files;
    QDirIterator it(m_folderPath, QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif" << "*.tif" << "*.tiff" << "*.pcx",
                    QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext() && m_isRunning) {
        files.append(it.next());
    }

    int total = files.size();
    if (total == 0) {
        emit finishedScan();
        return;
    }

    int processed = 0;


    QtConcurrent::blockingMap(files, [this, &processed, total](const QString &filePath) {
        if (!m_isRunning) return;

        ImageMetadata meta = ImageParser::parseFile(filePath);
        emit itemParsed(meta);

#pragma omp atomic
        processed++;

        if (processed % 10 == 0 || processed == total) {
            emit progressUpdated(processed, total);
        }
    });

    emit finishedScan();
}