#include <iostream>
#include "HoughCircle.hpp"

#define PATH "./Dataset2/"
#define WRI_PATH "./result/"

void test(int thresh, int rmin, int rmax) {
    HoughCircle hc1(PATH "1.bmp");
    hc1.hough(180, 240, 140);
    hc1.save(WRI_PATH "1.bmp");

    HoughCircle hc2(PATH "2.bmp");
    hc2.hough(100, 150, 130);
    hc2.save(WRI_PATH "2.bmp");

    HoughCircle hc3(PATH "3.bmp");
    hc3.hough(100, 210, 90);
    hc3.save(WRI_PATH "3.bmp");

    HoughCircle hc4(PATH "4.bmp");
    hc4.hough(30, 70, 160);
    hc4.save(WRI_PATH "4.bmp");

    HoughCircle hc5(PATH "5.bmp");
    hc5.hough(280, 350, 80);
    hc5.save(WRI_PATH "5.bmp");

    HoughCircle hc6(PATH "6.bmp");
    hc6.hough(140, 160, 130);
    hc6.save(WRI_PATH "6.bmp");
}

int main() {
    int thresh = 0, rmin = 0, rmax = 0;
    // std::cin >> rmin >> rmax >> thresh;
    test(thresh, rmin, rmax);
    return 0;
}