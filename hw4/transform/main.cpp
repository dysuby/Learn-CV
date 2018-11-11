#include "Transform.hpp"

#define PATH "./test_data/"

void test() {
    Transform(PATH "1.bmp").transform(PATH "2.bmp");
    Transform(PATH "3.bmp").transform(PATH "4.bmp");
    Transform(PATH "5.bmp").transform(PATH "6.bmp");
    Transform(PATH "7.bmp").transform(PATH "8.bmp");
    Transform(PATH "9.bmp").transform(PATH "10.bmp");
}

int main() {
    test();
    return 0;
}