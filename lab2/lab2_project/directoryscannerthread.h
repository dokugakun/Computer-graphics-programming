#ifndef DIRECTORYSCANNERTHREAD_H
#define DIRECTORYSCANNERTHREAD_H

#include <QThread>
#include <QDirIterator>
#include "ImageMetadata.h"

class DirectoryScannerThread : public QThread {
    Q_OBJECT
public:
    explicit DirectoryScannerThread(const QString &folderPath, QObject *parent = nullptr);
    void stop();

signals:
    void progressUpdated(int processed, int total);
    void itemParsed(const ImageMetadata &meta);
    void finishedScan();

protected:
    void run() override;

private:
    QString m_folderPath;
    bool m_isRunning = true;
};

#endif // DIRECTORYSCANNERTHREAD_H