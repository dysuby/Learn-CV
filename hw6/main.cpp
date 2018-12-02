#include "stitch.hpp"

extern "C" {
#include <vl/generic.h>
#include <vl/sift.h>
}

#define PATH "./test/imageData"
#define RES_PATH "./res/"

int main() {
	vector<const char *> imgs;
	imgs.push_back(PATH "1/1.bmp");
	imgs.push_back(PATH "1/2.bmp");
	imgs.push_back(PATH "1/3.bmp");
	imgs.push_back(PATH "1/4.bmp");

	//imgs.push_back(PATH "2/1.bmp");
	//imgs.push_back(PATH "2/2.bmp");
	//imgs.push_back(PATH "2/3.bmp");
	//imgs.push_back(PATH "2/4.bmp");
	//imgs.push_back(PATH "2/5.bmp");
	//imgs.push_back(PATH "2/6.bmp");
	//imgs.push_back(PATH "2/7.bmp");
	//imgs.push_back(PATH "2/8.bmp");	
	//imgs.push_back(PATH "2/9.bmp");
	//imgs.push_back(PATH "2/10.bmp");
	//imgs.push_back(PATH "2/11.bmp");
	//imgs.push_back(PATH "2/12.bmp");
	//imgs.push_back(PATH "2/13.bmp");
	//imgs.push_back(PATH "2/14.bmp");
	//imgs.push_back(PATH "2/15.bmp");
	//imgs.push_back(PATH "2/16.bmp");
	//imgs.push_back(PATH "2/17.bmp");
	//imgs.push_back(PATH "2/18.bmp");
	Stitch runner(imgs);
	runner.run();
	runner.saveRes(RES_PATH "res1.bmp");
    return 0;
}