#include "HoughLine.hpp"

HoughLine::HoughLine(const char *filename)
    : angleRange(180), sint(angleRange, 0), cost(angleRange, 0) {
    Canny canny(filename, 1, 1, 60, 30);
    raw = canny.canny();  // canny detector
    raw.display("I_edge");
    res = CImg<double>(raw.width(), raw.height(), 1, 3);
    edge = canny.deleteLine(3000);  // 删掉白纸里多余的边缘
    cimg_forXY(raw, x, y) {         // 8-bit to RGB
        if (raw(x, y)) {
            res(x, y, 0) = 255;
            res(x, y, 1) = 255;
            res(x, y, 2) = 255;
        }
    }

    rhoRange = (edge.width() + edge.height()) * 2;  // 计算出 rho 的范围
}

void HoughLine::initTable() {
    // theta 的范围设为 -90 ~ 90 度
    for (int i = 0; i < angleRange; ++i) {
        double arc = cimg::PI * (i - 90) / 180;
        sint[i] = sin(arc);
        cost[i] = cos(arc);
    }
}

void HoughLine::hough(int thresh, int angle_step, int rho_step) {
    initTable();  // 初始化三角函数表
    votes = CImg<int>(angleRange, rhoRange);
    votes.fill(0);

    // 投票
    cimg_forXY(edge, x, y) if (edge(x, y)) vote(x, y);
    // votes.display();

    // 阈值
    threshold(thresh, angle_step, rho_step);

    // 画出直线
    for (int i = 0; i < lines.size(); ++i) {
        displayLine(lines[i].first, lines[i].second);
    }
    res.display("I_2");

    // 画出白纸边缘
    displayEdgePoint();
    res.display("I_3");

    // 画出白纸顶点
    displayCircle();
    res.display("I_4");
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
            int endx = min(x + angle_step, angleRange);
            int endy = min(y + rho_step, rhoRange);
            pair<int, int> maxpos(x, y);

            // 选出最大投票数的点
            for (int i = x; i < endx; ++i) {
                for (int j = y; j < endy; ++j) {
                    if (votes(i, j) >= votes(maxpos.first, maxpos.second))
                        maxpos = make_pair(i, j);
                }
            }

            // 若最大投票数超过阈值，则该点有效
            if (votes(maxpos.first, maxpos.second) > thresh)
                lines.push_back(make_pair(maxpos.first, maxpos.second));
        }
    }
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
    // 真正的 theta 为 theta-90
    if (theta == 0)
        printf("Line: y = %.2lf\ttheta = %d, rho = %d\n", r / sint[theta],
               theta - 90, r);
    else if (theta == 90)
        printf("Line: x = %.2lf\ttheta = %d, rho = %d\n", r / cost[theta],
               theta - 90, r);
    else
        printf(
            "Line: y = %.2lfx%+.2lf\ttheta = %d, rho = %d, sin = %lf, cos = "
            "%lf\n",
            -cost[theta] / sint[theta], r / sint[theta], theta - 90, r,
            sint[theta], cost[theta]);
}

void HoughLine::displayEdgePoint() {
    // 根据去除无效边后的边缘图进行画点
    cimg_forXY(res, x, y) {
        // if (edge(x, y)) {
        //     res(x, y, 0) = 255;
        //     res(x, y, 1) = 0;
        //     res(x, y, 2) = 0;
        // }

        // 若边缘点附近 10*10 的区域内有直线点，则将该点视为白纸边缘

        if (raw(x, y)) {
            for (int i = x - 10; i <= x + 10; ++i) {
                for (int j = y - 10; j <= y + 10; ++j) {
                    if (i < 0 || i >= res.width() || j < 0 || j >=
                    res.height())
                        continue;
                    if (res(i, j, 0) == 0 && res(i, j, 1) == 0 &&
                        res(i, j, 2) == 255) {
                        res(x, y, 0) = 255;
                        res(x, y, 1) = 0;
                        res(x, y, 2) = 0;
                        break;
                    }
                }
            }
        }
    }
}

void HoughLine::displayCircle() {
    int red[] = {255, 0, 0};
    // 通过计算直线两两之间的交点确定白纸顶点
    for (int i = 0; i < lines.size(); ++i) {
        for (int j = i + 1; j < lines.size(); ++j) {
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
            res.draw_circle(x, y, 5, red);
        }
    }
}

void HoughLine::save(const char *filename) { res.save(filename); }
