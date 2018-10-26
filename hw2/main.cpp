#include <iostream>
#include "canny.hpp"

void test() {
    int defaultArg, scale, th, tl;
    float sigma;
    cout << "use default arg ? 1 for yes, 0 for no" << endl;
    cin >> defaultArg;
    if (defaultArg) {
        Canny c1("./test_data/lena.bmp");
        Canny c2("./test_data/bigben.bmp");
        Canny c3("./test_data/stpietro.bmp");
        Canny c4("./test_data/twows.bmp");
    } else {
        cout << "\ninput sigma: ";
        cin >> sigma;
        cout << "\ninput scale: ";
        cin >> scale;
        cout << "\ninput high thresh: ";
        cin >> th;
        cout << "\ninput low thresh: ";
        cin >> tl;
        Canny c1("./test_data/lena.bmp", sigma, scale, th, tl);
        Canny c2("./test_data/bigben.bmp", sigma, scale, th, tl);
        Canny c3("./test_data/stpietro.bmp", sigma, scale, th, tl);
        Canny c4("./test_data/twows.bmp", sigma, scale, th, tl);
    }
}

int main(int argc, const char *argv[]) {
    test();
    return 0;
}
