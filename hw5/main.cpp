#include "Morphing.hpp"

#define PATH "./test_data/"

void test() {
    std::vector<Line> a, b;

    a.push_back(Line(11, 339, 108, 308));   // 左肩膀
    a.push_back(Line(321, 339, 234, 298));  // 右肩膀
    a.push_back(Line(82, 248, 166, 303));   // 左下巴
    a.push_back(Line(243, 246, 166, 303));  // 右下巴
    a.push_back(Line(69, 169, 82, 246));    // 左脸颊
    a.push_back(Line(255, 171, 243, 244));  // 右脸颊
    a.push_back(Line(166, 165, 166, 228));  // 鼻子
    a.push_back(Line(107, 186, 137, 187));  // 左眼下
    a.push_back(Line(115, 179, 130, 180));  // 左眼上
    a.push_back(Line(221, 186, 192, 188));  // 右眼下
    a.push_back(Line(213, 178, 197, 179));  // 右眼上
    a.push_back(Line(116, 244, 195, 244));  // 上嘴唇
    a.push_back(Line(153, 261, 174, 261));  // 下嘴唇
    a.push_back(Line(116, 280, 106, 305));  // 左颈
    a.push_back(Line(218, 274, 229, 297));  // 右颈
    a.push_back(Line(106, 165, 139, 165));  // 左眉
    a.push_back(Line(225, 165, 195, 165));  // 右眉
    a.push_back(Line(74, 76, 159, 29));     // 左发梢
    a.push_back(Line(260, 82, 193, 26));    // 右发梢
    a.push_back(Line(50, 175, 69, 225));    // 左耳
    a.push_back(Line(273, 175, 253, 226));  // 右耳
    a.push_back(Line(60, 85, 50, 155));     // 左发
    a.push_back(Line(270, 94, 274, 156));   // 右发

    b.push_back(Line(0, 362, 91, 336));     // 左肩膀
    b.push_back(Line(321, 347, 241, 322));  // 右肩膀
    b.push_back(Line(82, 273, 164, 347));   // 左上下巴
    b.push_back(Line(243, 270, 164, 347));  // 右上下巴
    b.push_back(Line(68, 179, 81, 272));    // 左脸颊
    b.push_back(Line(252, 180, 243, 268));  // 右脸颊
    b.push_back(Line(164, 173, 165, 241));  // 鼻子
    b.push_back(Line(105, 180, 134, 183));  // 左眼下
    b.push_back(Line(107, 171, 126, 173));  // 左眼上
    b.push_back(Line(219, 182, 188, 183));  // 右眼下
    b.push_back(Line(212, 172, 194, 172));  // 右眼上
    b.push_back(Line(131, 272, 187, 272));  // 上嘴唇
    b.push_back(Line(153, 284, 172, 284));  // 下嘴唇
    b.push_back(Line(92, 291, 93, 335));    // 左颈
    b.push_back(Line(234, 291, 237, 324));  // 右颈
    b.push_back(Line(93, 163, 134, 165));   // 左眉
    b.push_back(Line(234, 161, 186, 163));  // 右眉
    b.push_back(Line(51, 71, 125, 10));     // 左发梢
    b.push_back(Line(264, 56, 191, 13));    // 右发梢
    b.push_back(Line(54, 192, 70, 240));    // 左耳
    b.push_back(Line(269, 182, 251, 237));  // 右耳
    b.push_back(Line(42, 91, 51, 157));     // 左发
    b.push_back(Line(275, 93, 272, 155));   // 右发
    Morphing mp(PATH "1.bmp", PATH "2.bmp", a, b);
    mp.morph(11, 1, 2, 0);
    // mp.genGIF(PATH "res.gif");
}

int main() {
    test();
    return 0;
}