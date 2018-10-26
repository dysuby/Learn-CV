#include "HoughCircle.hpp"

HoughCircle::HoughCircle(const char *filename) : sint(360, 0), cost(360, 0) {
    Canny canny(filename, 1.5, 1, 80, 40);
    raw = canny.canny();  // canny detector
    raw.display("I_edge");
    res = CImg<double>(raw.width(), raw.height(), 1, 3);
    edge = canny.deleteLine(100);  // 删掉多余的边缘
    cimg_forXY(raw, x, y) {        // 8-bit to RGB
        if (raw(x, y)) {
            res(x, y, 0) = 255;
            res(x, y, 1) = 255;
            res(x, y, 2) = 255;
        }
    }
    // edge.display();
}

void HoughCircle::initTable() {
    // 360 度的三角函数表
    for (int i = 0; i < 360; ++i) {
        double arc = cimg::PI * i / 180;
        sint[i] = sin(arc);
        cost[i] = cos(arc);
    }
}

void HoughCircle::hough(int rmin, int rmax, int thresh) {
    initTable();  // 初始化三角函数表
    for (int r = rmin; r < rmax; ++r) {
        // 初始化 Hough 空间
        votes = CImg<int>(raw.width(), raw.height());
        votes.fill(0);

        // 投票
        cimg_forXY(edge, x, y) if (edge(x, y)) vote(x, y, r);

        // 阈值
        threshold(thresh, r);
    }

    // 移除圆心相近的圆
    removeNeighbor();

    // 画出圆
    for (int i = 0; i < circles.size(); ++i) {
        displayCircle(circles[i].a, circles[i].b, circles[i].r);
        printf("circle: (x - %d)^2 + (y - %d)^2 = %d votes: %d\n", circles[i].a,
               circles[i].b, circles[i].r * circles[i].r, circles[i].votes_num);
    }
    printf("coins nums: %d\n", circles.size());
    res.display("I_2");

    // 根据圆画出边缘点
    displayEdge();
    res.display("I_3");

    printf("DONE");
}

void HoughCircle::vote(int x, int y, int r) {
    for (int theta = 0; theta < 360; ++theta) {
        int a = x - r * cost[theta];
        int b = y - r * sint[theta];
        if (a < 0 || a >= votes.width() || b < 0 || b >= votes.height())
            continue;
        ++votes(a, b);
    }
}

void HoughCircle::threshold(int thresh, int r) {
    // 对当前 Hough 空间中大于阈值 thresh 的点视为有效
    cimg_forXY(votes, x, y) {
        if (votes(x, y) > thresh) {
            args maxpos;
            maxpos.a = x;
            maxpos.b = y;
            maxpos.r = r;
            maxpos.votes_num = votes(x, y);
            circles.push_back(maxpos);
        }
    }
}

void HoughCircle::removeNeighbor() {
    // 对圆集中任意两个圆判断：如果小圆圆心在大圆内则视为两圆重叠，移去小圆。
    for (int i = 0; i < circles.size(); ++i) {
        for (int j = i + 1; j < circles.size(); ++j) {
            int maxR = max(circles[i].r, circles[j].r);
            // 根据两圆圆心距离判断
            if (pow(circles[i].a - circles[j].a, 2) +
                    pow(circles[i].b - circles[j].b, 2) <
                maxR * maxR) {
                // 移去较小的圆
                if (circles[i].r < circles[j].r) {
                    circles.erase(circles.begin() + i);
                    --i;
                    break;
                } else {
                    circles.erase(circles.begin() + j);
                    --j;
                }
            }
        }
    }
}

void HoughCircle::displayCircle(int a, int b, int r) {
    // 根据半径计算出圆的边缘的合适粗细
    int delta = r * r / 49;
    cimg_forXY(res, x, y) {
        if (abs((x - a) * (x - a) + (y - b) * (y - b) - r * r) < delta) {
            res(x, y, 0) = 0;
            res(x, y, 1) = 0;
            res(x, y, 2) = 255;
        }
    }
}

void HoughCircle::displayEdge() {
    // 若圆的边缘周围 5 * 5 的区域内存在边缘点，则视为硬币边缘，将边缘点涂上红色
    cimg_forXY(res, x, y) {
        if (res(x, y, 0) == 0 && res(x, y, 1) == 0 && res(x, y, 2) == 255) {
            for (int i = x - 2; i <= x + 2; ++i) {
                for (int j = y - 2; j <= y + 2; ++j) {
                    if (i < 0 || i >= res.width() || j < 0 || j >= res.height())
                        continue;
                    if (raw(i, j)) {
                        res(i, j, 0) = 255;
                        res(i, j, 1) = 0;
                        res(i, j, 2) = 0;
                    }
                }
            }
        }
    }
}

void HoughCircle::save(const char *filename) { res.save(filename); }
