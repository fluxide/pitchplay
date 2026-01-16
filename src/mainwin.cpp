
#include "mainwin.h"

#include <fstream>
#include <chrono>
#include <random>
#include <windows.h>

Inter::Inter(QWidget* p) : QWidget(p)
{
	resize(400, 300);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setMouseTracking(true);
	setMinimumSize(QSize(250,300));
	setMaximumSize(QSize(800,600));

	mt = QThread::currentThread(); //main thread is set

	for (int i = 0;i < static_cast<int>(Button::BType::Count);i++) { //generate button widgets

		auto* nb = new Button(this, static_cast<Button::BType>(i));
		buttons.append(nb);
	}
	fscr = new Fscr(this);
	fdret(); //refresh the fscr in case there is anything in the working dir

	//knob inits
	vk = new LKnob(this, "Volume");
	pitk = new LKnob(this, "Pitch");
	fik = new LKnob(this, "Fade In");
	pk = new LKnob(this, "Panning");
	
	//knob labels
	vk->lab = new QLabel("Volume", this);
	pitk->lab = new QLabel("Pitch", this);
	fik->lab = new QLabel("Fade In", this);
	pk->lab = new QLabel("Panning", this);
	
	//knob fonts
	vk->lab->setFont(QFont("helvetica", 6, 500));
	pitk->lab->setFont(QFont("helvetica", 6, 500));
	fik->lab->setFont(QFont("helvetica", 6, 500));
	pk->lab->setFont(QFont("helvetica", 6, 500));

	//knob extents
	vk->ext = 100;
	pitk->ext = 200;
	fik->ext = 100;
	pk->ext = 100;

	//knob default values
	vk->val = 100;
	pitk->val = 100;
	fik->val = 0;
	pk->val = 50;

	pl = new APlay(this);
}

Inter::~Inter() //deletes everything, as this closes the app
{
	for (int i = 0;i < static_cast<int>(Button::BType::Count);i++) {

		delete buttons[i];
	}

	delete fscr;
	delete vk;
	delete pitk;
	delete fik;
	delete pk;
	delete pl;
}

void Inter::resizeEvent(QResizeEvent* e) //all the resize events are managed here
{
	int ss = log(std::min(e->size().width() , e->size().height()))/log(1.1);
	int sss = pow(e->size().width(), 1.1);
	int ssss = pow(e->size().width()-sss*0.3, 1.04)*0.95;
	int l = static_cast<int>(Button::BType::Count);
	double fac = 0.5;
	double cw = ss * fac / e->size().width();
	double ch = ss * fac / e->size().height();

	double fac2 = 0.3;

	double fac3 = 0.75;
	double fac4 = 0.37;
	double cw2 = ssss * fac3 / e->size().width()*1.1;

	//button resizing
	for (int i = 0; i < l; i++) {

		buttons[i]->setGeometry(Ut::setRGeometry(1 - l * cw + i * cw, 1 - ch, fac, fac, e->size(), QSize(ss, ss)));
	}

	//fscr resizing
	fscr->setGeometry(Ut::setRGeometry(0, 0, fac2, 1, e->size(), QSize(sss, e->size().height())));
	fscr->viewport()->setGeometry(Ut::setRGeometry(0, 0, fac2, 1, e->size(), 0.99*QSize(sss, e->size().height())));

	//knob resizing, and their labels. this looks very ugly i know
	vk->setGeometry(Ut::setRGeometry(1-cw2, 0.15, fac3, 0.2, e->size(), QSize(ssss, ss)));
	pitk->setGeometry(Ut::setRGeometry(1-cw2, 0.25, fac3, 0.2, e->size(), QSize(ssss, ss)));
	fik->setGeometry(Ut::setRGeometry(1-cw2, 0.35, fac3, 0.2, e->size(), QSize(ssss, ss)));
	pk->setGeometry(Ut::setRGeometry(1-cw2, 0.45, fac3, 0.2, e->size(), QSize(ssss, ss)));
	
	vk->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.1, fac4, 0.2, e->size(), QSize(ssss, ss)));
	pitk->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.2, fac4, 0.2, e->size(), QSize(ssss, ss)));
	fik->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.3, fac4, 0.2, e->size(), QSize(ssss, ss)));
	pk->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.4, fac4, 0.2, e->size(), QSize(ssss, ss)));
	
	vk->labt->setGeometry(Ut::setRGeometry(1.1-cw2, 0.1, fac4, 0.25, e->size(), QSize(ssss, ss)));
	pitk->labt->setGeometry(Ut::setRGeometry(1.1-cw2, 0.2, fac4, 0.25, e->size(), QSize(ssss, ss)));
	fik->labt->setGeometry(Ut::setRGeometry(1.1-cw2, 0.3, fac4, 0.25, e->size(), QSize(ssss, ss)));
	pk->labt->setGeometry(Ut::setRGeometry(1.1-cw2, 0.4, fac4, 0.25, e->size(), QSize(ssss, ss)));
}

void Inter::mouseMoveEvent(QMouseEvent* e) //this catches mouse movement within the main window and calls the button animations so that they arent stuck sometimes
{
	int l = static_cast<int>(Button::BType::Count);
	
	for (int i = 0; i < l;i++) {
		buttons[i]->mouseR(e);
	}
}

void Inter::fdret() {
	
	if (QObject::sender() != nullptr) { //sender is the file dialog, catch it and get the selected directory from it
		auto* fd = static_cast<QFileDialog*>(QObject::sender());
		if (fd->selectedFiles().size() > 0)
			cdr = fd->selectedFiles()[0].toStdString();
		if(CMAKE_INTDIR=="Debug")
			qDebug() << cdr;
		fscr->dlay();
		fd->deleteLater();
	}

	auto cd = std::filesystem::path(cdr);
	std::filesystem::directory_iterator cdirit(cd);

	fscr->gfiles().clear();
	fscr->sel = -1; //nothing is selected immediately

	std::string ex[3] = {".wav",".ogg",".mp3"}; //supported format list, files are only appended if the extension is here

	for (const auto& f : cdirit) {
		if (f.is_regular_file() && std::find(std::begin(ex), std::end(ex), f.path().extension()) != std::end(ex)) {
			fscr->gfiles().append(f);
		}
	}
	fscr->gimg();

	fscr->updateGeometry();
	fscr->viewport()->update();

}

Button::Button(QWidget* p, BType ty) : QWidget(p)
{
	type = ty;
	setMouseTracking(true);
	
	im = new BImage(40, 40, type);

	mt = static_cast<Inter*>(parent())->mt;

	winit();
}

void Button::winit() //initalizes animation timers and the graphic rendering thread
{
	wt = std::thread([this]() {

		im->redraw(tone,0);

	});

	wt.detach();

	if (!bat) { //only queue the next frame if the previous timer is done
		bat = true;
		QTimer::singleShot(static_cast<int>(cw * 0.1), [this]() {

			repaint();
		});
	}
}

void Fscr::winit() //same thing but for fscr
{

	if (!mbst) {
		mbst = true;

		QTimer::singleShot(20, [this]() {

			mmbrec();
		});
	}
}

void LKnob::winit() //and for lknob
{

	if (!mbst) {
		mbst = true;

		QTimer::singleShot(20, [this]() {

			repaint();
		});
	}
}

void Fscr::imdr(int si, int ei) {
	
	double ll = 0.2 * log(std::min(width(), height())) / log(1.04);
	int l = ll * gfiles().size();

	wt = std::thread([this](int sin ,int ein, int ll) {

		for (int i = sin; i < ein; i++) { //only draws the requested interval
			if(i!=sel) 
				ims[i]->redraw(abs(255*tone/ll), i);
			else    
				ims[i]->redraw(-128*abs(tone/ll), -1-i); //if file is selected, darken it and invert hue
		
		}
	}, si, ei, l);

	wt.detach();
}

void Fscr::gimg() {
	
	for (int i = 0;i < files.size();i++) {
		ims.push_back(new BImage(40, 10, -1));
	}

	int s = files.size();

	imdr(0,std::min(s,50)); //50 is an arbitrary number here, i didnt want to calculate how many widgets are visible after a new initialization
}

Button::~Button()
{
	delete im;

}

QRect Ut::setRGeometry(double x, double y, double w, double h, QSize g, QSize g2)
{
	//it really just scales two sizes and combines them
	if (g2.isNull()) g2 = g;
	double x1 = x * g.width();
	double y1 = y * g.height();
	return QRect(x1, y1, w * g2.width(), h * g2.height());
}

void Button::mousePressEvent(QMouseEvent* e)
{
	
	targ = 0;
	fac = 0.5;
	cw = 40;
	pressing = true;

	winit();
	//nothing much is done here, actual processing happens when the button is released
}

void Button::mouseMoveEvent(QMouseEvent* e)
{
	//hover animations
	
	QPoint ori = QPoint(pos())+QPoint(2, 2);
	QRect rect2 = QRect(ori, rect().size() - QSize(4,4));

	cw = 100;

	if (rect2.contains(pos() + e->position().toPoint()) && !pressing) {
		targ = 128;
	}
	else if (!pressing) {
		targ = 255;
	}

	if (abs(targ - tone) < 1) return;

	winit();

}

void Button::mouseReleaseEvent(QMouseEvent* e)
{
	//this function includes all the processing for individual buttons

	targ = 128;
	fac = 0.3;
	cw = 40;
	pressing = false;
	emit pressed(e);

	if (e != nullptr) {

		auto* pare = static_cast<Inter*>(parent());

		switch (this->getType()) {

		case(BType::Start):
			if (CMAKE_INTDIR == "Debug")
				qDebug() << "start";
			pare->pl->play(pare->crf); //start the player and break
			break;

		case(BType::Stop):
			
			if (CMAKE_INTDIR == "Debug")
				qDebug() << "stop";
			pare->pl->stop(); //stop the player and break
			break;

		case(BType::FileD):
		{
			if (CMAKE_INTDIR == "Debug")
				qDebug() << "filed";

			//initalize file dialog and connect it to the file dialog return function
			auto* fd = new QFileDialog(this, "Choose Directory", pare->cdr.c_str(), "Sound (*.ogg, *.wav, *.mp3)");
			fd->setFileMode(QFileDialog::FileMode::Directory);
			fd->setAcceptMode(QFileDialog::AcceptOpen);
			QObject::connect(fd, &QFileDialog::finished, pare, &Inter::fdret);
			fd->open();
		}
			break;
		
		case(BType::CopyC):
		{
			//copy the corresponding event command into the clipboard. only works on windows for now
			if (CMAKE_INTDIR == "Debug") { //debug setup for reading rpgmaker clipboard content
				qDebug() << "copy";

				{
					OpenClipboard(nullptr);

					HANDLE h = GetClipboardData(566);
					if (!h) {
						printf("No data for format %u\n", 566);
						CloseClipboard();
						goto A;
					}

					void* ptr = GlobalLock(h);
					SIZE_T size = GlobalSize(h);

					printf("Size: %zu bytes\n", size);

					unsigned char* bytes = (unsigned char*)ptr;
					for (SIZE_T i = 0; i < size; i++) {
						printf("%02X ", bytes[i]);
						if ((i + 1) % 16 == 0) printf("\n");
					}
					printf("\n");

					GlobalUnlock(h);
					CloseClipboard();
				}
			}

			A:
			std::vector<uint8_t> buf; //byte array 

			auto u8 = [&](uint8_t v) { buf.push_back(v); }; //inserts one byte
			auto u16 = [&](uint16_t v) { buf.push_back(v & 0xFF); buf.push_back(v >> 8); }; //two bytes
			auto u32 = [&](uint32_t v) {
				buf.push_back(v & 0xFF);
				buf.push_back((v >> 8) & 0xFF);
				buf.push_back((v >> 16) & 0xFF);
				buf.push_back((v >> 24) & 0xFF);
			}; //four bytes

			auto command = [&](QByteArray fname, bool ct, int vol, int pit, int pan, int fin) { //actually write the data 

				//ct is code type, if its true we generate a play bgm command. play sfx command otherwise
				if (ct) u16(0x76D9); //2 byte code for playing bgm
				else u16(0x1EDA); //code for sfx
				u8(0); //indent level for the command
				u8(fname.size()); //size of the string param
				buf.insert(buf.end(), fname.begin(), fname.end()); //writes the chars in cp1252 format
				u8(ct? 6:5); //parameter number? 
				if(ct) u8(0); // fade in time, i couldnt decode how it works so hardcoded to 0 for now, for bgm
				u8(vol); //volume
				if (pit > 129) { u8(129); u8(pit - 128); } //pitch, pitch value is 2 bytes when its higher than 129
				else u8(pit); //single byte otherwise
				u8(pan); //panning
				u8(0); //some other parameter, i think it has to do with if parameters are constants or variables
				u8(0); //termination
			};

			QString fstr2 = QString::fromStdString(static_cast<Inter*>(parent())->crf.filename().string());
			QString fstr = QFileInfo(fstr2).completeBaseName(); //gets filename without extension

			if (fstr.toStdString() == "") break;

			buf.clear();
			//call the constructor with knob values and the filename. left mouse button currently copies bgm command, and its sfx command otherwise 
			command(fstr.toLocal8Bit(), e->button() == Qt::MouseButton::LeftButton, pare->vk->val, pare->pitk->val, pare->pk->val, pare->fik->val);

			//the size of the buffer must be added at the start of the buffer (excluding the 4 bytes used for it)
			uint32_t len = static_cast<uint32_t>(buf.size());
			uint8_t* lenv = (uint8_t*)&len;
			buf.insert(buf.begin(), lenv, lenv+4); //the final buffer is <data size>+4 bytes

			UINT fmt = 566; //commands are copied into this clipboard format id on windows
			OpenClipboard(0);
			EmptyClipboard();
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len+4);
			memcpy(GlobalLock(hMem), buf.data(), len+4);
			GlobalUnlock(hMem);
			SetClipboardData(fmt, hMem);
			CloseClipboard();
		}
			break;
		}
	}

	winit();
}

void Button::paintEvent(QPaintEvent* e)
{
	//gets the button icon and renders it with a painter
	bat = false;
	if (e != nullptr) {
		
		tone += (targ - tone) * fac * 0.2;
		body = im->getimp();

		QImage body2(body, im->wi, im->he, QImage::Format_RGBA8888);

		QPainter painter(this);
		painter.drawImage(e->rect().topLeft(), body2.scaled(rect().size()));
		painter.end();
	}

	if (abs(targ - tone) > 1) //requeue the animation loop if the target tone hasnt been reached 
		winit();
}

QSize Button::sizeHint() const
{
	return QSize(width(), height());
}

void Button::mouseR(QMouseEvent* e)
{
	mouseMoveEvent(e);
}

void Fscr::mmbrec()
{
	mbst = false;
	double drag = mousey - lp.y();
	scrollContentsBy(0, drag/10);
	
	if (mbs) winit();

}

Fscr::Fscr(QWidget* p) : QAbstractScrollArea(p)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //no scrollbars, for a simpler ui
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setMouseTracking(true);

	winit();
}

Fscr::~Fscr()
{
	dlay();
}

void Fscr::dlay()
{
	//deletes all the widget images
	for (int i = 0;i < ims.size();i++) {
		delete ims[i];
	}
	
	ims.clear();
}

QVector<std::filesystem::path>& Fscr::gfiles()
{
	return files;
}

void Fscr::scrollContentsBy(int dx, int dy)
{
	//updates curvl and updates the viewport (calls paintEvent) 
	double ll =  0.2 * log(std::min(width(), height())) / log(1.04);;
	int l = static_cast<int>(ll*files.size());
	curvl += static_cast<int>(dy*ll/2);
	
	//wrap around
	if (curvl < (-l)) {
		curvl = 0;
		tone += l;
	} 
	
	if (curvl > l) {
		curvl = 0;
		tone -= l;
	}

	viewport()->update();
}

void Fscr::wheelEvent(QWheelEvent* e)
{
	//code copied from qt docs
	QPoint numPixels = -e->pixelDelta();
	QPoint numDegrees = e->angleDelta() / -8;

	if (!numPixels.isNull()) {
		scrollContentsBy(numPixels.x(),numPixels.y());
	}
	else if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;
		scrollContentsBy(numSteps.x(),numSteps.y());
	}

	e->accept();
}

void Fscr::resizeEvent(QResizeEvent* e)
{
	//this used to be for my custom resizing function which i have deleted
	resize(e->size());
}

void Fscr::paintEvent(QPaintEvent* e)
{
	tone += (curvl - tone) * 0.3; //tween the parameter

	QPainter painter(this->viewport());
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.setFont(QFont("Helvetica",8,500));
	QTextOption to;
	to.setAlignment(Qt::AlignCenter);
	to.setWrapMode(QTextOption::WrapAnywhere);
	double ll = 0.2 * log(std::min(width(), height())) / log(1.04);
	int co = -1;
	int l = ll * gfiles().size();

	if (!(l > 0)) return; //return if no files, and avoid 0 division

	tone = fmod(tone,l);

	for (int ii = tone; ii < e->rect().height() + tone; ii++) { //paints scanlines on the viewport
		
		int i = fmod(ii + l, l);
		double i2 = fmod(ii - tone, l);

		int ci = floor(i / ll); // current index, index of which widget the current scanline falls 
		int ei = fmod((e->rect().height()/ll+1),gfiles().size());
		
		if (ci != co) { //co is current (index) old
			
			imdr(ci, ci+1); //redraw images one at a time
			if(ci != sel) //invert text color if selected
				painter.setPen(QColor(0, 0, 0));
			else painter.setPen(QColor(255, 255, 255));

			QImage bs(ims[ci]->getimp(), 40, 10, QImage::Format_RGBA8888);
			painter.drawImage(QPointF(0,i2), bs.scaled(QSize(e->rect().width(), ll*0.9)));
			painter.drawText(QRect(0, i2 , e->rect().width(), ll), files[ci].filename().generic_string().c_str(), to);
			co = ci;
		}

	}
	
	painter.end();

	if (abs(curvl - tone) > 1) //recall if target isnt reached yet
		winit();
}

void Fscr::mousePressEvent(QMouseEvent* e)
{
	double ll = 0.2 * log(std::min(width(), height())) / log(1.04);
	int l = ll * gfiles().size();
	int cy = floor(fmod(curvl + e->pos().y() + l, l) / ll);

	if (l>0 && cy<=gfiles().size() && e->button() == Qt::MouseButton::LeftButton) {
		
		//left button sets the current file and the selected widget, and stops the player
		if (CMAKE_INTDIR == "Debug")
			qDebug() << files[cy].generic_string().c_str();
		static_cast<Inter*>(parent())->crf = files[cy].generic_string().c_str();
		static_cast<Inter*>(parent())->pl->stop();
		sel = cy;
		imdr(cy, cy + 1);
		viewport()->update();
	}
	
	if (e->button() == Qt::MouseButton::MiddleButton) {

		//middle button initializes the scrolling loop
		lp = e->pos();
		mousey = e->pos().y();


		if (!mbs) {
			mbs = true;
			winit();
		}
	}

}

void Fscr::mouseMoveEvent(QMouseEvent* e)
{
	//only matters for middle button

	if (mbs) {
		mousey = e->pos().y();
		winit();
	}
}

void Fscr::mouseReleaseEvent(QMouseEvent* e)
{
	//only matters for middle button

	if (mbs) {
		mbs = false;
		lp = e->pos();
	}
}

LKnob::LKnob(QWidget* p, const char* kww) : QWidget(p)
{
	setMouseTracking(true);
	kw = kww;
	labt = new QLineEdit(parentWidget());
	labt->setFont(QFont("helvetica", 6, 500));
	QObject::connect(labt, &QLineEdit::editingFinished, this, &LKnob::check); //check when a new value is entered
}

LKnob::~LKnob()
{
	lab->deleteLater();
	labt->deleteLater();
}

void LKnob::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton) {

		//modify the knob value

		val = static_cast<double>(e->pos().x()) / width() * ext;
		
		double min = kw == "Pitch" ? 10.0 : 0.0; //pitch knob has a min value

		val = std::clamp(floor(val /10) * 10, min, ext); //snapping and clamping

		if (!mbs) {
			mbs = true;
			winit();
		}
	}
}

void LKnob::mouseMoveEvent(QMouseEvent* e)
{
	if (mbs) {
		
		//value is modified with the mouse movement
		double min = kw == "Pitch" ? 10.0 : 0.0; //pitch knob has a min value

		val = static_cast<double>(e->pos().x()) / width() *ext;
		val = std::clamp(floor(val/10)*(10), min, ext);
		winit();
	}
}

void LKnob::mouseReleaseEvent(QMouseEvent* e)
{
	if (mbs) {
		mbs = false;
	}
}

void LKnob::paintEvent(QPaintEvent* e)
{
	mbst = false;
	valt += (val - valt) * 0.5;
	QPainter painter(this);
	painter.setBackgroundMode(Qt::TransparentMode);
	std::vector<uint8_t> d = { 0,0,0,255 };
	QImage body(d.data(), 1, 1, QImage::Format_RGBA8888);
	
	for (int i = 0; i < e->rect().width(); i++) {
		
		int ma = 3; //margin (makes the knob cursor appear bigger than the knob line)
		double c = std::clamp(2*log(1000 *((static_cast<double>(i) / e->rect().width() + valt/ext * 0.09))) / log(1.2), 64.0, 192.0);

		if (abs(static_cast<double>(i) / e->rect().width() - valt/ext) < 0.05)
		{
			//render the cursor
			ma = 0;
			c = 192 - c;
		}

		if (fmod((ext/10) * static_cast<double>(i) / e->rect().width(), 1) < 0.15) {
			//render the snap/grid lines
			c = 192 - c;
		}

		painter.setPen(QColor(c,c,c));
		painter.drawLine(QPointF(i,ma), QPointF(i,e->rect().height()-ma));

	}

	labt->setText(std::to_string(val).c_str()); //update the label with value change
	painter.end();

	if (abs(val - valt) > 0.01) //recall if target is not reached yet
		winit();
}

void LKnob::check() {

	auto* p = static_cast<QLineEdit*>(QObject::sender());
	std::string et = p->text().toStdString();
	
	if (!et.empty() && std::find_if(et.begin(), et.end(), [](unsigned char c) { return !(std::isdigit(c)|| c == '.'); }) == et.end() && std::stod(et) <= ext && std::stod(et) >= (kw=="Pitch")?1:0) {
		//checks if the entered value is a valid number, and whether its also between allowed values
		val = std::stod(et);
		p->setText(et.c_str());
	}
	else {
		//dont update value if invalid
		p->setText(std::to_string(val).c_str());
	}
	update();
}