#ifndef HOUGHLINE
#define HOUGHLINE

#include "../lib/CImg.h"
#include "../lib/canny.hpp"

using namespace cimg_library;

class HoughLine {
   public:
    HoughLine(const char *filename);

    /**
     * 初始化三角函数表
     */
    void initTable();

    /**
     * 算法的主体
     */
    void hough(int thresh, int angle_step, int rho_step);

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
     * 根据直线方程画出边缘点
     */
    void displayEdgePoint();

    /**
     * 根据直线方程画出白纸顶点
     */
    void displayCircle();

    /**
     * 保存结果
     */
    void save(const char *filename);

   private:
    CImg<double> raw;               // 初始边缘图
    CImg<double> edge;              // 去除无用边缘后的边缘图
    CImg<double> res;               // 结果图
    CImg<int> votes;                // 投票空间
    int angleRange;                 // theta 的范围
    int rhoRange;                   // rho 的范围
    vector<double> sint;            // sin 表
    vector<double> cost;            // cos 表
    vector<pair<int, int> > lines;  // 直线集合
};

#endif
