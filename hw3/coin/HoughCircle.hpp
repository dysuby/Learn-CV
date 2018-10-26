#ifndef HOUGHCIRCLE
#define HOUGHCIRCLE

#include "../lib/CImg.h"
#include "../lib/canny.hpp"

using namespace cimg_library;

// 一个圆拥有的参数，(a, b)为圆心，r 为半径，votes_num 为该点的投票数
struct args {
    int a, b, r, votes_num;
};

class HoughCircle {
   public:
    HoughCircle(const char *filename);

    /**
     * 初始化三角函数表
     */
    void initTable();

    /**
     * 算法的主体
     */
    void hough(int rmin, int rmax, int thresh);

    /**
     * 对某一 (x0, y0) 固定半径，投票
     */
    void vote(int x, int y, int r);

    /**
     * 将 Hough空间内大于阈值的点视为有效
     */
    void threshold(int thresh, int r);

    /**
     * 移去重叠的圆
     */
    void removeNeighbor();

    /**
     * 根据圆心，半径画出圆
     */
    void displayCircle(int a, int b, int r);

    /**
     * 根据圆画出边缘
     */
    void displayEdge();

    /**
     * 保存结果
     */
    void save(const char *filename);

   private:
    CImg<double> raw;   // 初始边缘图
    CImg<double> edge;  // 去除无用边缘后的边缘图
    CImg<double> res;   // 结果图
    CImg<int> votes;    // 投票空间

    vector<double> sint;   // sin 表
    vector<double> cost;   // cos 表
    vector<args> circles;  // 圆集合
};

#endif
