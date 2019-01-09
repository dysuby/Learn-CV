#ifndef CANNY
#define CANNY

#include <string>
#include <vector>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

class Canny {
   public:
    Canny(string _filename);

    Canny(string _filename, double _sigma, int _scale, int _th, int _tl);
    
    Canny(const CImg<unsigned char> &src);

    /* 运行 canny 算法*/
    CImg<double> canny();

    /* 转成灰度图 */
    CImg<double> toGrayScale();

    /* 高斯滤波
     * @sigma - 正态分布标准差
     * @scale - 模糊半径
     */
    CImg<double> gauss();

    /* 计算梯度 */
    CImg<double> findLoG();

    /* 非极大值抑制
     *  @gradx - x 方向的梯度
     *  @grady - y 方向的梯度
     */
    CImg<double> nms(CImg<double> gradx, CImg<double> grady);

    /* 滞后阈值
     * @th - 高阈值
     * @tl - 低阈值
     */
    CImg<double> doubleThreshold();

    /* 将相邻的边缘连接 */
    CImg<double> linking();

    /* 删除一定长度一下的线
     *  @threshold - 小于 threshold 的被删除
     */
    CImg<double> deleteLine(int threshold);

    /* 保存图片的路径 */
    static const string GRAY_PATH;
    static const string SMOOTH_PATH;
    static const string GRADIENT_PATH;
    static const string NMS_PATH;
    static const string EDGE_PATH;
    static const string LINKED_PATH;
    static const string DELETED_PATH;

   private:
    CImg<double> img;
    CImg<double> gray_img;
    CImg<double> smooth_img;
    CImg<double> gradient;
    CImg<double> nonMaxSupped;
    CImg<double> edge;
    CImg<double> linked;
    CImg<double> deleted;

    string filename;
    double sigma;
    int scale, th, tl;
};

#endif
