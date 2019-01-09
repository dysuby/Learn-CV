#ifndef __DIGIT__
#define __DIGIT__
#include <vector>
#include <queue>
#include <set>
#include "CImg.h"
#include "Segmentation.hpp"
#include "canny.hpp"

using namespace cimg_library;
using namespace std;

class Digit {
   public:
    Digit();

    void run(const char *filename, const char *digitpath);
    void save(const char *);

   private:
    void init();
    int calboarder(int begin, int ord, int factor);
    void dilation();
    void finddigits();
    void resize();

    CImg<unsigned char> src;
    CImg<unsigned char> dilated;
    vector<CImg<unsigned char> > digits;
};

#endif