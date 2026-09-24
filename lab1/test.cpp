#include "test.h"
#include "colorconverter.h"

#include <cassert>
#include <cmath>
#include <iostream>

void Tests::runAll()
{
    {
        RGBColor red{255.0, 0.0, 0.0};
        HLSColor hls = ColorMath::RGBtoHLS(red);

        assert(std::round(hls.h) == 0.0);
        assert(std::round(hls.l) == 50.0);
        assert(std::round(hls.s) == 100.0);
    }

    {
        HLSColor hls{0.0, 50.0, 100.0};
        RGBColor rgb = ColorMath::HLStoRGB(hls);

        assert(std::round(rgb.r) == 255.0);
        assert(std::round(rgb.g) == 0.0);
        assert(std::round(rgb.b) == 0.0);
    }

    {
        RGBColor red{255.0, 0.0, 0.0};
        XYZColor xyz = ColorMath::RGBtoXYZ(red, Illuminant::D65);
        LABColor lab = ColorMath::XYZtoLAB(xyz, Illuminant::D65);

        assert(std::abs(lab.l - 53.24) < 1.0);
        assert(std::abs(lab.a - 80.09) < 1.0);
        assert(std::abs(lab.b - 67.20) < 1.0);
    }

    {
        RGBColor white{255.0, 255.0, 255.0};
        HLSColor hls = ColorMath::RGBtoHLS(white);

        assert(std::round(hls.l) == 100.0);
        assert(std::round(hls.s) == 0.0);
    }

    std::cout << "[SUCCESS] All tests passed successfully!" << std::endl;
}