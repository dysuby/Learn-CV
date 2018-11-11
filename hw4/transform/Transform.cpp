#include "Transform.hpp"

Transform::Transform(const char *filename) {
    img.load_bmp(filename);
}

void Transform::compute(const CImg<double> &tar, double mean[], double var[]) {
    int total = tar.width() * tar.height();
    
    // 计算均值
    cimg_forXY(tar, x, y) {
        mean[0] += tar(x, y, 0);
        mean[1] += tar(x, y, 1);
        mean[2] += tar(x, y, 2);
    }
    mean[0] /= total;
    mean[1] /= total;
    mean[2] /= total;

    // 计算方差
    cimg_forXY(tar, x, y) {
        var[0] += pow(tar(x, y, 0) - mean[0], 2);
        var[1] += pow(tar(x, y, 1) - mean[1], 2);
        var[2] += pow(tar(x, y, 2) - mean[2], 2);
    }
    var[0] /= total;
    var[1] /= total;
    var[2] /= total;
}

void Transform::transform(const char *filename) {
    CImg<double> dst(filename);
    img.display("src");
    dst.display("dst");

    // 转成 LAB 空间
    lab_img = ColorSpace::RGBtoLAB(img);
    res = CImg<double>(lab_img);
    dst = ColorSpace::RGBtoLAB(dst);

    // 分别计算原图和目标图的均值和方差
    double dst_mean[3] = {0}, dst_var[3] = {0}, src_mean[3] = {0},
           src_var[3] = {0};
    compute(dst, dst_mean, dst_var);
    compute(lab_img, src_mean, src_var);

    // 颜色转换
    cimg_forXY(res, x, y) {
        res(x, y, 0) =
            (dst_var[0] / src_var[0]) * (res(x, y, 0) - src_mean[0]) +
            dst_mean[0];
        res(x, y, 1) =
            (dst_var[1] / src_var[1]) * (res(x, y, 1) - src_mean[1]) +
            dst_mean[1];
        res(x, y, 2) =
            (dst_var[2] / src_var[2]) * (res(x, y, 2) - src_mean[2]) +
            dst_mean[2];
    }

    // 转回 RGB 空间
    res = ColorSpace::LABtoRGB(res);
    res.display("res");
}
