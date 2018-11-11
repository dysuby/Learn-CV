#ifndef COLORSPACE
#define COLORSPACE

#include "CImg.h"

using namespace cimg_library;

class ColorSpace {
   public:
    /**
     * RGB => XYZ => LAB
     */
    static CImg<double> RGBtoLAB(const CImg<double> &src) {
        CImg<double> ret(src.width(), src.height(), 1, 3);
        const double xyz[3][3] = {{0.412453, 0.357580, 0.180423},
                                  {0.212671, 0.715160, 0.072169},
                                  {0.019334, 0.119193, 0.950227}};

#define f(t)                       \
    (((t) > pow(0.20689655172, 3)) \
         ? pow((t), 0.33333333333) \
         : ((t) / (3 * pow(0.20689655172, 2)) + 0.13793103448))
        const double xn = 95.047, yn = 100, zn = 108.883;
        cimg_forXY(ret, i, j) {
            double r = src(i, j, 0), g = src(i, j, 1), b = src(i, j, 2);
            double x = xyz[0][0] * r + xyz[0][1] * g + xyz[0][2] * b,
                   y = xyz[1][0] * r + xyz[1][1] * g + xyz[1][2] * b,
                   z = xyz[2][0] * r + xyz[2][1] * g + xyz[2][2] * b;

            ret(i, j, 0) = 116 * f(y / yn) - 16;
            ret(i, j, 1) = 500 * (f(x / xn) - f(y / yn));
            ret(i, j, 2) = 200 * (f(y / yn) - f(z / zn));
        }
#undef f

        return ret;
    }

    /**
     * LAB => XYZ => RGB
     */
    static CImg<double> LABtoRGB(const CImg<double> &src) {
        CImg<double> ret(src.width(), src.height(), 1, 3);
        const double xyz[3][3] = {{3.240479, -1.53715, -0.498535},
                                  {-0.969256, 1.875991, 0.041556},
                                  {0.055648, -0.204043, 1.057311}};
#define f(t)               \
    (((t) > 0.20689655172) \
         ? pow((t), 3)     \
         : (3 * pow(0.20689655172, 2) * ((t)-0.13793103448)))
        const double xn = 95.047, yn = 100, zn = 108.883;
        cimg_forXY(ret, i, j) {
            double l = src(i, j, 0), a = src(i, j, 1), b = src(i, j, 2);
            double x = xn * f((l + 16) / 116 + a / 500),
                   y = yn * f((l + 16) / 116),
                   z = zn * f((l + 16) / 116 - b / 200);
            for (int k = 0; k < 3; ++k)
                ret(i, j, k) = cimg::cut(
                    xyz[k][0] * x + xyz[k][1] * y + xyz[k][2] * z, 0, 255);
        }
#undef f

        return ret;
    }

    static CImg<double> RGBtoYCbCr(const CImg<double> &src) {
        CImg<double> ret(src.width(), src.height(), 1, 3);
        cimg_forXY(src, x, y) {
            double R = src(x, y, 0), G = src(x, y, 1), B = src(x, y, 2);
            ret(x, y, 0) = cimg::cut(
                (0.256789 * R + 0.504129 * G + 0.097906 * B) + 16, 0, 255);
            ret(x, y, 1) = cimg::cut(
                (-0.148223 * R - 0.290992 * G + 0.439215 * B) + 128, 0, 255);
            ret(x, y, 2) = cimg::cut(
                (0.439215 * R - 0.367789 * G - 0.071426 * B) + 128, 0, 255);
        }
        return ret;
    }

    static CImg<double> YCbCrtoRGB(const CImg<double> &src) {
        CImg<double> ret(src.width(), src.height(), 1, 3);
        cimg_forXY(src, x, y) {
            double Y = src(x, y, 0), Cb = src(x, y, 1), Cr = src(x, y, 2);
            ret(x, y, 0) =
                cimg::cut(1.164383 * (Y - 16) + 1.596027 * (Cr - 128), 0, 255);
            ret(x, y, 1) =
                cimg::cut(1.164383 * (Y - 16) - 0.391762 * (Cb - 128) -
                              0.812969 * (Cr - 128),
                          0, 255);
            ret(x, y, 2) =
                cimg::cut(1.164383 * (Y - 16) + 2.017230 * (Cb - 128), 0, 255);
        }
        return ret;
    }
};

// CImg<double> ColorSpace::RGBtoLAB(const CImg<double> &src) {
// if (src._spectrum != 3) {  // Number of colour channels
//     cerr << "RGB2Lab(): Instance is not a RGB image." << endl;
// }
// CImg<float> Labimg(src._width, src._height, 1, 3,
//                    0);  // w,h,depth,spectrum,initVal
// float R, G, B, L, M, S, l, alpha, beta;
// cimg_forXY(src, x, y) {
//     R = std::max(1.0 * src(x, y, 0) / 255,
//                  1.0 / 255);  // must be 1.0f, or 1/255 will be zero!!
//     G = std::max(1.0 * src(x, y, 1) / 255, 1.0 / 255);
//     B = std::max(1.0 * src(x, y, 2) / 255, 1.0 / 255);
//     // RGB -> LMS
//     L = 0.3811f * R + 0.5783f * G + 0.0402f * B;
//     M = 0.1967f * R + 0.7244f * G + 0.0782f * B;
//     S = 0.0241f * R + 0.1288f * G + 0.8444f * B;
//     // Convert the data to logarithmic space
//     L = log10(L);  // log(x), x > 0
//     M = log10(M);
//     S = log10(S);
//     // LMS -> Lab
//     l = 1.0 / sqrt(3) * L + 1.0 / sqrt(3) * M + 1.0 / sqrt(3) * S;
//     alpha = 1.0 / sqrt(6) * L + 1.0 / sqrt(6) * M - 2 / sqrt(6) * S;
//     beta = 1.0 / sqrt(2) * L - 1.0 / sqrt(2) * M + 0 * S;
//     Labimg(x, y, 0) = l;
//     Labimg(x, y, 1) = alpha;
//     Labimg(x, y, 2) = beta;
// }
// return Labimg;
// }

// CImg<double> ColorSpace::LABtoRGB(const CImg<double> &src) {
// CImg<double> ret(src.width(), src.height(), 1, 3);
// cimg_forXY(src, x, y) {
//     double L, M, S, l = src(x, y, 0), alpha = src(x, y, 1),
//                     beta = src(x, y, 2);
//     L = sqrt(3.0) / 3.0 * l + sqrt(6) / 6.0 * alpha +
//         sqrt(2) / 2.0 * beta;
//     M = sqrt(3.0) / 3.0 * l + sqrt(6) / 6.0 * alpha -
//         sqrt(2) / 2.0 * beta;
//     S = sqrt(3.0) / 3.0 * l - sqrt(6) / 3.0 * alpha - 0 * beta;
//     L = pow(10.0, L);
//     M = pow(10.0, M);
//     S = pow(10.0, S);
//     ret(x, y, 0) = 4.4679 * L - 3.5873 * M + 0.1193 * S;
//     ret(x, y, 1) = -1.2186 * L + 2.3809 * M - 0.1624 * S;
//     ret(x, y, 2) = 0.0497 * L - 0.2439 * M + 1.2045 * S;
// }

// return ret;
// }

#endif