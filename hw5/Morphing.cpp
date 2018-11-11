#include "Morphing.hpp"

Line::Line(double px, double py, double qx, double qy)
    : P(px, py),
      Q(qx, qy),
      PQ(Q.first - P.first, Q.second - P.second),
      Per_PQ(PQ.second, -PQ.first) {
    len = sqrt(pow(qx - px, 2) + pow(qy - py, 2));
}

double Line::u(int x, int y) {
    // (X - P) * (Q - P) / (|| Q - P ||)^2
    Vec PX(x - P.first, y - P.second);
    return (PX.first * PQ.first + PX.second * PQ.second) / (len * len);
}

double Line::v(int x, int y) {
    // (X - P) * perpendicular(Q - P) / || Q - P||
    Vec PX(x - P.first, y - P.second);
    return (PX.first * Per_PQ.first + PX.second * Per_PQ.second) / len;
}

Point Line::getPoint(double u, double v) {
    // P' + u * (Q' - P') + v * perpendicular(Q' - P') / ||Q' - P'||
    Point ret;
    ret.first = P.first + u * PQ.first + (v * Per_PQ.first) / len;
    ret.second = P.second + u * PQ.second + (v * Per_PQ.second) / len;
    return ret;
}

Morphing::Morphing(const char *src_filename, const char *dst_filename,
                   std::vector<Line> src_features,
                   std::vector<Line> dst_features)
    : src(src_filename),
      dst(dst_filename),
      sf(src_features),
      df(dst_features),
      frame_lines(df.size()) {
    if (sf.empty() || df.empty() || sf.size() != df.size()) {
        printf("Lines of src img and dst img must be the same and non-zero\n");
        exit(1);
    }

    // 显示特征线
    int color[3] = {255, 0, 0};
    CImg<unsigned char> temp1(src), temp2(dst);
    for (int i = 0; i < sf.size(); ++i) {
        temp1.draw_line(sf[i].P.first, sf[i].P.second, sf[i].Q.first,
                        sf[i].Q.second, color);
        temp2.draw_line(df[i].P.first, df[i].P.second, df[i].Q.first,
                        df[i].Q.second, color);
    }
    temp1.display();
    temp2.display();
}

void Morphing::lineInterpolation() {
    printf("Line interpolation begins\n");
    // 对每一对特征线生成每一帧的中间特征线
    for (int i = 0; i < df.size(); ++i) {
        frame_lines[i].resize(fc);
        Line sl = sf[i], dl = df[i];
        for (int j = 0; j < fc; ++j) {
            double ratio = (double)(j + 1) / fc;
            int px = (1.0 - ratio) * sl.P.first + ratio * dl.P.first,
                py = (1.0 - ratio) * sl.P.second + ratio * dl.P.second,
                qx = (1.0 - ratio) * sl.Q.first + ratio * dl.Q.first,
                qy = (1.0 - ratio) * sl.Q.second + ratio * dl.Q.second;
            frame_lines[i][j] = Line(px, py, qx, qy);
        }
    }
    printf("Line interpolation ends\n");
}

void Morphing::morph(int frame_count, double a, double b, double p) {
    fc = frame_count + 1;
    _a = a;
    _b = b;
    _p = p;
    lineInterpolation();
    frames = CImgList<unsigned char>(fc, dst.width(), dst.height(), 1, 3);
    for (int i = 0; i < fc; ++i) genFrame(i);
}

void Morphing::genFrame(int now) {
    printf("Gen frame %d\n", now + 1);
    int size = df.size();
    double ratio = (double)(now + 1) / fc, u, v, weight;
    Line src_line, tmp_line, dst_line;
    cimg_forXY(frames[now], x, y) {
        // 求 X’
        Point src_dsum(0, 0), dst_dsum(0, 0), X(x, y);
        double weightSum = 0;
        for (int i = 0; i < size; ++i) {
            tmp_line = frame_lines[i][now];
            // 求 u, v, weight
            u = tmp_line.u(x, y), v = tmp_line.v(x, y);
            weight = getWeight(tmp_line, X, u, v);
            weightSum += weight;

            // 求在原图的 X'
            src_line = sf[i];
            X = Point(src_line.getPoint(u, v));

            src_dsum.first += X.first * weight;
            src_dsum.second += X.second * weight;

            // 求在目标图的 X'
            dst_line = df[i];
            X = Point(dst_line.getPoint(u, v));

            dst_dsum.first += X.first * weight;
            dst_dsum.second += X.second * weight;
        }

        src_dsum.first =
            cimg::cut(src_dsum.first / weightSum, 0, src.width() - 1);
        src_dsum.second =
            cimg::cut(src_dsum.second / weightSum, 0, src.height() - 1);
        dst_dsum.first =
            cimg::cut(dst_dsum.first / weightSum, 0, dst.width() - 1);
        dst_dsum.second =
            cimg::cut(dst_dsum.second / weightSum, 0, dst.height() - 1);

        // 得到 X' 的 rgb
        unsigned char src_rgb[3], dst_rgb[3];
        bilinear(src, src_dsum, src_rgb);
        bilinear(dst, dst_dsum, dst_rgb);

        // 求中间帧的 rgb
        frames[now](x, y, 0) = (1.0 - ratio) * src_rgb[0] + ratio * dst_rgb[0];
        frames[now](x, y, 1) = (1.0 - ratio) * src_rgb[1] + ratio * dst_rgb[1];
        frames[now](x, y, 2) = (1.0 - ratio) * src_rgb[2] + ratio * dst_rgb[2];
    }

    char path[20] = {0};
    sprintf(path, "./res/%d.bmp", now + 1);
    frames[now].save(path);
    printf("Frame %d done\n", now + 1);
}

double Morphing::getWeight(Line l, Point X, double u, double v) {
    // weight = (len^p / (a + dist))^b
    double dis;

    if (u > 0.0 && u < 1.0)
        dis = fabs(v);
    else if (u <= 0.0)  // 在 PQ 反向延长线上
        dis = sqrt(pow(X.first - l.P.first, 2) + pow(X.second - l.P.second, 2));
    else if (u >= 1.0)  // 在 PQ 延长线上
        dis = sqrt(pow(X.first - l.Q.first, 2) + pow(X.second - l.Q.second, 2));

    return pow(pow(l.len, _p) / (_a + dis), _b);
}

void Morphing::bilinear(const CImg<unsigned char> &img, Point X,
                        unsigned char rgb[]) {
    int x_floor = floor(X.first), y_floor = floor(X.second);
    int x_ceil = std::min(x_floor + 1, img.width() - 1),
        y_ceil = std::min(y_floor + 1, img.height() - 1);
    double a = X.first - x_floor, b = X.second - y_floor;

    // S(x,y) = a*b*S(i,j) + a*(1-b)*S(i+1,j) + (1-a)*b*S(i,j+1) +
    //             (1-a)*(1-b)*S(i+1,j+1)
    for (int i = 0; i <= 2; ++i) {
        rgb[i] = cimg::cut((1 - a) * (1 - b) * img(x_floor, y_floor, i) +
                               a * (1 - b) * img(x_ceil, y_floor, i) +
                               (1 - a) * b * img(x_floor, y_ceil, i) +
                               a * b * img(x_ceil, y_ceil, i),
                           0, 255);
    }
}

void Morphing::genGIF(const char *filename) { system("py gif.py"); }