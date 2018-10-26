#include "CImg.h"

#define PAINT_SINGLE(img, x, y, color)  img(x, y, 0) = color[0]; \
                                        img(x, y, 1) = color[1]; \
                                        img(x, y, 2) = color[2];

using namespace cimg_library;

unsigned char blue[3] = {0, 0, 255};
unsigned char yellow[3] = {255, 255, 0};
unsigned char white[3] = {255, 255, 255};
unsigned char red[3] = {255, 0, 0};
unsigned char black[3] = {0, 0, 0};
unsigned char green[3] = {0, 255, 0};
double degree35 = 35.0 * 3.14159265 / 180;

class Image {
  public:
    Image(const char *name) {
      img.load_bmp(name);
    }

    void display() {
      img.display();
    }

    void paint() {
      cimg_forXY(img, x, y) {
        if (img(x, y, 0) == white[0] && img(x, y, 1) == white[1] && img(x, y, 2) == white[2]) {
          PAINT_SINGLE(img, x, y, red);
        }

        if (img(x, y, 0) == black[0] && img(x, y, 1) == black[1] && img(x, y, 2) == black[2]) {
          PAINT_SINGLE(img, x, y, green);
        }
      }
    }

    void drawLine_Raw() {
      const int endY = 100 * sin(degree35), endX = 100 * cos(degree35);
      const double delta = (double)endY / endX;
      double err = 0;
      int y = 0;
      cimg_forX(img, x) {
        PAINT_SINGLE(img, x, y, blue);
        err += delta;
        if (std::abs(err) >= 0.5) {
          ++y;
          --err;
        }
        if (y == endY)
          break;
      }
    }

    void drawLine() {
      img.draw_line(0, 0, 100 * cos(degree35), 100 * sin(degree35), blue);
    }

    void drawCircle_Raw() {
      cimg_forXY(img, x, y) {
        if (pow(x - 50, 2) + pow(y - 50, 2) <= 900) {
          PAINT_SINGLE(img, x, y, blue);
        }
        if (pow(x - 50, 2) + pow(y - 50, 2) <= 9) {
          PAINT_SINGLE(img, x, y, yellow);
        }
      }
    }

    void drawCircle() {
      img.draw_circle(50, 50, 30, blue);
      img.draw_circle(50, 50, 3, yellow);
    }

    void save(const char *name) {
      img.save_bmp(name);
    }
  private:
    CImg<unsigned char> img;
};

int main() {
  Image src1("1.bmp");  // 不调用 cimg 方法
  Image src2("1.bmp");  // 调用 cimg 方法
  src1.display();

  src1.paint();
  src2.paint();

  src1.drawCircle_Raw();
  src2.drawCircle();

  src1.drawLine_Raw();
  src2.drawLine();

  src1.display();
  src2.display();

  src1.save("2（手动）.bmp");
  src2.save("2（cimg）.bmp");
  return 0;
}