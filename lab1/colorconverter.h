#pragma once

#include <cmath>
#include <algorithm>

enum class Illuminant { D65, D50, E };

enum class OutOfGamutStrategy { Clamp, Scale };

struct RGBColor
{
    double r, g, b;
    bool outOfRange = false;
};

struct XYZColor
{
    double x, y, z;
};

struct LABColor
{
    double l, a, b;
};

struct HLSColor
{
    double h, l, s;
};

class ColorMath
{
public:

    static void getWhitePoint(Illuminant ill, double &Xn, double &Yn, double &Zn)
    {
        switch (ill)
        {
        case Illuminant::D50:
            Xn = 96.4212;
            Yn = 100.0;
            Zn = 82.5188;
            break;
        case Illuminant::E:
            Xn = 100.0;
            Yn = 100.0;
            Zn = 100.0;
            break;
        case Illuminant::D65:
        default:
            Xn = 95.047;
            Yn = 100.0;
            Zn = 108.883;
            break;
        }
    }

    static double Hue_2_RGB(double v1, double v2, double vH)
    {
        if (vH < 0.0)
        {
            vH += 1.0;
        }
        if (vH > 1.0)
        {
            vH -= 1.0;
        }

        if ((6.0 * vH) < 1.0)
        {
            return (v1 + (v2 - v1) * 6.0 * vH);
        }
        if ((2.0 * vH) < 1.0)
        {
            return (v2);
        }
        if ((3.0 * vH) < 2.0)
        {
            return (v1 + (v2 - v1) * ((2.0 / 3.0) - vH) * 6.0);
        }

        return v1;
    }

    static HLSColor RGBtoHLS(RGBColor rgb)
    {
        double var_R = rgb.r / 255.0;
        double var_G = rgb.g / 255.0;
        double var_B = rgb.b / 255.0;

        double var_Min = std::min({var_R, var_G, var_B});
        double var_Max = std::max({var_R, var_G, var_B});
        double del_Max = var_Max - var_Min;

        double H = 0.0;
        double S = 0.0;
        double L = (var_Max + var_Min) / 2.0;

        if (del_Max == 0.0)
        {
            H = 0.0;
            S = 0.0;
        }
        else
        {
            if (L < 0.5)
            {
                S = del_Max / (var_Max + var_Min);
            }
            else
            {
                S = del_Max / (2.0 - var_Max - var_Min);
            }

            double del_R = (((var_Max - var_R) / 6.0) + (del_Max / 2.0)) / del_Max;
            double del_G = (((var_Max - var_G) / 6.0) + (del_Max / 2.0)) / del_Max;
            double del_B = (((var_Max - var_B) / 6.0) + (del_Max / 2.0)) / del_Max;

            if (var_R == var_Max)
            {
                H = del_B - del_G;
            }
            else if (var_G == var_Max)
            {
                H = (1.0 / 3.0) + del_R - del_B;
            }
            else if (var_B == var_Max)
            {
                H = (2.0 / 3.0) + del_G - del_R;
            }

            if (H < 0.0)
            {
                H += 1.0;
            }
            if (H > 1.0)
            {
                H -= 1.0;
            }
        }

        return { H * 360.0, L * 100.0, S * 100.0 };
    }

    static RGBColor HLStoRGB(HLSColor hls)
    {
        double H = hls.h / 360.0;
        double L = hls.l / 100.0;
        double S = hls.s / 100.0;

        double R, G, B;

        if (S == 0.0)
        {
            R = L * 255.0;
            G = L * 255.0;
            B = L * 255.0;
        }
        else
        {
            double var_2;
            if (L < 0.5)
            {
                var_2 = L * (1.0 + S);
            }
            else
            {
                var_2 = (L + S) - (S * L);
            }

            double var_1 = 2.0 * L - var_2;

            R = 255.0 * Hue_2_RGB(var_1, var_2, H + (1.0 / 3.0));
            G = 255.0 * Hue_2_RGB(var_1, var_2, H);
            B = 255.0 * Hue_2_RGB(var_1, var_2, H - (1.0 / 3.0));
        }

        return { R, G, B, false };
    }

    static RGBColor XYZtoRGB(XYZColor xyz, Illuminant ill = Illuminant::D65, OutOfGamutStrategy strategy = OutOfGamutStrategy::Clamp)
    {
        double x = xyz.x / 100.0;
        double y = xyz.y / 100.0;
        double z = xyz.z / 100.0;

        double r =  3.2406 * x - 1.5372 * y - 0.4986 * z;
        double g = -0.9689 * x + 1.8758 * y + 0.0415 * z;
        double b =  0.0557 * x - 0.2040 * y + 1.0570 * z;

        auto gamma = [](double val)
        {
            return (val > 0.0031308) ? (1.055 * std::pow(val, 1.0 / 2.4) - 0.055) : (12.92 * val);
        };

        r = gamma(r) * 255.0;
        g = gamma(g) * 255.0;
        b = gamma(b) * 255.0;

        bool outOfRange = (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255);

        if (outOfRange)
        {
            if (strategy == OutOfGamutStrategy::Clamp)
            {
                r = std::clamp(r, 0.0, 255.0);
                g = std::clamp(g, 0.0, 255.0);
                b = std::clamp(b, 0.0, 255.0);
            }
            else if (strategy == OutOfGamutStrategy::Scale)
            {
                double minVal = std::min({r, g, b});
                if (minVal < 0)
                {
                    r -= minVal;
                    g -= minVal;
                    b -= minVal;
                }
                double maxVal = std::max({r, g, b});
                if (maxVal > 255.0)
                {
                    r = (r / maxVal) * 255.0;
                    g = (g / maxVal) * 255.0;
                    b = (b / maxVal) * 255.0;
                }
            }
        }

        return { r, g, b, outOfRange };
    }

    static XYZColor RGBtoXYZ(RGBColor rgb, Illuminant ill = Illuminant::D65)
    {
        double r = rgb.r / 255.0;
        double g = rgb.g / 255.0;
        double b = rgb.b / 255.0;

        auto invGamma = [](double val)
        {
            return (val > 0.04045) ? std::pow((val + 0.055) / 1.055, 2.4) : (val / 12.92);
        };

        r = invGamma(r);
        g = invGamma(g);
        b = invGamma(b);

        double x = (r * 0.4124 + g * 0.3576 + b * 0.1805) * 100.0;
        double y = (r * 0.2126 + g * 0.7152 + b * 0.0722) * 100.0;
        double z = (r * 0.0193 + g * 0.1192 + b * 0.9505) * 100.0;

        return { x, y, z };
    }

    static LABColor XYZtoLAB(XYZColor xyz, Illuminant ill = Illuminant::D65)
    {
        double Xn, Yn, Zn;
        getWhitePoint(ill, Xn, Yn, Zn);

        double x = xyz.x / Xn;
        double y = xyz.y / Yn;
        double z = xyz.z / Zn;

        auto f = [](double t)
        {
            return (t > 0.008856) ? std::pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);
        };

        double fx = f(x);
        double fy = f(y);
        double fz = f(z);

        double L = 116.0 * fy - 16.0;
        double A = 500.0 * (fx - fy);
        double B = 200.0 * (fy - fz);

        return { L, A, B };
    }

    static XYZColor LABtoXYZ(LABColor lab, Illuminant ill = Illuminant::D65)
    {
        double Xn, Yn, Zn;
        getWhitePoint(ill, Xn, Yn, Zn);

        double fy = (lab.l + 16.0) / 116.0;
        double fx = lab.a / 500.0 + fy;
        double fz = fy - lab.b / 200.0;

        auto invF = [](double t)
        {
            double t3 = std::pow(t, 3.0);
            return (t3 > 0.008856) ? t3 : ((t - 16.0 / 116.0) / 7.787);
        };

        double x = invF(fx) * Xn;
        double y = invF(fy) * Yn;
        double z = invF(fz) * Zn;

        return { x, y, z };
    }
};