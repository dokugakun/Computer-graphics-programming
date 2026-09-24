#pragma once

#include <QMainWindow>
#include "controller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onXYZChanged();
    void onLABChanged();
    void onHLSChanged();
    void onSelectColor();
    void updateAll();

private:
    Ui::MainWindow *ui;
    ColorController controller;
    bool isUpdating = false;

    Illuminant getIlluminant();
    OutOfGamutStrategy getStrategy();

    void updateUI(RGBColor rgb, XYZColor xyz, LABColor lab, HLSColor hls);
    void updateSliderGradients(XYZColor xyz, LABColor lab, HLSColor hls);
};