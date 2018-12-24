#include "HoughLine.hpp"

HoughLine::HoughLine(const CImg<unsigned char> &src)
    : edge(src), angleRange(180), sint(angleRange, 0), cost(angleRange, 0) {
    res = CImg<unsigned char>(edge.width(), edge.height(), 1, 3);
    cimg_forXY(res, x, y) res(x, y, 0) = res(x, y, 1) = res(x, y, 2) =
        edge(x, y);
    rhoRange = (edge.width() + edge.height()) * 2;  // 计算出 rho 的范围
    initTable();                                    // 初始化三角函数表
}

void HoughLine::initTable() {
    // theta 的范围设为 -90 ~ 90 度
    for (int i = 0; i < angleRange; ++i) {
        double arc = cimg::PI * (i - 90) / 180;
        sint[i] = sin(arc);
        cost[i] = cos(arc);
    }
}

std::vector<std::pair<int, int> > HoughLine::hough(int thresh, int angle_step,
                                                   int rho_step) {
    votes = CImg<int>(angleRange, rhoRange);
    votes.fill(0);

    // 投票
    cimg_forXY(edge, x, y) if (edge(x, y)) vote(x, y);

    // 阈值
    threshold(thresh, angle_step, rho_step);

    // 画出直线
    for (int i = 0; i < lines.size(); ++i) {
        displayLine(lines[i].first, lines[i].second);
    }

    // 画出白纸顶点
    displayCircle();

    res.display("point");

    return points;
}

void HoughLine::vote(int x, int y) {
    int r;
    for (int theta = 0; theta < angleRange; ++theta) {
        r = x * cost[theta] + y * sint[theta];
        ++votes(theta, r + rhoRange / 2);  // 避免 rho < 0
    }
}

void HoughLine::threshold(int thresh, int angle_step, int rho_step) {
    // 将投票空间划分为数个 angel_step * rho_step 的区域
    for (int x = 0; x < angleRange; x += angle_step) {
        for (int y = 0; y < rhoRange; y += rho_step) {
            int endx = std::min(x + angle_step, angleRange);
            int endy = std::min(y + rho_step, rhoRange);
            std::pair<int, int> maxpos(x, y);

            // 选出最大投票数的点
            for (int i = x; i < endx; ++i) {
                for (int j = y; j < endy; ++j) {
                    if (votes(i, j) >= votes(maxpos.first, maxpos.second))
                        maxpos = std::make_pair(i, j);
                }
            }

            // 若最大投票数超过阈值，则该点有效
            if (votes(maxpos.first, maxpos.second) > thresh)
                lines.push_back(std::make_pair(maxpos.first, maxpos.second));
        }
    }

    if (lines.size() == 4) return;

    std::vector<std::pair<int, int> > tmp;
    std::vector<bool> checked(lines.size(), false);
    for (int i = 0; i < lines.size(); ++i) {
        if (checked[i]) continue;
        int dist = 0;
        int max_idx = -1, min_idx = i;
        for (int j = i + 1; j < lines.size(); ++j) {
            int rmax = abs(lines[max_idx].second - rhoRange / 2);
            int rmin = abs(lines[min_idx].second - rhoRange / 2);
            int rj = abs(lines[j].second - rhoRange / 2);
            if ((abs(lines[i].first - lines[j].first) < 60 ||
                 abs(lines[i].first - lines[j].first) > 120)) {
                checked[j] = true;
                if (max_idx == -1) {
                    max_idx = j;
                    dist = abs(rj - rmin);
                    continue;
                }
                if (abs(rj - rmin) >
                    abs(rj - rmax) &&
                    abs(rj - rmin) > dist) {
                    dist = abs(rj - rmin);
                    max_idx = j;
                } else if (abs(rj - rmax) > dist) {
                    dist = abs(rj - rmax);
                    min_idx = j;
                }
            }
        }
        tmp.push_back(lines[min_idx]);
        tmp.push_back(lines[max_idx]);
        checked[i] = true;
    }
    lines = tmp;
    for (int i = 0; i < lines.size(); ++i)
        printf("line: %d %d\n", lines[i].first, lines[i].second);
}

void HoughLine::displayLine(int theta, int r) {
    r -= rhoRange / 2;  // 得到真正的 rho
    cimg_forXY(res, x, y) {
        if (fabs(r - (x * cost[theta] + y * sint[theta])) < 1) {
            res(x, y, 0) = 0;
            res(x, y, 1) = 0;
            res(x, y, 2) = 255;
        }
    }
}

void HoughLine::displayCircle() {
    int red[] = {255, 0, 0};
    // 通过计算直线两两之间的交点确定白纸顶点
    for (int i = 0; i < lines.size(); ++i) {
        for (int j = i + 1; j < lines.size(); ++j) {
            if ((abs(lines[i].first - lines[j].first) < 60 ||
                 abs(lines[i].first - lines[j].first) > 120))
                 continue;
            int theta1 = lines[i].first, rho1 = lines[i].second - rhoRange / 2;
            int theta2 = lines[j].first, rho2 = lines[j].second - rhoRange / 2;
            // 直接使用极坐标方程求交点
            double x =
                (sint[theta1] * rho2 - sint[theta2] * rho1) /
                (cost[theta2] * sint[theta1] - cost[theta1] * sint[theta2]);
            double y;
            if (theta1 != 90)
                y = (rho1 - x * cost[theta1]) / sint[theta1];
            else
                y = (rho2 - x * cost[theta2]) / sint[theta2];
            if (x < 0 || x >= res.width() || y < 0 || y >= res.height())
                continue;
            printf("Point: (%.0lf, %.0lf)\n", x, y);
            points.push_back(std::make_pair(x, y));
            res.draw_circle(x, y, 5, red);
        }
    }
}
