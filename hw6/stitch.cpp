#include "stitch.hpp"

Stitch::Stitch(vector<const char*> filenames) {
	if (filenames.size() < 2) {
		exit(-1);
	}
	for (int i = 0; i < filenames.size(); ++i)
		ori.push_back(CImg<float>(filenames[i]));
}

Stitch::~Stitch() {
	clear();
}

void Stitch::clear() {
	matchedPoint.clear();
	lfeatures.clear();
	for (int i = 0; i < ldescriptors.size(); ++i)
		delete[] ldescriptors[i];
	ldescriptors.clear();
	rfeatures.clear();
	for (int i = 0; i < rdescriptors.size(); ++i)
		delete[] rdescriptors[i];
	rdescriptors.clear();
}

void Stitch::draw(CImg<unsigned char> &display, const CImg<float> &left, const CImg<float> &right) {
	display.clear();
	display.append(left);
	display.append(right);
	const unsigned char BLUE[3] = { 0, 0, 255 }, RED[3] = { 255, 0, 0 };
	for (int i = 0; i < lfeatures.size(); ++i) {
		display.draw_circle(lfeatures[i].x, lfeatures[i].y, 2, RED);
	}
	for (int i = 0; i < rfeatures.size(); ++i) {
		display.draw_circle(rfeatures[i].x + left.width(), rfeatures[i].y, 2, RED);
	}
	for (int i = 0; i < matchedPoint.size(); ++i) {
		VlSiftKeypoint p1(matchedPoint[i].first), p2(matchedPoint[i].second);
		display.draw_line(p1.x, p1.y, p2.x + left.width(), p2.y, BLUE);
	}
}

void Stitch::run() {
	CImg<float> left(warp(ori[0])), right;
	CImg<unsigned char> display;
	for (int i = 1; i < ori.size(); ++i) {
		printf("Stitching %d\n", i);
		clear();
		right = warp(ori[i]);

		// sift
		printf("Sifting\n");
		sift(left, lfeatures, ldescriptors, ori[i-1].width());
		sift(right, rfeatures, rdescriptors);

		// features match
		printf("Features matching\n");
		match();

		draw(display, left, right);
		display.display("match");

		correctPosition(left, right);

		printf("Ransacing\n");
		runRansac();

		draw(display, left, right);
		display.display("ransac");

		printf("Blending\n");
		left = blend(left, right);
		display.clear();
		display.append(left);
		display.display("res");
	}
	res.append(left);
}

CImg<unsigned char> Stitch::warp(const CImg<float> &src) {
	CImg<unsigned char> res(src.width(), src.height(), 1, 3);
	double alpha = cimg::PI / 16;
	double f = (double)src.width() / (2 * tan(alpha / 2));
	double W = src.width() / 2;
	double H = src.height() / 2;
	int lx, ly, hx, hy;
	double xratio, yratio;
	cimg_forXY(src, x, y) {
		double theta = atan2((x - W), f);
		double mx = f * tan((x - W) / f) + W;
		double my = (y - H) / cos(theta) + H;
		if (mx < 0 || my < 0 || mx >= src.width() - 1 || my >= src.height() - 1) {
			res(x, y, 0) = res(x, y, 1) = res(x, y, 2) = 0;
			continue;
		}
		lx = (int)mx;
		hx = lx + 1;
		ly = (int)my;
		hy = ly + 1;
		xratio = mx - lx;
		yratio = my - ly;
		res(x, y, 0) = (1 - xratio) * (1 - yratio) * src(lx, ly, 0) + xratio * (1 - yratio) * src(hx, ly, 0)
			+ (1 - xratio) * yratio * src(lx, hy, 0) + xratio * yratio * src(hx, hy, 0);
		res(x, y, 1) = (1 - xratio) * (1 - yratio) * src(lx, ly, 1) + xratio * (1 - yratio) * src(hx, ly, 1)
			+ (1 - xratio) * yratio * src(lx, hy, 1) + xratio * yratio * src(hx, hy, 1);
		res(x, y, 2) = (1 - xratio) * (1 - yratio) * src(lx, ly, 2) + xratio * (1 - yratio) * src(hx, ly, 2)
			+ (1 - xratio) * yratio * src(lx, hy, 2) + xratio * yratio * src(hx, hy, 2);
	}
	return res;
}

void Stitch::sift(const CImg<float> &src, vector<VlSiftKeypoint> &features, vector<float*> &descriptor, int limit) {
	const unsigned char RED[3] = { 255, 0, 0 };
	if (limit == -1)
		limit = src.width();
	// 转成灰度图
	CImg<float> gray_img(src.width(), src.height());
	cimg_forXY(gray_img, x, y) {
		gray_img(x, y) = src(x, y, 0) * 0.2126 + src(x, y, 1) * 0.7152 + src(x, y, 2) * 0.0722;
	}

	// 创建一个新的 sift 滤波器
	VlSiftFilt *SiftFilt = vl_sift_new(gray_img.width(), gray_img.height(), NOCTAVES, NLEVELS, O_MIN);
	//vl_sift_set_peak_thresh(SiftFilt, PEAK_THRESH);
	//vl_sift_set_edge_thresh(SiftFilt, EDGE_THRESH);

	vl_sift_pix *ImageData = gray_img.data();
	if (vl_sift_process_first_octave(SiftFilt, ImageData) != VL_ERR_EOF) {
		do {
			//计算每组中的关键点
			vl_sift_detect(SiftFilt);
			VlSiftKeypoint *pKeyPoint = SiftFilt->keys;
			for (int i = 0; i < SiftFilt->nkeys; ++i) {
				double angles[4];
				VlSiftKeypoint *tmpKp = pKeyPoint + i;
				if (tmpKp->x > limit) continue;
				vl_sift_calc_keypoint_orientations(SiftFilt, angles, tmpKp);

				// 计算主方向特征描述子
				float *des = new float[128];
				vl_sift_calc_keypoint_descriptor(SiftFilt, des, tmpKp, angles[0]);
				descriptor.push_back(des);
				features.push_back(*tmpKp);
			}
		} while (vl_sift_process_next_octave(SiftFilt) != VL_ERR_EOF);
	}
	vl_sift_delete(SiftFilt);
}

void Stitch::match() { 
	// 建立 k-d 树
	float *data = new float[128 * rdescriptors.size()];
	for (int i = 0; i < rdescriptors.size(); ++i) {
		memcpy(data + i * 128, rdescriptors[i], sizeof(float) * 128);
	}

	VlKDForest *kdtree = vl_kdforest_new(VL_TYPE_FLOAT, 128, 1, VlDistanceL1);

	vl_kdforest_build(kdtree, rdescriptors.size(), data);

	// 找 KNN
	VlKDForestSearcher *searcher = vl_kdforest_new_searcher(kdtree);

	for (int i = 0; i < ldescriptors.size(); i++) {
		VlKDForestNeighbor neighbour[2];
		vl_kdforestsearcher_query(searcher, neighbour, 2, ldescriptors[i]);
		if (neighbour[0].distance < DIS_RATIO_THRESH * neighbour[1].distance) {
			matchedPoint.push_back(make_pair(lfeatures[i], rfeatures[neighbour[0].index]));
		}
	}

	// 释放
	vl_kdforest_delete(kdtree);
	delete[] data;
}

void Stitch::runRansac() {
	ransac runner(matchedPoint);
	matchedPoint = runner.run();
}

void Stitch::correctPosition(CImg<float> &left, CImg<float> &right) {
	double ratio = 0;
	for (int i = 0; i < matchedPoint.size(); ++i) {
		ratio += (matchedPoint[i].first.x > matchedPoint[i].second.x);
	}
	if (ratio / matchedPoint.size() < .8) {
		left.swap(right);
		for (int i = 0; i < matchedPoint.size(); ++i) {
			swap(matchedPoint[i].first, matchedPoint[i].second);
		}
		lfeatures.swap(rfeatures);
		ldescriptors.swap(rdescriptors);
	}
}

CImg<unsigned char> Stitch::blend(const CImg<float> &left, const CImg<float> &right) {
	double dx = 0, dy = 0;
	// pdy=abs(dy) cw=重叠区域宽度
	double pdy, cw;
	int lwidth = left.width(), lheight = left.height(), rwidth = right.width(), rheight = right.height();
	for (int i = 0; i < matchedPoint.size(); ++i) {
		dx += matchedPoint[i].first.x - matchedPoint[i].second.x;
		dy += matchedPoint[i].first.y - matchedPoint[i].second.y;
	}
	dx /= matchedPoint.size();
	dy /= matchedPoint.size();
	pdy = abs(dy);
	cw = lwidth - dx;

	CImg<unsigned char> res(left.width() + right.width() - cw, max(left.height(), right.height()) + pdy, 1, 3);

	cimg_forXY(res, x, y) {
		if (dy >= 0) {
			// 向下平移
			if (x < lwidth && y < lheight) {
				// (x, y) 在左图中
				if (x >= dx && y >= dy) {
					// 重叠
					res(x, y, 0) = cimg::cut(left(x, y, 0) * (lwidth - x) / cw
						+ right(x - dx, y - dy, 0) * (x - dx) / cw, 0, 255);
					res(x, y, 1) = cimg::cut(left(x, y, 1) * (lwidth - x) / cw
						+ right(x - dx, y - dy, 1) * (x - dx) / cw, 0, 255);
					res(x, y, 2) = cimg::cut(left(x, y, 2) * (lwidth - x) / cw
						+ right(x - dx, y - dy, 2) * (x - dx) / cw, 0, 255);
				} else {
					//A独在部分
					res(x, y, 0) = left(x, y, 0);
					res(x, y, 1) = left(x, y, 1);
					res(x, y, 2) = left(x, y, 2);
				}
			} else if (x >= dx && y >= dy) {
				//B独在部分
				res(x, y, 0) = right(x - dx, y - dy, 0);
				res(x, y, 1) = right(x - dx, y - dy, 1);
				res(x, y, 2) = right(x - dx, y - dy, 2);
			} else {    
				//黑色部分
				res(x, y, 0) = res(x, y, 1) = res(x, y, 2) = 0;
			}
		} else {    
			// 往上平移
			if (x < lwidth && y >= pdy && y < lheight) {
				if (x >= dx && y < rheight) {
					// 重叠
					res(x, y, 0) = cimg::cut(left(x, y - pdy, 0) * (lwidth - x) / cw
						+ right(x - dx, y, 0) * (x - dx) / cw, 0, 255);
					res(x, y, 1) = cimg::cut(left(x, y - pdy, 1) * (lwidth - x) / cw
						+ right(x - dx, y, 1) * (x - dx) / cw, 0, 255);
					res(x, y, 2) = cimg::cut(left(x, y - pdy, 2) * (lwidth - x) / cw
						+ right(x - dx, y, 2) * (x - dx) / cw, 0, 255);
				} else {    
					// A独在部分
					res(x, y, 0) = left(x, y - pdy, 0);
					res(x, y, 1) = left(x, y - pdy, 1);
					res(x, y, 2) = left(x, y - pdy, 2);
				}
			} else if (x >= dx && y < rheight) {    
				// B独在部分
				res(x, y, 0) = right(x - dx, y, 0);
				res(x, y, 1) = right(x - dx, y, 1);
				res(x, y, 2) = right(x - dx, y, 2);
			} else {    
				// 黑色部分
				res(x, y, 0) = res(x, y, 1) = res(x, y, 2) = 0;
			}
		}
	}
	return res;
}

void Stitch::saveRes(const char *resPath) {
	res.save(resPath);
}