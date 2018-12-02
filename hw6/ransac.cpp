#include "ransac.hpp"

ransac::ransac(PointPairs mp): matchedPoint(mp), XY(mp.size(), 3), mappedXY(mp.size(), 3), bestErr(mp.size()), bestMask(mp.size()) {
	cimg_forX(XY, i) {
		XY(i, 0) = mp[i].first.x;
		XY(i, 1) = mp[i].first.y;
		XY(i, 2) = 1;

		mappedXY(i, 0) = mp[i].second.x;
		mappedXY(i, 1) = mp[i].second.y;
		mappedXY(i, 2) = 1;
	}
}

PointPairs ransac::run(double max_iters, double confidence, double reject_thresh) {
	int maxGoodCount = 0, goodCount = 0;
	double H[9];
	vector<bool> mask(matchedPoint.size());
	vector<double> err(matchedPoint.size());
	for (int iter = 0; iter < max_iters; ) {
		// 获得随机样本
		PointPairs samples = getSubset();
		
		if (calHomography(samples, H)) {
			goodCount = findInliers(H, reject_thresh, mask, err);  //找出内点
			++iter;
			if (goodCount > maxGoodCount) {
				// 更新模型
				memcpy(xform, H, 9 * sizeof(double));
				maxGoodCount = goodCount;
				bestMask.swap(mask);
				bestErr.swap(err);

				// 更新迭代
				max_iters = log(1. - confidence) / log(1. - pow((double)goodCount / matchedPoint.size(), 4));
			}
			printf("inliers: %d iter_time: %d max_iters:%.0lf\n", maxGoodCount, iter, max_iters);
		}
	}

	PointPairs ret;
	for (int i = 0; i < matchedPoint.size(); ++i) {
		if (bestMask[i])
			ret.push_back(matchedPoint[i]);
	}
	printf("%d/%d\n", ret.size(), matchedPoint.size());
	return ret;
}

bool ransac::check(VlSiftKeypoint p1, VlSiftKeypoint p2, VlSiftKeypoint p3) {
	return (p3.y - p1.y) * (p3.x - p2.x) != (p3.y - p2.y) * (p3.x - p1.x);
}

PointPairs ransac::getSubset() {
	srand(time(NULL));
	PointPairs ret;
	int allThree[4][3] = { {0, 1, 2}, {0, 1, 3}, {0, 2, 3}, {1, 2 ,3} };
	bool flag = true;
	int visitd[4] = { 0 };
	while (flag) {
		ret.clear();
		flag = false;
		for (int i = 0; i < 4; ) {
			int index = rand() % matchedPoint.size(), j;
			// 不重复
			for (j = 0; j < i; ++j) {
				if (index == visitd[j])
					break;
			}
			if (j == i) {
				ret.push_back(matchedPoint[index]);
				visitd[i] = index;
				++i;
			}
		}
		// 不共线
		for (int i = 0; i < 4; ++i) {
			if (!check(ret[allThree[i][0]].first, ret[allThree[i][1]].first, ret[allThree[i][2]].first))
				flag = true;
		}
	}
	return ret;
}

bool ransac::gauss_jordan(double A[8][8], int b[8], double H[8]) {
	int n = 8;
	double B[8][9];
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			B[i][j] = A[i][j]; 
		}
		B[i][n] = b[i];	
	}

	for (int i = 0; i < n; ++i) {
		int pivot = i;
		for (int j = i; j < n; ++j) {
			if (abs(B[j][i]) > abs(B[pivot][i]))
				pivot = j;
		}
		swap(B[i], B[pivot]);

		//无解或者有无穷解 
		if (abs(B[i][i]) < 1E-8) return false;

		double k = B[i][i];
		for (int j = i; j <= n; ++j) B[i][j] /= k;
		for (int j = 0; j < n; ++j) {
			if (i != j) {
				for (int k = i + 1; k <= n; ++k) {
					B[j][k] -= B[j][i] * B[i][k];
				}
			}
		}
	}
	for (int i = 0; i < n; ++i)
		H[i] = B[i][n];
	return true;
}

bool ransac::calHomography(const PointPairs &samples, double H[9]) {
	int x1[4], y1[4], x2[4], y2[4];
	for (int i = 0; i < samples.size(); ++i) {
		x1[i] = samples[i].first.x;
		y1[i] = samples[i].first.y;
		x2[i] = samples[i].second.x;
		y2[i] = samples[i].second.y;
	}
#define A(i) { x1[i], y1[i], 1, 0, 0, 0, -x1[i] * x2[i], -x2[i] * y1[i] }, { 0, 0, 0, x1[i], y1[i], 1, -x1[i] * y2[i], -y1[i] * y2[i] }
	double src[8][8] = { A(0), A(1), A(2), A(3) };
#undef A

	int v[8];
	for (int i = 0; i < 4; ++i) {
		v[2 * i] = x2[i];
		v[2 * i + 1] = y2[i];
	}

	H[8] = 1;
	return gauss_jordan(src, v, H);
}

int ransac::findInliers(double H[9], double threshold, vector<bool> &mask, vector<double> &err) {
	int goodCount = 0;

	computeReprojError(H, err);
	threshold *= threshold;
	for (int i = 0; i < matchedPoint.size(); ++i) {
		if (err[i] <= threshold) {
			++goodCount;
			mask[i] = true;
		} else {
			mask[i] = false;
		}
	}
	return goodCount;
}


void ransac::computeReprojError(double H[9], vector<double> &err) {
	for (int i = 0; i < matchedPoint.size(); ++i) {
		double ww = 1. / (H[6] * XY(i, 0) + H[7] * XY(i, 1) + 1.);
		double dx = (H[0] * XY(i, 0) + H[1] * XY(i, 1) + H[2]) * ww - mappedXY(i, 0);
		double dy = (H[3] * XY(i, 0) + H[4] * XY(i, 1) + H[5]) * ww - mappedXY(i, 1);
		err[i] = dx * dx + dy * dy;
	}
}
