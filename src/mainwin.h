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

//slider widget
class Slider : public QWidget {

	Q_OBJECT

public:

	Slider(QWidget* p, const char* kww, bool d = false);
	~Slider();
	Slider(const Slider& l)=default;
	double val = 0;
	double valt = 0; //value tween
	QLabel* lab=nullptr; //label
	QLineEdit* labt=nullptr; //label text
	QString kw = ""; //keyword (what the slider represents)
	double ext=0; //extent, max value the value can have 
	bool direction = false; //creates a vertically algined slider if true, horizontal otherwise. for now, labels are disabled if true
	bool mbs = false;

public slots:

	void check(); //checks if the value entered is a number

signals:

	void valChanged(double valyt);

protected:

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void winit();

private:

	bool mbst = false;

};

//file scroller widget
class Fscr : public QAbstractScrollArea {

	Q_OBJECT

public:

	Fscr(QWidget* p);
	~Fscr();
	Fscr(const Fscr& f) = default;

	QVector<std::filesystem::path>& gfiles(); //get files
	std::vector<std::filesystem::path>& gfilesd(); //get filesdisp
	int sel = -1; //currently selected file's index
	Slider* sl; //a slider used as a vertical scroll bar for the viewport
	std::filesystem::path proot; //project root dir
	std::filesystem::path curd; //current dir

public slots:

	void mmbrec(); //mouse middle button receive
	void sbrec(); //scroll bar receive
	void winit2(double vt);
	void searchUpdate(const QString& text);


protected:

	void scrollContentsBy(int dx, int dy) override;
	void wheelEvent(QWheelEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void winit();

private:

	QVector<std::filesystem::path> files; //file array
	std::vector<std::filesystem::path> filesdisp; //file array (the ones that are actually displayed)
	int curvl = 0; //current value
	int mousey = 0;
	QPointF lp; //last (mouse) position
	bool mbs = false; //middle button scroll (is scrolling?)
	bool mbst = false; //middle button scroll timer (is started?)
	bool mbst2 = false; //scroll bar scroll timer (is started?)
	double subcurvl = 0; //prevents a dangling value in scroll bar processing

	double tone = 0;

	QLineEdit* search; //search or filter bar for quick searching

	std::thread wt;
};

class Inter : public QWidget {

	Q_OBJECT

public:

	Inter(QWidget* p);
	~Inter();
	Inter(const Inter& i) = default;

	void hintm(const char* m); // sets hint message

	QThread* mt; //main thread
	std::string cdr = "."; //current dir
	std::filesystem::path crf = ""; //current file (to play)
	Slider* pitk = nullptr; //pitch knob
	Slider* fik = nullptr; //fade in knob
	Slider* vk = nullptr; //volume knob
	Slider* pk = nullptr; //pan knob
	APlay* pl; //audio player
	QLabel* hp = nullptr; //hint panel

	Fscr* fscr;

public slots:

	void fdret(); //file dialog return

protected:

	void resizeEvent(QResizeEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:

	QVector<Button*> buttons;
};


namespace Ut { //utility

	//set relative geometry
	QRect setRGeometry(double x, double y, double w, double h, QSize g, QSize g2 = QSize(0, 0));
}