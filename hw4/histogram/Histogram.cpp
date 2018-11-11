#include "Histogram.hpp"

Histogram::Histogram(const char *filename) {
    img.load_bmp(filename);
    img.display("src");
}

void Histogram::toGrayScale() {
    // 根据 ITU-R BT.709 Luma 系数转变灰度图
    gray_img = CImg<unsigned char>(img.width(), img.height(), 1);
    cimg_forXY(img, x, y) {
        unsigned char r = img(x, y, 0);
        unsigned char g = img(x, y, 1);
        unsigned char b = img(x, y, 2);
        gray_img(x, y) = r * 0.2126 + g * 0.7152 + b * 0.0722;
    }
}

void Histogram::equalize() {
    toGrayScale();
    gray_img.display("GrayScale");
    run(gray_img);
    res.display("result");

    /* in rgb */
    // CImg<unsigned char> r = img.get_channel(0);
    // CImg<unsigned char> g = img.get_channel(1);
    // CImg<unsigned char> b = img.get_channel(2);
    // run(r);
    // CImg<unsigned char> c1(res);
    // run(g);
    // CImg<unsigned char> c2(res);
    // run(b);
    // CImg<unsigned char> c3(res);
    // res = CImg<unsigned char>(img.width(), img.height(), 1, 3);
    // cimg_forXY(res, x, y) {
    //     res(x, y, 0) = c1(x, y);
    //     res(x, y, 1) = c2(x, y);
    //     res(x, y ,2) = c3(x, y);
    // }

    ycrcb_img = ColorSpace::RGBtoYCbCr(img);
    run(ycrcb_img);
    res = ColorSpace::YCbCrtoRGB(res);
    res.display("result");
}

void Histogram::run(const CImg<unsigned char> &src) {
    res = CImg<unsigned char>(src);

    // 计算直方图
    double his[256] = {0};
    cimg_forXY(src, x, y)++ his[(int)src(x, y)];

    // 计算累积分布函数
    double total = img.width() * img.height();
    double sk[256] = {his[0] / total};
    for (int i = 1; i < 256; ++i) {
        sk[i] = sk[i - 1] + his[i] / total;
    }

    // 计算新灰度
    for (int i = 0; i < 256; ++i) {
        sk[i] = (int)(sk[i] * 255 + 0.5);
    }

    cimg_forXY(res, x, y) res(x, y) = sk[(int)src(x, y)];
}
