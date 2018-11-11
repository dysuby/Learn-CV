#ifndef HISTOGRAM
#define HISTOGRAM

#include "../lib/CImg.h"
#include "../lib/ColorSpace.hpp"

using namespace cimg_library;

class Histogram {
   public:
    Histogram(const char *filename);

    void equalize();

   private:
    /**
     * 转成灰度图
     */
    void toGrayScale();

    /**
     * 核心步骤
     */
    void run(const CImg<unsigned char> &src);

    CImg<unsigned char> img;
    CImg<unsigned char> gray_img;
    CImg<unsigned char> ycrcb_img;
    CImg<unsigned char> res;
};

#endif