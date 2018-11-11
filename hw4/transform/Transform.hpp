#ifndef TRANSFORM
#define TRANSFORM

#include "../lib/CImg.h"
#include "../lib/ColorSpace.hpp"

using namespace cimg_library;

class Transform {
   public:
    Transform(const char *filename);

    /**
     * 根据目标图转换颜色
     * filename - 目标图路径
     */
    void transform(const char *filename);

   private:
    /**
     * 计算均值和方差
     */
    void compute(const CImg<double> &tar, double mean[], double var[]);

    CImg<double> img;
    CImg<double> lab_img;
    CImg<double> res;
};

#endif