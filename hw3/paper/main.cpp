#include "HoughLine.hpp"

#define PATH "./Dataset1/"
#define WRI_PATH "./result/"

#define THRESH 200
#define ANGEL_STEP 29
#define RHO_STEP 499

void test() {
    HoughLine hough1(PATH "1.bmp");
    hough1.hough(THRESH, ANGEL_STEP, RHO_STEP);
    hough1.save(WRI_PATH "1.bmp");

    HoughLine hough2(PATH "2.bmp");
    hough2.hough(THRESH, ANGEL_STEP, RHO_STEP);
    hough2.save(WRI_PATH "2.bmp");

    HoughLine hough3(PATH "3.bmp");
    hough3.hough(THRESH, ANGEL_STEP, RHO_STEP);
    hough3.save(WRI_PATH "3.bmp");

    HoughLine hough4(PATH "4.bmp");
    hough4.hough(THRESH, ANGEL_STEP, RHO_STEP);
    hough4.save(WRI_PATH "4.bmp");

    HoughLine hough5(PATH "5.bmp");
    hough5.hough(THRESH, ANGEL_STEP, RHO_STEP);
    hough5.save(WRI_PATH "5.bmp");

    HoughLine hough6(PATH "6.bmp");
    hough6.hough(THRESH, ANGEL_STEP, RHO_STEP);
    hough6.save(WRI_PATH "6.bmp");
}

int main() {
    test();
    return 0;
}