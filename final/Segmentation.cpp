#include "Segmentation.hpp"

#define THRESH 160
#define ANGEL_STEP 29
#define RHO_STEP 499

#define A4_length (297 * 10)
#define A4_width (210 * 10)

Segmentation::Segmentation() { cimg::srand(cimg::time()); }

void Segmentation::init() {
    src = raw;
    gray_img = CImg<double>(src.width(), src.height(), 1, 1);
    clustered = CImg<unsigned char>(src.width(), src.height(), 1, 1);
    mask = CImg<int>(src.width(), src.height(), 1, 1);
    detected = CImg<unsigned char>(src.width(), src.height(), 1, 1);
    centroid.clear();
    pairs.clear();
    points.clear();
}

void Segmentation::run(const char *filename, const char *save_path, const char *angle_path) {
    raw = CImg<unsigned char>(filename);
    init();

    toGray();

    // gray_img.display("res");

    // 聚类找出白纸区域
    kmeans(2);

    cimg_forXY(clustered, x, y) clustered(x, y) = centroid[mask(x, y)];
    // clustered.display("clustered");

    // 检测出白纸边缘
    detect();
    // detected.display("detected");

    // 霍夫直线检测找出 4 个角点
    points = HoughLine(detected).hough(THRESH, ANGEL_STEP, RHO_STEP);

    calMapPoint();

    projective();

    // res.display();

    res.save(save_path);

    std::ofstream out(angle_path);
    for (int i = 0; i < points.size(); ++i) {
        out << points[i].first << " " << points[i].second << std::endl;
    }
    out.close();
}

void Segmentation::toGray() {
    // 根据 ITU-R BT.709 Luma 系数转变灰度图
    cimg_forXY(src, x, y) {
        double r = src(x, y, 0);
        double g = src(x, y, 1);
        double b = src(x, y, 2);
        gray_img(x, y) = r * 0.2126 + g * 0.7152 + b * 0.0722;
    }
}

void Segmentation::kmeans(int k) {
    double minJ = DBL_MAX;
    CImg<int> tmpmask(src.width(), src.height());

    // 初始化 centroid
    centroid.push_back(gray_img[(int)(cimg::rand(gray_img.size()))]);
    while (centroid.size() < k) {
        int maxi = -1;
        double maxd = 0;
        for (int i = 0; i < gray_img.size(); ++i) {
            double dis = 0;
            for (int j = 0; j < centroid.size(); ++j)
                dis += pow(gray_img[i] - centroid[j], 2);
            if (dis > maxd) {
                maxd = dis;
                maxi = i;
            }
        }
        centroid.push_back(gray_img[maxi]);
    }

    std::vector<int> count(k);
    std::vector<double> new_centroid(k);

    bool change = true;
    while (change) {
        change = false;
        std::fill(new_centroid.begin(), new_centroid.end(), 0);
        std::fill(count.begin(), count.end(), 0);

        // compute c
        cimg_forXY(gray_img, x, y) {
            int mini = -1;
            double mind = DBL_MAX, dis;
            for (int i = 0; i < k; ++i) {
                dis = pow(gray_img(x, y) - centroid[i], 2);
                if (dis < mind) {
                    mini = i;
                    mind = dis;
                }
            }
            tmpmask(x, y) = mini;
            new_centroid[mini] += gray_img(x, y);
            ++count[mini];
        }

        // compute centroid
        for (int i = 0; i < k; ++i) {
            if (count[i]) new_centroid[i] /= count[i];
        }

        // compute J
        double j = 0;
        cimg_forXY(gray_img, x, y) j +=
            pow(gray_img(x, y) - new_centroid[tmpmask(x, y)], 2);

        // update centroid
        if (j < minJ) {
            centroid = new_centroid;
            minJ = j;
            mask = tmpmask;
            change = true;
        }
        printf("Current: J %lf k1: %lf k2: %lf\n", j, new_centroid[0],
               new_centroid[1]);
    }
}

void Segmentation::detect() {
    // 检测出边缘
    unsigned char fore = std::max(centroid[0], centroid[1]);
    cimg_forXY(clustered, x, y) {
        if (clustered(x, y) == fore) {
            bool bp = false, fp = false;
            for (int i = x - 1; i <= x + 1; ++i) {
                for (int j = y - 1; j <= y + 1; ++j) {
                    if (i == x && j == y) continue;
                    if (i < 0 || i >= clustered.width() || j < 0 ||
                        j >= clustered.height())
                        continue;
                    if (clustered(i, j) == fore)
                        fp = true;
                    else
                        bp = true;
                }
            }
            detected(x, y) = (bp && fp) * 255;
        }
    }

    // 广搜删除干扰线条
    std::queue<std::pair<int, int> > bfs;
    std::set<std::pair<int, int> > line;

    CImg<bool> visited(detected.width(), detected.height(), 1);
    cimg_forXY(visited, x, y) { visited(x, y) = false; }
    cimg_forXY(detected, x, y) {
        if (detected(x, y) == 255 && !visited(x, y)) {
            std::set<std::pair<int, int> > points;
            points.insert(std::make_pair(x, y));
            bfs.push(std::make_pair(x, y));
            visited(x, y) = true;
            while (bfs.size()) {
                std::pair<int, int> top = bfs.front();
                bfs.pop();
                for (int c = top.first - 1; c <= top.first + 1; ++c) {
                    for (int r = top.second - 1; r <= top.second + 1; ++r) {
                        if (r < 0 || c < 0 || r >= detected.height() ||
                            c >= detected.width())
                            continue;
                        if (detected(c, r) == 255 && !visited(c, r)) {
                            bfs.push(std::make_pair(c, r));
                            points.insert(std::make_pair(c, r));
                            visited(c, r) = true;
                        }
                    }
                }
            }
            if (points.size() > line.size()) line = points; // 记下最长的边缘
        }
    }
    // 删掉其他边缘
    cimg_forXY(detected, x, y) {
        if (detected(x, y) && line.find(std::make_pair(x, y)) == line.end())
            detected(x, y) = 0;
    }
}

void Segmentation::calMapPoint() {
    // 简单排序，将左上角点放在第一位
    for (int i = 1; i < points.size(); ++i) {
        if (points[i].second + points[i].first <
            points[0].second + points[0].first)
            std::iter_swap(points.begin() + i, points.begin());
    }

    // 根据距离确定位置关系
    std::pair<int, int> p1 = points[0];
    pairs.push_back(std::make_pair(Point(0, 0), Point(p1.first, p1.second)));
    std::map<double, int> mp;
    for (int i = 1; i < points.size(); ++i) {
        double dis = pow(points[i].first - p1.first, 2) +
                     pow(points[i].second - p1.second, 2);
        mp[dis] = i;
    }
    std::map<double, int>::iterator ite = mp.begin();
    int width = sqrt(ite->first);
    ++ite;
    int height = sqrt(ite->first);
    res = CImg<unsigned char>(width, height, 1, 3);

    Point tomap[] = {Point(width - 1, 0), Point(0, height - 1),
                     Point(width - 1, height - 1)};
    int i = 0;
    for (std::map<double, int>::iterator it = mp.begin(); it != mp.end();
         ++it, ++i) {
        pairs.push_back(std::make_pair(
            tomap[i],
            Point(points[it->second].first, points[it->second].second)));
        printf("Mapping point: %d %d %d %d\n", points[it->second].first,
               points[it->second].second, tomap[i].x, tomap[i].y);
    }
}

bool Segmentation::gauss_jordan(double A[8][8], int b[8], double H[8]) {
    int n = 8;
    double B[8][9];
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            B[i][j] = A[i][j];
        }
        B[i][n] = b[i];
    }

    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i; j < n; ++j) {
            if (abs(B[j][i]) > abs(B[pivot][i])) pivot = j;
        }
        std::swap(B[i], B[pivot]);

        //无解或者有无穷解
        if (abs(B[i][i]) < 1E-8) return false;

        double k = B[i][i];
        for (int j = i; j <= n; ++j) B[i][j] /= k;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                for (int k = i + 1; k <= n; ++k) {
                    B[j][k] -= B[j][i] * B[i][k];
                }
            }
        }
    }
    for (int i = 0; i < n; ++i) H[i] = B[i][n];
    return true;
}

bool Segmentation::calHomography(const PointPairs &samples, double H[9]) {
    // 计算单应矩阵
    int x1[4], y1[4], x2[4], y2[4];
    for (int i = 0; i < samples.size(); ++i) {
        x1[i] = samples[i].first.x;
        y1[i] = samples[i].first.y;
        x2[i] = samples[i].second.x;
        y2[i] = samples[i].second.y;
    }

#define A(i)                                                      \
    {x1[i], y1[i], 1, 0, 0, 0, -x1[i] * x2[i], -y1[i] * x2[i]}, { \
        0, 0, 0, x1[i], y1[i], 1, -x1[i] * y2[i], -y1[i] * y2[i]  \
    }
    double src[8][8] = {A(0), A(1), A(2), A(3)};
#undef A

    int v[8];
    for (int i = 0; i < 4; ++i) {
        v[2 * i] = x2[i];
        v[2 * i + 1] = y2[i];
    }

    H[8] = 1;
    return gauss_jordan(src, v, H);
}

void Segmentation::projective() {
    double H[9];
    if (calHomography(pairs, H)) {
        printf("projective matrix: ");
        for (int i = 0; i < 9; ++i) printf("%lf ", H[i]);
        putchar('\n');
        cimg_forXY(res, x, y) {
            // 计算反向映射的点
            double ww = 1. / (H[6] * x + H[7] * y + 1.);
            double dx = (H[0] * x + H[1] * y + H[2]) * ww;
            double dy = (H[3] * x + H[4] * y + H[5]) * ww;
            if (dx < 0 || dx >= raw.width() - 1 || dy < 0 ||
                dy >= raw.height() - 1)
                continue;
                
            // bilinear
            int lx = (int)dx, ly = (int)dy;
            if (lx == raw.width() - 1 || ly == raw.height() - 1) {
                res(x, y, 0) = raw(lx, ly, 0);
                res(x, y, 1) = raw(lx, ly, 1);
                res(x, y, 2) = raw(lx, ly, 2);
                continue;
            }
            int hx = lx + 1, hy = ly + 1;
            double a = dx - lx, b = dy - ly;
            res(x, y, 0) = cimg::cut(raw(lx, ly, 0) * (1 - a) * (1 - b) +
                                         raw(lx, hy, 0) * (1 - a) * b +
                                         raw(hx, ly, 0) * a * (1 - b) +
                                         raw(hx, hy, 0) * a * b,
                                     0, 255);
            res(x, y, 1) = cimg::cut(raw(lx, ly, 1) * (1 - a) * (1 - b) +
                                         raw(lx, hy, 1) * (1 - a) * b +
                                         raw(hx, ly, 1) * a * (1 - b) +
                                         raw(hx, hy, 1) * a * b,
                                     0, 255);
            res(x, y, 2) = cimg::cut(raw(lx, ly, 2) * (1 - a) * (1 - b) +
                                         raw(lx, hy, 2) * (1 - a) * b +
                                         raw(hx, ly, 2) * a * (1 - b) +
                                         raw(hx, hy, 2) * a * b,
                                     0, 255);
        }
    }
}
