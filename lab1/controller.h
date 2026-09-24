#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "colorconverter.h"

class ColorController : public QObject
{
    Q_OBJECT
public:
    explicit ColorController(QObject *parent = nullptr) : QObject(parent) {}

    void processXYZChange(XYZColor xyz, Illuminant ill, OutOfGamutStrategy strat,
                          RGBColor &outRgb, LABColor &outLab, HLSColor &outHls)
    {
        outRgb = ColorMath::XYZtoRGB(xyz, ill, strat);
        outLab = ColorMath::XYZtoLAB(xyz, ill);
        outHls = ColorMath::RGBtoHLS(outRgb);
    }

    void processLABChange(LABColor lab, Illuminant ill, OutOfGamutStrategy strat,
                          RGBColor &outRgb, XYZColor &outXyz, HLSColor &outHls)
    {
        outXyz = ColorMath::LABtoXYZ(lab, ill);
        outRgb = ColorMath::XYZtoRGB(outXyz, ill, strat);
        outHls = ColorMath::RGBtoHLS(outRgb);
    }

    void processHLSChange(HLSColor hls, Illuminant ill, OutOfGamutStrategy strat,
                          RGBColor &outRgb, XYZColor &outXyz, LABColor &outLab)
    {
        outRgb = ColorMath::HLStoRGB(hls);
        outXyz = ColorMath::RGBtoXYZ(outRgb, ill);
        outLab = ColorMath::XYZtoLAB(outXyz, ill);
    }
};

#endif // CONTROLLER_H
