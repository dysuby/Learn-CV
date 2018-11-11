#include "Histogram.hpp"

#define PATH "./test_data/"

void test() {
    Histogram(PATH "1.bmp").equalize();
    Histogram(PATH "2.bmp").equalize();
    Histogram(PATH "3.bmp").equalize();
    Histogram(PATH "4.bmp").equalize();
    Histogram(PATH "5.bmp").equalize();
    Histogram(PATH "6.bmp").equalize();
}

int main() { 
    test();
    return 0; 
}
