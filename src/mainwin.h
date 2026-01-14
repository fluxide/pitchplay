#pragma once

#include "header.h"
#include "imager.h"
#include "player.h" 

class Worker : public QObject {

	Q_OBJECT

public:

	Worker(QWidget* p, QThread* mtt);
	~Worker() = default;
	bool isValid();

	bool alive = false;

signals:

	void repaint();
	void mmbe();

public slots:

	void anim(int w, double ta, double to, bool p);
	void mmbrec2(bool m);

private:

	QThread* mt = nullptr;

};

class Button : public QWidget {

	Q_OBJECT

public:

	enum BType { Start = 0, Stop = 1, FileD = 2, Count = 3 };
	Q_ENUM(BType)

		Button(QWidget* p, BType ty);
	~Button();
	Button(const Button& b) = default;

	BType getType() const { return type; }
	bool getifActive() const { return active; }
	bool getifPressing() const { return pressing; }

	QSize sizeHint() const;


public slots:

	void wterm();
	void mouseR(QMouseEvent* e);

signals:

	void pressed(QMouseEvent* e);
	void recalc(int w, double ta, double to, bool p);

protected:

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void winit();


private:

	bool pressing = false;
	bool active = false;
	bool bat = false;
	BType type = BType::Start;
	uint8_t* body = nullptr;
	Worker* bw = nullptr;
	std::thread wt;
	QThread* mt = nullptr;
	BImage* im;


	double tone = 255;
	double targ = 255;
	double fac = 0.3;
	int cw = 0;

};

class Filew : public QWidget {

	Q_OBJECT

public:

	Filew(QWidget* p, std::filesystem::path fn);
	~Filew();
	Filew(const Filew& f) = default;

	QSize sizeHint() const;

	std::filesystem::path pat;

protected:

	void paintEvent(QPaintEvent* e) override;

};

class Fscr : public QAbstractScrollArea {

	Q_OBJECT

public:

	Fscr(QWidget* p);
	~Fscr();
	Fscr(const Fscr& f) = default;
	void resize(QSize s);

	void dlay();
	void gimg();
	QVector<Filew*>& gfiles();

public slots:

	void mmbrec();

protected:

	void scrollContentsBy(int dx, int dy) override;
	void wheelEvent(QWheelEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void winit();
	void imdr(int si, int ei);
	void wterm();

private:

	QVector<Filew*> files;
	std::vector<BImage*> ims;
	int curvl = 0;
	int mousey = 0;
	QPointF lp;
	bool mbs=false;
	bool mbst=false;

	double tone = 0;

	Worker* wor = nullptr;
	std::thread wt;
};

class LKnob : public QWidget {

	Q_OBJECT

public:

	LKnob(QWidget* p);
	~LKnob();
	LKnob(const LKnob& l)=default;
	double val = 0;
	double valt = 0;
	QLabel* lab=nullptr;

protected:

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void winit();

private:

	bool mbs = false;
	bool mbst = false;

};

class Inter : public QWidget {

	Q_OBJECT

public:

	Inter(QWidget* p);
	~Inter();
	Inter(const Inter& i) = default;

	QThread* mt;
	std::string cdr = ".";
	std::filesystem::path crf = "";
	LKnob* pitk = nullptr;
	LKnob* fik = nullptr;
	LKnob* vk = nullptr;
	APlay* pl;


public slots:

	void fdret();

protected:

	void resizeEvent(QResizeEvent* e) override;
	void moveEvent(QMoveEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:

	QVector<Button*> buttons;

	QFile* cur = nullptr;
	Fscr* fscr;
};


namespace Ut {

	QRect setRGeometry(double x, double y, double w, double h, QSize g, QSize g2 = QSize(0, 0));
}