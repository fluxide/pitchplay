#pragma once

#include "header.h"

//general purpose button image (icon) renderer 
class BImage {

public:

	struct fPoint { //basic point struct
		fPoint(int x1, int y1);
		float x;
		float y;
	};

	BImage() = default;
	~BImage();
	BImage(const int w, const int h, int bt);

	void redraw(double param, int iii); //main rendering function
	uint8_t* getimp(); //gets pointer to the image buffer

	bool PointInTriangle(fPoint pt, fPoint v1, fPoint v2, fPoint v3); //is a point within a triangle defined by 3 other points?
	float sign(fPoint p1, fPoint p2, fPoint p3); //used in the processing of the above function
	std::vector<uint8_t> rgbhsv(double r, double g, double b); //color space conversions
	std::vector<double> hsvrgb(double h, double s, double v);

	int wi = 0; //width
	int he = 0; //height

private:

	uint8_t* base = nullptr; //byte array with pixel data
	int ty = 0; //button type, determines what to render

};