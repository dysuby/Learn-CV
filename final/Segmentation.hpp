#ifndef __SEGMENTATION__
#define __SEGMENTATION__

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include <fstream>

#include "CImg.h"
#include "HoughLine.hpp"

using namespace cimg_library;

struct Point {
    int x, y;
    Point(int _x, int _y) : x(_x), y(_y) {}
};

typedef std::vector<std::pair<Point, Point> > PointPairs;

class Segmentation {
   public:
    Segmentation();

    void run(const char *filename, const char *save_path,
             const char *angle_path);

   private:
    void init();

    void toGray();

    /**
     * K-means 聚类
     */
    void kmeans(int k);

    /**
     * 检测边缘
     */
    void detect();

    /**
     * 计算角点变换后的点
     */
    void calMapPoint();

    /**
     * 高斯消元法解线性方程组
     */
    bool gauss_jordan(double A[8][8], int b[8], double H[8]);

    /**
     * 计算单应矩阵
     */
    bool calHomography(const PointPairs &samples, double H[9]);

    /**
     * 投影变换
     */
    void projective();

   private:
    CImg<unsigned char> raw;
    CImg<unsigned char> src;
    CImg<double> gray_img;
    CImg<unsigned char> clustered;
    CImg<unsigned char> detected;
    CImg<unsigned char> res;

    std::vector<double> centroid;
    std::vector<std::pair<int, int> > points;
    PointPairs pairs;

    CImg<int> mask;
};

#endif