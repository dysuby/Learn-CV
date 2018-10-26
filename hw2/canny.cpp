#include "canny.hpp"
#include <queue>
#include <set>
#include <utility>

const string Canny::GRAY_PATH = "./test_data/grayScale/";
const string Canny::SMOOTH_PATH = "./test_data/smooth/";
const string Canny::GRADIENT_PATH = "./test_data/gradient/";
const string Canny::NMS_PATH = "./test_data/nms/";
const string Canny::EDGE_PATH = "./test_data/edge/";
const string Canny::LINKED_PATH = "./test_data/linked/";
const string Canny::DELETED_PATH = "./test_data/deleted/";

Canny::Canny(string _filename)
    : sigma(1.5), scale(2), th(60), tl(30), filename(_filename.substr(11)) {
    img.load_bmp(_filename.c_str());
    canny();
}

Canny::Canny(string _filename, double _sigma, int _scale, int _th, int _tl)
    : sigma(_sigma),
      scale(_scale),
      th(_th),
      tl(_tl),
      filename(_filename.substr(11)) {
    img.load_bmp(_filename.c_str());
    canny();
}

CImg<double> Canny::canny() {
    toGrayScale();
    gauss();
    findLoG();  // findLoG and nms
    doubleThreshold();
    linking();
    deleteLine(20);

    gray_img.display();
    smooth_img.display();
    gradient.display();
    nonMaxSupped.display();
    edge.display();
    linked.display();
    deleted.display();

    gray_img.save((GRAY_PATH + filename).c_str());
    smooth_img.save((SMOOTH_PATH + filename).c_str());
    gradient.save((GRADIENT_PATH + filename).c_str());
    nonMaxSupped.save((NMS_PATH + filename).c_str());
    edge.save((EDGE_PATH + filename).c_str());
    linked.save((LINKED_PATH + filename).c_str());
    deleted.save((DELETED_PATH + filename).c_str());

    return edge;
}

void Canny::toGrayScale() {
    // 根据 ITU-R BT.709 Luma 系数转变灰度图
    gray_img = CImg<double>(img.width(), img.height(), 1);
    cimg_forXY(img, x, y) {
        double r = img(x, y, 0);
        double g = img(x, y, 1);
        double b = img(x, y, 2);
        gray_img(x, y) = r * 0.2126 + g * 0.7152 + b * 0.0722;
    }
}

void Canny::gauss() {
    int ksize = 2 * scale + 1;
    CImg<double> gauss_filter(ksize, ksize, 1);
    printf(" FILLING UP GAUSS FILTER VALUES\n");
    double constant = 2.0 * sigma * sigma, sum = 0;

    /* 建立高斯滤波器 */
    cimg_forXY(gauss_filter, x, y) {
        gauss_filter(x, y) =
            (exp(-(pow(x - scale, 2) + pow(y - scale, 2)) / constant)) /
            (cimg::PI * constant);
        sum += gauss_filter(x, y);
    }

    gauss_filter /= sum;

    printf(" CONVOLUTION WITH GAUSS FILTER\n");

    /* 进行卷积 */
    smooth_img = gray_img.get_convolve(gauss_filter);
    printf(" GAUSS SMOOTHING DONE!\n");
}

void Canny::findLoG() {
    int height = img.height(), width = img.width();

    gradient = CImg<double>(width, height, 1);

    printf(" FIND THE LoG OF THE SMOOTHED IMAGE\n");

    // "2" 表示使用 Sobel 算子求梯度，"xy" 表示在XY方向上求
    CImgList<double> list = smooth_img.get_gradient("xy", 2);
    CImg<double> xg(list[0]);
    CImg<double> yg(list[1]);
    cimg_forXY(gradient, x, y) {
        gradient(x, y) = sqrt(xg(x, y) * xg(x, y) + yg(x, y) * yg(x, y));
    }
    printf(" DONE\n");
    nms(xg, yg);
}

void Canny::nms(CImg<double> gradx, CImg<double> grady) {
    printf(" BEGIN NMS\n");
    nonMaxSupped = CImg<double>(gradient.width(), gradient.height(), 1);
    /* 将图像边缘设为不可能的点 */
    cimg_forX(nonMaxSupped, x) {
        nonMaxSupped(x, 0) = 0;
        nonMaxSupped(x, gradient.height() - 1) = 0;
    }
    cimg_forY(nonMaxSupped, y) {
        nonMaxSupped(0, y) = 0;
        nonMaxSupped(gradient.width() - 1, y) = 0;
    }

    // ptemp1, ptemp2, weight 为插值。
    // xtemp, ytemp 为对应方向梯度
    double p1, p2, p3, p4, ptemp1, ptemp2, xtemp, ytemp, weight;
    for (int i = 1; i < gradient.width() - 1; ++i) {
        for (int j = 1; j < gradient.height() - 1; ++j) {
            if (gradient(i, j) == 0) {
                nonMaxSupped(i, j) = 0;
            } else {
                xtemp = gradx(i, j);
                ytemp = grady(i, j);
                if (fabs(ytemp) > fabs(xtemp)) {
                    // 梯度偏向 y 轴
                    weight = fabs(xtemp) / fabs(ytemp);
                    p2 = gradient(i, j - 1);
                    p4 = gradient(i, j + 1);
                    if (ytemp * xtemp > 0) {
                        // 90 ~ 135°
                        p1 = gradient(i - 1, j - 1);
                        p3 = gradient(i + 1, j + 1);
                    } else {
                        // 45 ~ 90°
                        p1 = gradient(i + 1, j - 1);
                        p3 = gradient(i - 1, j + 1);
                    }
                } else {
                    // 梯度偏向 x 轴
                    weight = fabs(ytemp) / fabs(xtemp);
                    p2 = gradient(i - 1, j);
                    p4 = gradient(i + 1, j);
                    if (ytemp * xtemp > 0) {
                        // 135 ~ 180°
                        p1 = gradient(i - 1, j - 1);
                        p3 = gradient(i + 1, j + 1);
                    } else {
                        // 0 ~ 45°
                        p1 = gradient(i - 1, j + 1);
                        p3 = gradient(i + 1, j - 1);
                    }
                }
                // 计算线性插值
                ptemp1 = weight * p1 + (1 - weight) * p2;
                ptemp2 = weight * p3 + (1 - weight) * p4;
                // 抑制非极大值
                if (gradient(i, j) >= ptemp1 && gradient(i, j) >= ptemp2)
                    nonMaxSupped(i, j) = gradient(i, j);
                else
                    nonMaxSupped(i, j) = 0;
            }
        }
    }
    printf(" NMS DONE!\n");
}

void Canny::doubleThreshold() {
    th = min(th, 255);
    tl = min(tl, 255);
    edge = CImg<double>(nonMaxSupped);
    printf(" BEGIN THRESHOLDING\n");
    cimg_forXY(edge, x, y) {
        if (edge(x, y) > th) {
            // 强边缘点
            edge(x, y) = 255;
        } else if (edge(x, y) < tl) {
            // 非边缘点
            edge(x, y) = 0;
        } else {
            bool anyHigh = false;
            /* 八邻域 */
            for (int c = x - 1; c <= x + 1; ++c) {
                for (int r = y - 1; r <= y + 1; ++r) {
                    if (r < 0 || c < 0 || r >= edge.height() ||
                        c >= edge.width())
                        continue;
                    if (edge(c, r) > th) {
                        edge(x, y) = 255;
                        anyHigh = true;
                        break;
                    }
                }
                if (anyHigh) break;
            }
            if (!anyHigh) edge(x, y) = 0;
        }
    }
    printf(" THRESHOLDING DONE\n");
}

void Canny::linking() {
    linked = CImg<double>(edge);
    int edge_label = 0;
    queue<pair<int, int> > bfs;
    // 端点集
    set<pair<int, int> > vertexes;

    /* 通过广搜，标记同一条边的边缘点 */
    printf(" LINKING NEIGHBOR LINES\n");

    // 初始化标记
    CImg<double> label(edge.width(), edge.height(), 1);
    cimg_forXY(label, x, y) { label(x, y) = -1; }

    cimg_forXY(label, x, y) {
        if (linked(x, y) == 255 && label(x, y) == -1) {
            bfs.push(make_pair(x, y));
            label(x, y) = edge_label;
            while (bfs.size()) {
                int neighbor = 0;
                pair<int, int> top = bfs.front();
                bfs.pop();
                for (int c = top.first - 1; c <= top.first + 1; ++c) {
                    for (int r = top.second - 1; r <= top.second + 1; ++r) {
                        if (r < 0 || c < 0 || r >= linked.height() ||
                            c >= linked.width())
                            continue;
                        if (linked(c, r) == 255 && label(c, r) == -1) {
                            ++neighbor;
                            label(c, r) = edge_label;
                            bfs.push(make_pair(c, r));
                        } else if (label(c, r) == edge_label) {
                            ++neighbor;
                        }
                    }
                }
                // 如果为一条边的端点
                if (neighbor == 1) vertexes.insert(top);
            }
            ++edge_label;
        }
    }

    set<pair<int, int> > islinked;
    /* 寻找边的端点到相邻边中间的非边缘点并标为边缘点 */
    cimg_forXY(linked, x, y) {
        if (linked(x, y) == 0) {
            for (int c = x - 1; c <= x + 1; ++c) {
                for (int r = y - 1; r <= y + 1; ++r) {
                    if (r < 0 || c < 0 || r >= linked.height() ||
                        c >= linked.width())
                        continue;
                    if (edge(c, r) == 255) {
                        // 已经被另一条边标记，说明当前边与另一条边相邻
                        if (label(x, y) != -1 && label(x, y) != label(c, r) &&
                            vertexes.find(make_pair(c, r)) != vertexes.end()) 
                            linked(x, y) = 255;
                        // 未被标记过
                        else if (vertexes.find(make_pair(c, r)) !=
                                vertexes.end())
                            label(x, y) = label(c, r);
                    }
                }
            }
        }
    }
    printf(" LINKING DONE\n");
}

void Canny::deleteLine(int threshold) {
    deleted = CImg<double>(linked);
    queue<pair<int, int> > bfs;

    printf(" DELETING LINES \n");

    /* 通过广搜，获得一条边缘的长度和点集 */
    CImg<bool> visited(deleted.width(), deleted.height(), 1);
    cimg_forXY(visited, x, y) { visited(x, y) = false; }
    cimg_forXY(deleted, x, y) {
        if (deleted(x, y) == 255 && !visited(x, y)) {
            set<pair<int, int> > points;
            points.insert(make_pair(x, y));
            bfs.push(make_pair(x, y));
            visited(x, y) = true;
            while (bfs.size()) {
                pair<int, int> top = bfs.front();
                bfs.pop();
                for (int c = top.first - 1; c <= top.first + 1; ++c) {
                    for (int r = top.second - 1; r <= top.second + 1; ++r) {
                        if (r < 0 || c < 0 || r >= deleted.height() ||
                            c >= deleted.width())
                            continue;
                        if (deleted(c, r) == 255 && !visited(c, r)) {
                            bfs.push(make_pair(c, r));
                            points.insert(make_pair(c, r));
                            visited(c, r) = true;
                        }
                    }
                }
            }
            /* 删除不满足条件的边缘 */
            if (points.size() < threshold) {
                for (set<pair<int, int> >::iterator i = points.begin();
                     i != points.end(); ++i)
                    deleted(i->first, i->second) = 0;
            }
        }
    }
    printf(" DELETING DONE\n");
}
