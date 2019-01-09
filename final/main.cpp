#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Digit.hpp"
#include "Segmentation.hpp"

#define SRC std::string("test/good samples/")
#define TMP std::string("tmp/good samples/")
#define SAVE std::string("digits/good samples/")
#define ANGLE std::string("angles.txt")

std::vector<std::string> getFiles(const char *cate_dir) {
    std::vector<std::string> files;  //存放文件名
    _finddata_t file;
    long lf;
    //输入文件夹路径
    if ((lf = _findfirst(cate_dir, &file)) == -1) {
        printf("%s not found!!!\n", cate_dir);
    } else {
        while (_findnext(lf, &file) == 0) {
            //输出文件名
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            files.push_back(file.name);
        }
    }
    _findclose(lf);
    //排序，按从小到大排序
    sort(files.begin(), files.end());
    return files;
}

void test() {
    Segmentation s;
    Digit d;
    std::vector<std::string> images =
        getFiles((string(".\\") + SRC + string("\\*")).c_str());
    for (int i = 0; i < images.size(); ++i) {
        string filename = images[i];
        int dot = images[i].find('.');
        string dir = images[i].substr(0, dot);
        mkdir((SAVE + dir).c_str());
        dir += "/";
        printf("%s\n", dir.c_str());
        s.run((SRC + filename).c_str(), (TMP + filename).c_str(), (SAVE + dir + ANGLE).c_str());
        d.run((TMP + filename).c_str(), (SAVE + dir).c_str());
    }
}

int main() {
    test();
    return 0;
}