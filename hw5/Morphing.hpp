#ifndef MORPHING
#define MORPHING

#include <vector>
#include "CImg.h"

typedef std::pair<double, double> Point;
typedef std::pair<double, double> Vec;

using namespace cimg_library;

/**
 * 用于表示特征线
 */
struct Line {
    Point P, Q;
    Vec PQ, Per_PQ;
    double len;

    Line(double px = 0, double py = 0, double qx = 0, double qy = 0);

    // 得到 u
    double u(int x, int y);
    // 得到 v
    double v(int x, int y);
    // 得到 X'
    Point getPoint(double u, double v);
};

class Morphing {
   public:
    Morphing(const char *src_filename, const char *dst_filename,
             std::vector<Line> src_features, std::vector<Line> dst_features);

    void morph(int frame_count, double a = 1, double b = 2, double p = 0);

    // 生成 gif
    void genGIF(const char *filename);

   private:
    // 生成中间帧
    void genFrame(int now);

    // 插值得到 rgb
    void bilinear(const CImg<unsigned char> &img, Point X, unsigned char rgb[]);

    // 根据 X 和 u，v 获得该点在 l 的权重
    double getWeight(Line l, Point X, double u, double v);

    // 生成中间帧的特征线
    void lineInterpolation();

    CImg<unsigned char> src;
    CImg<unsigned char> dst;
    CImgList<unsigned char> frames;

    // 保存原图的特征线
    std::vector<Line> sf;
    // 保存目标图的特征线
    std::vector<Line> df;
    // 保存中间帧的特征线，frame_lines[i][j] 表示第 j 帧第 i 条特征线
    std::vector<std::vector<Line> > frame_lines;

    int fc;  // 帧数
    double _a;
    double _b;
    double _p;
};

#endif