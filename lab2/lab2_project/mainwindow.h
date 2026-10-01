#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "DirectoryScannerThread.h"
#include "ImageMetadata.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSelectFolder_clicked();
    void updateProgress(int processed, int total);
    void addTableRow(const ImageMetadata &meta);
    void scanFinished();
    void onRowSelected();

private:
    Ui::MainWindow *ui;
    DirectoryScannerThread *m_scannerThread = nullptr;
    QList<ImageMetadata> m_results;
};

#endif // MAINWINDOW_H