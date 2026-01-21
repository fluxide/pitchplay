
#include "imager.h"
#include <xutility>

BImage::BImage(const int w, const int h, int bt)
{
	wi = w;
	he = h;
	ty = bt;
	base = new uint8_t[wi * he * 4]; //size is not dynamic
}

BImage::~BImage()
{
	delete[] base;
}

void BImage::redraw(double param, int iii)
{
	//param is a general purpose parameter value. mostly in 0-255 range. iii is used specifically for fscr rendering
	uint8_t qr = 255; //parameter max value

	double cx = wi / 2; //center x
	double cy = he / 2; //center y

	double dx = wi / 3; //delta x
	double dy = he / 3; //delta y
	double da = 2 * M_PI / 3; //delta angle 1,2,3
	double da2 = 2 * M_PI / 4;
	double da3 = 2 * M_PI / 6;

	double o = 2*M_PI*param/qr; //modified param values
	double o1 = param/qr;
	double o2 = 2*M_PI*param/qr/30;
	double o3 = 2*M_PI*(param/qr+0.125);

	switch (ty) { //what to render based on button type

	case(0): //start button, its a simple triangle 
	{
		fPoint p1(cx + dx * cos(da + o), cy + dy * sin(da + o));
		fPoint p2(cx + dx * cos(2 * da + o), cy + dy * sin(2 * da + o));
		fPoint p3(cx + dx * cos(3 * da + o), cy + dy * sin(3 * da + o));

		for (int y = 0;y < he;y++) {
			for (int x = 0; x < wi;x++) {

				int pi = wi * y + x;

				fPoint p(x, y);

				int in = static_cast<int>(PointInTriangle(p, p1, p2, p3));

				double col = in;

				//pixel data is arranged in RGBA, one byte for each
				base[4 * pi] = static_cast<uint8_t>((sin(col) + 1) * qr / 2); //red 
				base[4 * pi + 1] = static_cast<uint8_t>((sin(col + o2) + 1) * qr / 2); //green
				base[4 * pi + 2] = static_cast<uint8_t>((sin(col + 2 * o2) + 1) * qr / 2); //blue
				base[4 * pi + 3] = static_cast<uint8_t>(qr); //alpha, mostly opaque
			}
		}
	}
	break;

	case(1): //stop button, a square
	{
		fPoint p1(cx + dx * cos(da2 + o3), cy + dy * sin(da2 + o3));
		fPoint p2(cx + dx * cos(2 * da2 + o3), cy + dy * sin(2 * da2 + o3));
		fPoint p3(cx + dx * cos(3 * da2 + o3), cy + dy * sin(3 * da2 + o3));
		fPoint p4(cx + dx * cos(4 * da2 + o3), cy + dy * sin(4 * da2 + o3));

		for (int y = 0;y < he;y++) {
			for (int x = 0; x < wi;x++) {

				int pi = wi * y + x;

				fPoint p(x, y);

				int in = static_cast<int>(PointInTriangle(p, p1, p2, p3) || PointInTriangle(p, p1, p3, p4));

				double col = in + 1.4;

				base[4 * pi] = static_cast<uint8_t>((sin(col) + 1) * qr / 2);
				base[4 * pi + 1] = static_cast<uint8_t>((sin(col + o2) + 1) * qr / 2);
				base[4 * pi + 2] = static_cast<uint8_t>((sin(col + 2 * o2) + 1) * qr / 2);
				base[4 * pi + 3] = static_cast<uint8_t>(qr);
			}
		}
	}
	break;

	case(2): //file dialog button, a hexagon
	{
		fPoint p1(cx + dx * cos(da3 + o), cy + dy * sin(da3 + o));
		fPoint p2(cx + dx * cos(2 * da3 + o), cy + dy * sin(2 * da3 + o));
		fPoint p3(cx + dx * cos(3 * da3 + o), cy + dy * sin(3 * da3 + o));
		fPoint p4(cx + dx * cos(4 * da3 + o), cy + dy * sin(4 * da3 + o));
		fPoint p5(cx + dx * cos(5 * da3 + o), cy + dy * sin(5 * da3 + o));
		fPoint p6(cx + dx * cos(6 * da3 + o), cy + dy * sin(6 * da3 + o));

		for (int y = 0;y < he;y++) {
			for (int x = 0; x < wi;x++) {

				int pi = wi * y + x;

				fPoint p(x, y);

				int in = static_cast<int>(PointInTriangle(p, p1, p2, p3) ||
					PointInTriangle(p, p1, p3, p4) ||
					PointInTriangle(p, p1, p4, p5) ||
					PointInTriangle(p, p1, p5, p6));

				double col = in + 2.4;

				base[4 * pi] = static_cast<uint8_t>((sin(col) + 1) * qr / 2);
				base[4 * pi + 1] = static_cast<uint8_t>((sin(col + o2) + 1) * qr / 2);
				base[4 * pi + 2] = static_cast<uint8_t>((sin(col + 2 * o2) + 1) * qr / 2);
				base[4 * pi + 3] = static_cast<uint8_t>(qr);
			}
		}
	}
	break;

	case(3): //copy command button, a square hole
	{
		std::vector<double> cols1 = hsvrgb(fmod(o1 * 360 + 0 * 180, 360), 0.3, 0.8);
		std::vector<double> cols2 = hsvrgb(fmod(o1 * 360 + 1 * 180, 360), 0.3, 0.8);
		fPoint p1(cx + dx * cos(da2 + o), cy + dy * sin(da2 + o));
		fPoint p2(cx + dx * cos(2 * da2 + o), cy + dy * sin(2 * da2 + o));
		fPoint p3(cx + dx * cos(3 * da2 + o), cy + dy * sin(3 * da2 + o));
		fPoint p4(cx + dx * cos(4 * da2 + o), cy + dy * sin(4 * da2 + o));
		fPoint p5(cx + 0.5 * dx * cos(1 * da2 + o), cy + 0.5 * dy * sin(1 * da2 + o));
		fPoint p6(cx + 0.5 * dx * cos(2 * da2 + o), cy + 0.5 * dy * sin(2 * da2 + o));
		fPoint p7(cx + 0.5 * dx * cos(3 * da2 + o), cy + 0.5 * dy * sin(3 * da2 + o));
		fPoint p8(cx + 0.5 * dx * cos(4 * da2 + o), cy + 0.5 * dy * sin(4 * da2 + o));

		for (int y = 0;y < he;y++) {
			for (int x = 0; x < wi;x++) {

				int pi = wi * y + x;

				fPoint p(x, y);

				int in = static_cast<int>(PointInTriangle(p, p1, p5, p6) ||
					PointInTriangle(p, p1, p2, p6) ||
					PointInTriangle(p, p2, p6, p7) ||
					PointInTriangle(p, p2, p3, p7) ||
					PointInTriangle(p, p3, p7, p8) ||
					PointInTriangle(p, p3, p4, p8) ||
					PointInTriangle(p, p4, p8, p5) ||
					PointInTriangle(p, p4, p1, p5));

				if (in) {

					base[4 * pi] = static_cast<uint8_t>(fmod(cols1[0] * 256, 256));
					base[4 * pi + 1] = static_cast<uint8_t>(fmod(cols1[1] * 256, 256));
					base[4 * pi + 2] = static_cast<uint8_t>(fmod(cols1[2] * 256, 256));
					base[4 * pi + 3] = static_cast<uint8_t>(qr);
				}
				else {
					base[4 * pi] = static_cast<uint8_t>(fmod(cols2[0] * 256, 256));
					base[4 * pi + 1] = static_cast<uint8_t>(fmod(cols2[1] * 256, 256));
					base[4 * pi + 2] = static_cast<uint8_t>(fmod(cols2[2] * 256, 256));
					base[4 * pi + 3] = static_cast<uint8_t>(qr);
				}
			}
		}
	}
	break;
	}
}

uint8_t* BImage::getimp()
{
	return base;
}

float BImage::sign(fPoint p1, fPoint p2, fPoint p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

std::vector<uint8_t> BImage::rgbhsv(double r, double g, double b)
{
	double maxc = std::max(r / 256, std::max(g / 256, b / 256));
	double minc = std::min(r / 256, std::min(g / 256, b / 256));
	uint8_t v = static_cast<uint8_t>(256 * maxc);
	if (minc == maxc)
		return { 0, 0, v };
	double s = (maxc - minc) / maxc;
	double rc = (maxc - r) / (maxc - minc);
	double gc = (maxc - g) / (maxc - minc);
	double bc = (maxc - b) / (maxc - minc);
	double h = 0;
	if (r == maxc)
		h = bc - gc;
	else if (g == maxc)
		h = 2.0 + rc - bc;
	else
		h = 4.0 + gc - rc;

	h = fmod((h / 6.0), 1.0);

	return {static_cast<uint8_t>(h * 256), static_cast<uint8_t>(s * 256), v};
}

std::vector<double> BImage::hsvrgb(double H, double S, double V)
{
	double r, g, b;

	float h = H / 360;
	float s = S / 1;
	float v = V / 1;

	int i = floor(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	switch (i % 6) {
	case 0: r = v, g = t, b = p; break;
	case 1: r = q, g = v, b = p; break;
	case 2: r = p, g = v, b = t; break;
	case 3: r = p, g = q, b = v; break;
	case 4: r = t, g = p, b = v; break;
	case 5: r = v, g = p, b = q; break;
	}

	std::vector<double> color = {0,0,0};
	color[0] = r;
	color[1] = g;
	color[2] = b;

	return color;
}

bool BImage::PointInTriangle(fPoint pt, fPoint v1, fPoint v2, fPoint v3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sign(pt, v1, v2);
	d2 = sign(pt, v2, v3);
	d3 = sign(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

BImage::fPoint::fPoint(int x1, int y1)
{
	x = x1;
	y = y1;
}
