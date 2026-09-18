#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->comboIlluminant, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateAll);
    connect(ui->comboGamut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateAll);

    connect(ui->btnColorPicker, &QPushButton::clicked, this, &MainWindow::onSelectColor);

    connect(ui->sliderX, &QSlider::valueChanged, this, &MainWindow::onXYZChanged);
    connect(ui->sliderY, &QSlider::valueChanged, this, &MainWindow::onXYZChanged);
    connect(ui->sliderZ, &QSlider::valueChanged, this, &MainWindow::onXYZChanged);

    connect(ui->spinX, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderX->setValue(val);
                    onXYZChanged();
                }
            });
    connect(ui->spinY, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderY->setValue(val);
                    onXYZChanged();
                }
            });
    connect(ui->spinZ, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderZ->setValue(val);
                    onXYZChanged();
                }
            });

    connect(ui->sliderL, &QSlider::valueChanged, this, &MainWindow::onLABChanged);
    connect(ui->sliderA, &QSlider::valueChanged, this, &MainWindow::onLABChanged);
    connect(ui->sliderB_lab, &QSlider::valueChanged, this, &MainWindow::onLABChanged);

    connect(ui->spinL, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderL->setValue(val);
                    onLABChanged();
                }
            });
    connect(ui->spinA, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderA->setValue(val);
                    onLABChanged();
                }
            });
    connect(ui->spinB_lab, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderB_lab->setValue(val);
                    onLABChanged();
                }
            });

    connect(ui->sliderH, &QSlider::valueChanged, this, &MainWindow::onHLSChanged);
    connect(ui->sliderL_hls, &QSlider::valueChanged, this, &MainWindow::onHLSChanged);
    connect(ui->sliderS, &QSlider::valueChanged, this, &MainWindow::onHLSChanged);

    connect(ui->spinH, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderH->setValue(val);
                    onHLSChanged();
                }
            });
    connect(ui->spinL_hls, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderL_hls->setValue(val);
                    onHLSChanged();
                }
            });
    connect(ui->spinS, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val)
            {
                if (!isUpdating)
                {
                    ui->sliderS->setValue(val);
                    onHLSChanged();
                }
            });

    updateAll();
}

MainWindow::~MainWindow()
{
    delete ui;
}

Illuminant MainWindow::getIlluminant()
{
    return static_cast<Illuminant>(ui->comboIlluminant->currentIndex());
}

OutOfGamutStrategy MainWindow::getStrategy()
{
    return static_cast<OutOfGamutStrategy>(ui->comboGamut->currentIndex());
}

void MainWindow::onXYZChanged()
{
    if (isUpdating)
    {
        return;
    }

    XYZColor xyz{(double)ui->sliderX->value(), (double)ui->sliderY->value(), (double)ui->sliderZ->value()};
    RGBColor rgb;
    LABColor lab;
    HLSColor hls;

    controller.processXYZChange(xyz, getIlluminant(), getStrategy(), rgb, lab, hls);

    updateUI(rgb, xyz, lab, hls);
}

void MainWindow::onLABChanged()
{
    if (isUpdating)
    {
        return;
    }

    LABColor lab{(double)ui->sliderL->value(), (double)ui->sliderA->value(), (double)ui->sliderB_lab->value()};
    RGBColor rgb;
    XYZColor xyz;
    HLSColor hls;

    controller.processLABChange(lab, getIlluminant(), getStrategy(), rgb, xyz, hls);

    updateUI(rgb, xyz, lab, hls);
}

void MainWindow::onHLSChanged()
{
    if (isUpdating)
    {
        return;
    }

    HLSColor hls{(double)ui->sliderH->value(), (double)ui->sliderL_hls->value(), (double)ui->sliderS->value()};
    RGBColor rgb;
    XYZColor xyz;
    LABColor lab;

    controller.processHLSChange(hls, getIlluminant(), getStrategy(), rgb, xyz, lab);

    updateUI(rgb, xyz, lab, hls);
}

void MainWindow::onSelectColor()
{
    QColor c = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
    if (c.isValid())
    {
        RGBColor rgb{(double)c.red(), (double)c.green(), (double)c.blue()};
        XYZColor xyz = ColorMath::RGBtoXYZ(rgb, getIlluminant());
        LABColor lab = ColorMath::XYZtoLAB(xyz, getIlluminant());
        HLSColor hls = ColorMath::RGBtoHLS(rgb);

        updateUI(rgb, xyz, lab, hls);
    }
}

void MainWindow::updateAll()
{
    if (isUpdating)
    {
        return;
    }

    LABColor lab{(double)ui->sliderL->value(), (double)ui->sliderA->value(), (double)ui->sliderB_lab->value()};
    RGBColor rgb;
    XYZColor xyz;
    HLSColor hls;

    controller.processLABChange(lab, getIlluminant(), getStrategy(), rgb, xyz, hls);

    updateUI(rgb, xyz, lab, hls);
}

void MainWindow::updateUI(RGBColor rgb, XYZColor xyz, LABColor lab, HLSColor hls)
{
    isUpdating = true;

    ui->sliderX->setValue((int)xyz.x);
    ui->spinX->setValue((int)xyz.x);
    ui->sliderY->setValue((int)xyz.y);
    ui->spinY->setValue((int)xyz.y);
    ui->sliderZ->setValue((int)xyz.z);
    ui->spinZ->setValue((int)xyz.z);

    ui->sliderL->setValue((int)lab.l);
    ui->spinL->setValue((int)lab.l);
    ui->sliderA->setValue((int)lab.a);
    ui->spinA->setValue((int)lab.a);
    ui->sliderB_lab->setValue((int)lab.b);
    ui->spinB_lab->setValue((int)lab.b);

    ui->sliderH->setValue((int)hls.h);
    ui->spinH->setValue((int)hls.h);
    ui->sliderL_hls->setValue((int)hls.l);
    ui->spinL_hls->setValue((int)hls.l);
    ui->sliderS->setValue((int)hls.s);
    ui->spinS->setValue((int)hls.s);

    if (rgb.outOfRange)
    {
        ui->lblWarning->setText("Предупреждение: Цвет выходит за пределы охвата sRGB!");
    }
    else
    {
        ui->lblWarning->clear();
    }

    QColor color((int)rgb.r, (int)rgb.g, (int)rgb.b);
    ui->previewFrame->setStyleSheet(QString("background-color: %1; border: 1px solid black;").arg(color.name()));

    updateSliderGradients(xyz, lab, hls);

    isUpdating = false;
}

void MainWindow::updateSliderGradients(XYZColor xyz, LABColor lab, HLSColor hls)
{
    auto makeGradientStyle = [](const QString &stops)
    {
        return QString(
                   "QSlider::groove:vertical {"
                   "  border: 1px solid #a0a0a0;"
                   "  width: 14px;"
                   "  background: qlineargradient(x1:0, y1:1, x2:0, y2:0, %1);"
                   "  border-radius: 4px;"
                   "}"
                   "QSlider::handle:vertical {"
                   "  background: #ffffff;"
                   "  border: 2px solid #555555;"
                   "  height: 12px;"
                   "  margin-left: -3px;"
                   "  margin-right: -3px;"
                   "  border-radius: 5px;"
                   "}"
                   "QSlider::handle:vertical:hover {"
                   "  background: #f0f0f0;"
                   "  border: 2px solid #000000;"
                   "}"
                   ).arg(stops);
    };

    Illuminant ill = getIlluminant();
    OutOfGamutStrategy strat = getStrategy();

    auto getXyzColor = [&](double x, double y, double z)
    {
        RGBColor c = ColorMath::XYZtoRGB({x, y, z}, ill, strat);
        return QColor((int)c.r, (int)c.g, (int)c.b).name();
    };

    QString stopsX = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = (ui->sliderX->maximum() / 5.0) * i;
        stopsX += QString("stop:%1 %2%3").arg(i / 5.0).arg(getXyzColor(val, xyz.y, xyz.z)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderX->setStyleSheet(makeGradientStyle(stopsX));

    QString stopsY = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = (ui->sliderY->maximum() / 5.0) * i;
        stopsY += QString("stop:%1 %2%3").arg(i / 5.0).arg(getXyzColor(xyz.x, val, xyz.z)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderY->setStyleSheet(makeGradientStyle(stopsY));

    QString stopsZ = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = (ui->sliderZ->maximum() / 5.0) * i;
        stopsZ += QString("stop:%1 %2%3").arg(i / 5.0).arg(getXyzColor(xyz.x, xyz.y, val)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderZ->setStyleSheet(makeGradientStyle(stopsZ));

    auto getLabColor = [&](double lVal, double aVal, double bVal)
    {
        XYZColor tempXyz = ColorMath::LABtoXYZ({lVal, aVal, bVal}, ill);
        RGBColor c = ColorMath::XYZtoRGB(tempXyz, ill, strat);
        return QColor((int)c.r, (int)c.g, (int)c.b).name();
    };

    QString stopsL_lab = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = (ui->sliderL->maximum() / 5.0) * i;
        stopsL_lab += QString("stop:%1 %2%3").arg(i / 5.0).arg(getLabColor(val, lab.a, lab.b)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderL->setStyleSheet(makeGradientStyle(stopsL_lab));

    QString stopsA = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = ui->sliderA->minimum() + ((ui->sliderA->maximum() - ui->sliderA->minimum()) / 5.0) * i;
        stopsA += QString("stop:%1 %2%3").arg(i / 5.0).arg(getLabColor(lab.l, val, lab.b)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderA->setStyleSheet(makeGradientStyle(stopsA));

    QString stopsB = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = ui->sliderB_lab->minimum() + ((ui->sliderB_lab->maximum() - ui->sliderB_lab->minimum()) / 5.0) * i;
        stopsB += QString("stop:%1 %2%3").arg(i / 5.0).arg(getLabColor(lab.l, lab.a, val)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderB_lab->setStyleSheet(makeGradientStyle(stopsB));

    auto getHlsColor = [&](double hVal, double lVal, double sVal)
    {
        RGBColor c = ColorMath::HLStoRGB({hVal, lVal, sVal});
        return QColor((int)c.r, (int)c.g, (int)c.b).name();
    };

    QString stopsH = "";
    for (int i = 0; i <= 6; ++i)
    {
        double val = i * 60.0;
        stopsH += QString("stop:%1 %2%3").arg(i / 6.0).arg(getHlsColor(val, hls.l, hls.s)).arg(i == 6 ? "" : ", ");
    }
    ui->sliderH->setStyleSheet(makeGradientStyle(stopsH));

    QString stopsL_hls = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = i * 20.0;
        stopsL_hls += QString("stop:%1 %2%3").arg(i / 5.0).arg(getHlsColor(hls.h, val, hls.s)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderL_hls->setStyleSheet(makeGradientStyle(stopsL_hls));

    QString stopsS = "";
    for (int i = 0; i <= 5; ++i)
    {
        double val = i * 20.0;
        stopsS += QString("stop:%1 %2%3").arg(i / 5.0).arg(getHlsColor(hls.h, hls.l, val)).arg(i == 5 ? "" : ", ");
    }
    ui->sliderS->setStyleSheet(makeGradientStyle(stopsS));
}