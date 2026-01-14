#pragma once

#include "header.h"

class BImage {

public:

	struct fPoint {
		fPoint(int x1, int y1);
		float x;
		float y;
	};

	BImage() = default;
	~BImage();
	BImage(const int w, const int h, int bt);

	void redraw(double param, int iii);
	uint8_t* getimp();

	void bblend(const char* f, double t);

	bool PointInTriangle(fPoint pt, fPoint v1, fPoint v2, fPoint v3);
	float sign(fPoint p1, fPoint p2, fPoint p3);
	std::vector<uint8_t> rgbhsv(double r, double g, double b);
	std::vector<double> hsvrgb(double h, double s, double v);

	int wi = 0;
	int he = 0;

private:

	uint8_t* im5 = nullptr;
	uint8_t* base = nullptr;
	int ty = 0;

};