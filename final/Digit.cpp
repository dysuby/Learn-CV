#include "Digit.hpp"

Digit::Digit() {}

void Digit::run(const char *filename, const char *digitpath) {
    src = CImg<unsigned char>(filename);
    src = Canny(src).canny();
    init();
    dilation();
    dilated.display();
    finddigits();
    resize();
    save(digitpath);
}

int Digit::calboarder(int begin, int ord, int factor) {
    int thresh = 10;
    while (true) {
        int white = 0;
        if (ord == 0)
            cimg_forY(src, y) white += (src(begin, y) == 255);
        else
            cimg_forX(src, x) white += (src(x, begin) == 255);
        if (white <= thresh)
            break;
        else
            begin += factor;
    }
    return begin;
}

void Digit::init() {
    int beginx = calboarder(5, 0, 1),
        endx = calboarder(src.width() - 5, 0, -1);
    int beginy = calboarder(5, 1, 1),
        endy = calboarder(src.height() - 100, 1, -1);
    cimg_forXY(src, x, y) {
        if (x < beginx || x > endx || y < beginy || y > endy) src(x, y) = 0;
    }
    digits.clear();
    dilated = CImg<unsigned char>(src);
}

void Digit::dilation() {
    int tp[3][3] = {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}};
    int target = 0;
    CImg<unsigned char> tmp(src);
    for (int k = 0; k < 1; ++k) {
        cimg_forXY(tmp, x, y) {
            if (tmp(x, y) == target) {
                int flag = 0;
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        if (x + i < 0 || x + i >= tmp.width() || y + j < 0 ||
                            y + j >= tmp.height())
                            continue;
                        if (i == 0 && j == 0) continue;
                        if (tp[j + 1][i + 1] == -1) continue;
                        if (tmp(x + i, y + j) == tp[j + 1][i + 1]) {
                            ++flag;
                        }
                    }
                }
                if (flag) dilated(x, y) = tp[1][1];
            }
        }
        if (k == 0) {
            tp[0][0] = tp[0][2] = tp[2][0] = tp[2][2] = -1;
        } else if (k == 1) {
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j) tp[i][j] = 0;
            target = 255;
        }
        tmp = dilated;
    }
}

void Digit::finddigits() {
    /* 通过广搜，获得一条边缘的长度和点集 */
    queue<pair<int, int> > bfs;
    vector<set<pair<int, int> > > lines;
    CImg<int> visited(dilated.width(), dilated.height(), 1);
    int index = 1;
    cimg_forXY(visited, x, y) { visited(x, y) = false; }
    cimg_forXY(dilated, x, y) {
        if (dilated(x, y) == 255 && !visited(x, y)) {
            set<pair<int, int> > points;
            points.insert(make_pair(x, y));
            bfs.push(make_pair(x, y));
            visited(x, y) = index;
            while (bfs.size()) {
                pair<int, int> top = bfs.front();
                bfs.pop();
                for (int c = top.first - 1; c <= top.first + 1; ++c) {
                    for (int r = top.second - 1; r <= top.second + 1; ++r) {
                        if (r < 0 || c < 0 || r >= dilated.height() ||
                            c >= dilated.width())
                            continue;
                        if (dilated(c, r) == 255 && !visited(c, r)) {
                            bfs.push(make_pair(c, r));
                            points.insert(make_pair(c, r));
                            visited(c, r) = index;
                        }
                    }
                }
            }
            /* 删除不满足条件的边缘 */
            if (points.size() < 100) {
                for (set<pair<int, int> >::iterator i = points.begin();
                     i != points.end(); ++i)
                    dilated(i->first, i->second) = 0;
            } else {
                lines.push_back(points);
                ++index;
            }
        }
    }

    // 得到数字并排序
    vector<pair<int, int> > center(lines.size());
    int thresh = 0;
    for (int i = 0; i < lines.size(); ++i) {
        set<pair<int, int> > points = lines[i];

        // 计算边界点
        int miny = INT_MAX, maxy = INT_MIN, minx = INT_MAX, maxx = INT_MIN;
        for (set<pair<int, int> >::iterator ite = points.begin();
             ite != points.end(); ++ite) {
            if (ite->first < minx) minx = ite->first;
            if (ite->first > maxx) maxx = ite->first;
            if (ite->second < miny) miny = ite->second;
            if (ite->second > maxy) maxy = ite->second;
        }
        center[i] = make_pair((maxx + minx) / 2, (maxy + miny) / 2);
        thresh += maxy - miny;

        // 得到子图像
        int dx = maxx - minx + 1, dy = maxy - miny + 1;
        int boarder = max(dx, dy), d = abs(dx - dy) / 2;
        int black = boarder / 10 * 2;
        CImg<unsigned char> tmp(boarder + 2 * black, boarder + 2 * black);
        cimg_forXY(tmp, x, y) {
            tmp(x, y) = 0;
            if (x < black || y < black || x >= boarder + black ||
                y >= boarder + black)
                continue;
            if (boarder == dy) {
                if (x >= d + black && x < dx + d + black &&
                    visited(minx + x - d - black, miny + y - black) == i + 1) {
                    tmp(x, y) = dilated(minx + x - d - black, miny + y - black);
                }
            } else if (y >= d + black && y < dy + d + black &&
                       visited(minx + x - black, miny + y - d - black) ==
                           i + 1) {
                tmp(x, y) = dilated(minx + x - black, miny + y - d - black);
            }
        }
        digits.push_back(tmp);
    }

    thresh /= digits.size();
    // 排序
    for (int i = 0; i < digits.size(); ++i) {
        for (int j = i + 1; j < digits.size(); ++j) {
            if (center[i].second > center[j].second + thresh ||
                abs(center[i].second - center[j].second) < thresh &&
                    center[i].first > center[j].first) {
                swap(digits[i], digits[j]);
                swap(center[i], center[j]);
            }
        }
    }
    // 刪掉边界
    int begin = 0;
    while (begin < center.size() - 1) {
        if (center[begin].second > center[begin + 1].second - thresh) break;
        ++begin;
    }
    int end = center.size() - 1;
    while (end > 0) {
        if (center[end].second < center[end - 1].second + thresh) break;
        --end;
    }
    digits = vector<CImg<unsigned char> >(digits.begin() + begin, digits.begin() + end + 1);
}

void Digit::resize() {
    vector<CImg<unsigned char> > tmp;
    for (int i = 0; i < digits.size(); ++i) {
        CImg<unsigned char> img(28, 28);
        int boarder = digits[i].height();
        cimg_forXY(img, x, y) {
            // bilinear
            double dx = x / 28.0 * boarder, dy = y / 28.0 * boarder;
            if (dx < 0 || dy < 0 || dx >= digits[i].width() ||
                dy >= digits[i].height())
                continue;
            int lx = (int)dx, ly = (int)dy;
            if (lx == digits[i].width() - 1 || ly == digits[i].height() - 1) {
                img(x, y) = digits[i](lx, ly);
                continue;
            }
            int hx = lx + 1, hy = ly + 1;
            double a = dx - lx, b = dy - ly;
            img(x, y) = cimg::cut(digits[i](lx, ly) * (1 - a) * (1 - b) +
                                      digits[i](lx, hy) * (1 - a) * b +
                                      digits[i](hx, ly) * a * (1 - b) +
                                      digits[i](hx, hy) * a * b,
                                  0, 255);
        }
        tmp.push_back(img);
    }
    digits = tmp;
}

void Digit::save(const char *folder) {
    char path[20] = {0};
    if (digits.size() != 37) {
        printf("Not eaqul to 37!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    for (int i = 0; i < digits.size(); ++i) {
        sprintf(path, "%s%d.bmp", folder, i);
        digits[i].save(path);
    }
}