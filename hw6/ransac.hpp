#ifndef RANSAC
#define RANSAC

#include <vector>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

extern "C" {
#include <vl/sift.h>
#include <vl/random.h>
}

#define CONFIDENCE 0.995
#define MAX_ITERS 20000
#define REJECT_THRESH 3

typedef vector<pair<VlSiftKeypoint, VlSiftKeypoint> > PointPairs;

class ransac {
	public:
		ransac(PointPairs mp);

		PointPairs run(double max_iters = MAX_ITERS, double confidence = CONFIDENCE, double reject_thresh = REJECT_THRESH);

	private:
		PointPairs getSubset();

		bool check(VlSiftKeypoint p1, VlSiftKeypoint p2, VlSiftKeypoint newp);

		bool calHomography(const PointPairs &samples, double model[9]);

		void computeReprojError(double H[9], vector<double> &err);

		int findInliers(double H[9], double reject_thresh, vector<bool> &mask, vector<double> &err);

		bool gauss_jordan(double A[8][8], int b[8], double H[8]);

	private:
		PointPairs matchedPoint;
		
		double xform[9];

		CImg<> XY;

		CImg<> mappedXY;

		vector<double> bestErr;

		vector<bool> bestMask;
};

#endif
