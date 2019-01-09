#ifndef HOUGHLINE
#define HOUGHLINE

#include "CImg.h"
#include <vector>

using namespace cimg_library;

class HoughLine {
   public:
    HoughLine(const CImg<unsigned char> &src);

    /**
     * 初始化三角函数表
     */
    void initTable();

    /**
     * 算法的主体
     */
    std::vector<std::pair<int, int> > hough(int thresh, int angle_step,
                                            int rho_step);

    /**
     * 对某一 (x0, y0) 投票
     */
    void vote(int x, int y);

    /**
     * 将
     * Hough空间划分成固定大小的区域，每个区域选出投票数最大的点，若该点大于阈值则有效
     */
    void threshold(int thresh, int angle_step, int rho_step);

    /**
     * 根据直线方程画直线
     */
    void displayLine(int theta, int r);

    /**
     * 根据直线方程画出白纸顶点
     */
    void displayCircle();

   private:
    CImg<double> edge;              // 去除无用边缘后的边缘图
    CImg<double> res;               // 结果图
    CImg<int> votes;                // 投票空间
    int angleRange;                 // theta 的范围
    int rhoRange;                   // rho 的范围
    std::vector<double> sint;            // sin 表
    std::vector<double> cost;            // cos 表
    std::vector<std::pair<int, int> > lines;  // 直线集合
    std::vector<std::pair<int, int> > points;  // 直线集合
};

#endif
