#pragma once

#include "header.h"
#include "imager.h"
#include "player.h" 

//generic button object
class Button : public QWidget {

	Q_OBJECT

public:

	enum BType { Start = 0, Stop = 1, FileD = 2, CopyC = 3, Count = 4 }; //adding more enums here automatically adds more buttons to the ui
	Q_ENUM(BType)

		Button(QWidget* p, BType ty);
	~Button();
	Button(const Button& b) = default;

	BType getType() const { return type; }
	bool getifActive() const { return active; }
	bool getifPressing() const { return pressing; }

	QSize sizeHint() const;


public slots:

	void mouseR(QMouseEvent* e); //mouse Receive

signals:

	void pressed(QMouseEvent* e);
	void recalc(int w, double ta, double to, bool p); 

protected:

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void winit(); //work init


private:

	bool pressing = false;
	bool active = false;
	bool bat = false; //button animation timer (started or not)
	BType type = BType::Start;
	uint8_t* body = nullptr;
	std::thread wt; //work thread, for graphics
	QThread* mt = nullptr; //main thread
	BImage* im;


	double tone = 255;
	double targ = 255; //target tone
	double fac = 0.3;
	int cw = 0;

};

//file scroller widget
class Fscr : public QAbstractScrollArea {

	Q_OBJECT

public:

	Fscr(QWidget* p);
	~Fscr();
	Fscr(const Fscr& f) = default;

	void dlay(); //delete layout
	void gimg(); //generate images
	QVector<std::filesystem::path>& gfiles(); //get files
	int sel = -1; //currently selected file's index

public slots:

	void mmbrec(); //mouse middle button receive

protected:

	void scrollContentsBy(int dx, int dy) override;
	void wheelEvent(QWheelEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void winit();
	void imdr(int si, int ei); //image draw

private:

	QVector<std::filesystem::path> files; //file array
	std::vector<BImage*> ims; 
	int curvl = 0; //current value
	int mousey = 0; 
	QPointF lp;
	bool mbs=false; //middle button scroll (is scrolling?)
	bool mbst=false; //middle button scroll timer (is started?)

	double tone = 0;

	std::thread wt;
};

//linear knob widget
class LKnob : public QWidget {

	Q_OBJECT

public:

	LKnob(QWidget* p, const char* kww);
	~LKnob();
	LKnob(const LKnob& l)=default;
	double val = 0;
	double valt = 0; //value tween
	QLabel* lab=nullptr; //label
	QLineEdit* labt=nullptr; //label text
	QString kw = ""; //keyword (what the knob represents)
	double ext=0; //extent 

public slots:

	void check(); //checks if the value entered is a number

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

	QThread* mt; //main thread
	std::string cdr = "."; //current dir
	std::filesystem::path crf = ""; //current file (to play)
	LKnob* pitk = nullptr; //pitch knob
	LKnob* fik = nullptr; //fade in knob
	LKnob* vk = nullptr; //volume knob
	LKnob* pk = nullptr; //pan knob
	APlay* pl; //audio player


public slots:

	void fdret(); //file dialog return

protected:

	void resizeEvent(QResizeEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:

	QVector<Button*> buttons;

	Fscr* fscr; 
};


namespace Ut { //utility

	//set relative geometry
	QRect setRGeometry(double x, double y, double w, double h, QSize g, QSize g2 = QSize(0, 0));
}