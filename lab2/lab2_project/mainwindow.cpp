#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    qRegisterMetaType<ImageMetadata>("ImageMetadata");

    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setHorizontalHeaderLabels({
        "Имя файла", "Формат", "Размер (px)", "DPI", "Глубина цвета", "Сжатие", "Статус"
    });

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &MainWindow::onRowSelected);
}

MainWindow::~MainWindow() {
    if (m_scannerThread) {
        m_scannerThread->stop();
        m_scannerThread->wait();
    }
    delete ui;
}

void MainWindow::on_btnSelectFolder_clicked() {
    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку для сканирования");
    if (folder.isEmpty()) return;

    ui->tableWidget->setRowCount(0);
    m_results.clear();
    ui->btnSelectFolder->setEnabled(false);

    m_scannerThread = new DirectoryScannerThread(folder, this);
    connect(m_scannerThread, &DirectoryScannerThread::progressUpdated, this, &MainWindow::updateProgress);
    connect(m_scannerThread, &DirectoryScannerThread::itemParsed, this, &MainWindow::addTableRow);
    connect(m_scannerThread, &DirectoryScannerThread::finishedScan, this, &MainWindow::scanFinished);

    m_scannerThread->start();
}

void MainWindow::updateProgress(int processed, int total) {
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(processed);
    ui->lblStatus->setText(QString("Обработано: %1 / %2").arg(processed).arg(total));
}

void MainWindow::addTableRow(const ImageMetadata &meta) {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    m_results.append(meta);

    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(meta.filename));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(meta.formatType));

    if (meta.isCorrupted) {
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem("-"));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem("-"));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem("-"));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem("-"));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(meta.errorMessage));
    } else {
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString("%1 x %2").arg(meta.width).arg(meta.height)));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString("%1 x %2").arg(meta.dpiX).arg(meta.dpiY)));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString("%1 bit").arg(meta.colorDepth)));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(meta.compression));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem("OK"));
    }
}

void MainWindow::scanFinished() {
    ui->btnSelectFolder->setEnabled(true);
    ui->lblStatus->setText("Сканирование завершено.");
}

void MainWindow::onRowSelected() {
    int row = ui->tableWidget->currentRow();
    if (row < 0 || row >= m_results.size()) return;

    const ImageMetadata &meta = m_results[row];

    if (!meta.isCorrupted) {
        QPixmap pix(meta.filepath);
        if (!pix.isNull()) {
            ui->lblPreview->setPixmap(pix.scaled(ui->lblPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            ui->lblPreview->setText("Превью недоступно");
        }
    } else {
        ui->lblPreview->setText("Битый файл");
    }

    QString html = QString("<b>Имя:</b> %1<br><b>Путь:</b> %2<br>").arg(meta.filename, meta.filepath);
    if (meta.isCorrupted) {
        html += QString("<br><font color='red'><b>Ошибка:</b> %1</font>").arg(meta.errorMessage);
    } else {
        html += QString("<b>Формат:</b> %1<br><b>Размеры:</b> %2x%3 px<br><b>DPI:</b> %4x%5<br><b>Глубина:</b> %6 bit<br><b>Сжатие:</b> %7<hr>")
                    .arg(meta.formatType).arg(meta.width).arg(meta.height)
                    .arg(meta.dpiX).arg(meta.dpiY).arg(meta.colorDepth).arg(meta.compression);

        for (auto it = meta.extraInfo.begin(); it != meta.extraInfo.end(); ++it) {
            html += QString("• <b>%1:</b> %2<br>").arg(it.key(), it.value());
        }
    }

    ui->txtDetails->setHtml(html);
}