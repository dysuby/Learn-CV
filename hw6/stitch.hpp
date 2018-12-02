#ifndef STITCH
#define STITCH

#include <vector>
#include "CImg.h"
#include "ransac.hpp"

extern "C" {
#include <vl/sift.h>
#include <vl/kdtree.h>
}

#define NOCTAVES 4
#define NLEVELS 2
#define O_MIN 0
#define PEAK_THRESH 0.8
#define EDGE_THRESH 10.0
#define DIS_RATIO_THRESH 0.8

typedef vector<pair<VlSiftKeypoint, VlSiftKeypoint> > PointPairs;

using namespace std;
using namespace cimg_library;

class Stitch {
	public:
		Stitch(vector<const char *> filenames);

		~Stitch();

		void run();
		
		void saveRes(const char *resPath);

	private:
		void clear();

		CImg<unsigned char> warp(const CImg<float> &src);

		void sift(const CImg<float> &src, vector<VlSiftKeypoint> &features, vector<float*> &descriptor, int limit = -1);

		void match();

		void runRansac();

		void draw(CImg<unsigned char> &display, const CImg<float> &left, const CImg<float> &right);

		void correctPosition(CImg<float> &left, CImg<float> &right);

		CImg<unsigned char> blend(const CImg<float> &left, const CImg<float> &right);

	private:
		vector<CImg<unsigned char> > ori;
		CImg<unsigned char> res;

		vector<VlSiftKeypoint> lfeatures;
		vector<VlSiftKeypoint> rfeatures;

		vector<float*> ldescriptors;
		vector<float*> rdescriptors;

		PointPairs matchedPoint;
};

#endif