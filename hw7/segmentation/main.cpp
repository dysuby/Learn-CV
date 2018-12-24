#include "Segmentation.hpp"

#define PATH "test/"
#define SAVE "res/"

void test() {
    Segmentation s;
    s.run(PATH "1.bmp", SAVE "1.bmp");
    s.run(PATH "2.bmp", SAVE "2.bmp");
    s.run(PATH "3.bmp", SAVE "3.bmp");
    s.run(PATH "4.bmp", SAVE "4.bmp");
    s.run(PATH "5.bmp", SAVE "5.bmp");
    s.run(PATH "6.bmp", SAVE "6.bmp");
}

int main() {
    test();
    return 0;
}