
#include "imager.h"
#include <xutility>

BImage::BImage(const int w, const int h, int bt)
{
	wi = w;
	he = h;
	ty = bt;
	base = new uint8_t[wi * he * 4];
	im5 = new uint8_t[wi * he * 4];
}

BImage::~BImage()
{
	delete[] im5;
	delete[] base;
}

void BImage::redraw(double param, int iii)
{
	uint8_t qr = 255;

	double cx = wi / 2;
	double cy = he / 2;

	double dx = wi / 3;
	double dy = he / 3;
	double da = 2 * M_PI / 3;
	double da2 = 2 * M_PI / 4;
	double da3 = 2 * M_PI / 6;

	double o = 2*M_PI*param/qr;
	double o1 = param/qr;
	double o2 = 2*M_PI*param/qr/30;
	double o3 = 2*M_PI*(param/qr+0.125);

	switch (ty) {

	case(0):
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

				base[4 * pi] = static_cast<uint8_t>((sin(col) + 1) * qr / 2);
				base[4 * pi + 1] = static_cast<uint8_t>((sin(col + o2) + 1) * qr / 2);
				base[4 * pi + 2] = static_cast<uint8_t>((sin(col + 2 * o2) + 1) * qr / 2);
				base[4 * pi + 3] = static_cast<uint8_t>(qr);
			}
		}
	}
	break;

	case(1):
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

	case(2):
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

	case(-1):
	{
		std::mt19937 gen;
		constexpr double gm = static_cast<double>(gen.max());

		for (int y = 0;y < he;y++) {
			for (int x = 0; x < wi;x++) {

				int pi = wi * y + x;

				double in = static_cast<double>(70) * fmod((pi + param) , 150) / 100;

				double col = in + 2.4;

				gen.seed(iii);
				std::vector<double> cols = hsvrgb(360*(gen() / gm), 0.3+(sin(col+o)+1)/4*(gen() / gm), 0.8 + (sin(col + o)+1)/2*0.1*(gen() / gm) );

				base[4 * pi] = static_cast<uint8_t>(cols[0]*256);
				base[4 * pi + 1] = static_cast<uint8_t>(cols[1]*256);
				base[4 * pi + 2] = static_cast<uint8_t>(cols[2]*256);
				base[4 * pi + 3] = static_cast<uint8_t>(qr);
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

void BImage::bblend(const char* f, double t)
{
	uint8_t qr = 255;

	for (size_t i = 0; i < wi* he; ++i) {
		im5[4 * i] = ((qr - t) * (qr - base[4 * i]) + t * base[4 * i]) / qr;
		im5[4*i+1] = ((qr - t) * (qr - base[4 * i+1]) + t * base[4 * i+1]) / qr;
		im5[4*i+2] = ((qr - t) * (qr - base[4 * i+2]) + t *  base[4 * i+2]) / qr;
		im5[4*i+3] = ((qr - t) * (qr - base[4 * i+3]) + t * base[4 * i+3]) / qr;
	}
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

std::vector<double> BImage::hsvrgb(double h, double s, double v)
{
	double hh, p, q, t, ff;
	long i;
	std::vector<double> out = {0,0,0};

	if (s <= 0.0) {
		out[0] = v;
		out[1] = v;
		out[2] = v;
		return out;
	}
	hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		out[0] = v;
		out[1] = t;
		out[2] = p;
		break;
	case 1:
		out[0] = q;
		out[1] = v;
		out[2] = p;
		break;
	case 2:
		out[0] = p;
		out[1] = v;
		out[2] = t;
		break;

	case 3:
		out[0] = p;
		out[1] = q;
		out[2] = v;
		break;
	case 4:
		out[0] = t;
		out[1] = p;
		out[2] = v;
		break;
	case 5:
	default:
		out[0] = v;
		out[1] = p;
		out[2] = q;
		break;
	}
	return out;
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
