
#include "mainwin.h"

#include <fstream>
#include <chrono>
#include <random>

Inter::Inter(QWidget* p) : QWidget(p)
{
	resize(200, 150);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setMouseTracking(true);
	setMinimumSize(size());
	setMaximumSize(QSize(800,600));

	mt = QThread::currentThread();

	for (int i = 0;i < static_cast<int>(Button::BType::Count);i++) {

		auto* nb = new Button(this, static_cast<Button::BType>(i));
		buttons.append(nb);
	}
	fscr = new Fscr(this);
	fdret();

	vk = new LKnob(this);
	pitk = new LKnob(this);
	fik = new LKnob(this);
	
	vk->lab = new QLabel("Volume", this);
	pitk->lab = new QLabel("Pitch", this);
	fik->lab = new QLabel("Fade In", this);
	
	vk->lab->setFont(QFont("helvetica", 6, 500));
	pitk->lab->setFont(QFont("helvetica", 6, 500));
	fik->lab->setFont(QFont("helvetica", 6, 500));

	vk->val = 1;
	pitk->val = 0.5;
	fik->val = 0;

	pl = new APlay(this);
}

Inter::~Inter()
{
	for (int i = 0;i < static_cast<int>(Button::BType::Count);i++) {

		delete buttons[i];
	}

	delete fscr;
	delete vk;
	delete pitk;
	delete fik;
	delete pl;
}

void Inter::resizeEvent(QResizeEvent* e)
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
	double cw2 = ssss * fac3 / e->size().width()*1.1;

	for (int i = 0; i < l; i++) {

		buttons[i]->setGeometry(Ut::setRGeometry(1 - l * cw + i * cw, 1 - ch, fac, fac, e->size(), QSize(ss, ss)));
	}

	fscr->setGeometry(Ut::setRGeometry(0, 0, fac2, 1, e->size(), QSize(sss, e->size().height())));
	vk->setGeometry(Ut::setRGeometry(1-cw2, 0.15, fac3, 0.2, e->size(), QSize(ssss, ss)));
	pitk->setGeometry(Ut::setRGeometry(1-cw2, 0.25, fac3, 0.2, e->size(), QSize(ssss, ss)));
	fik->setGeometry(Ut::setRGeometry(1-cw2, 0.35, fac3, 0.2, e->size(), QSize(ssss, ss)));
	vk->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.1, fac3, 0.2, e->size(), QSize(ssss, ss)));
	pitk->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.2, fac3, 0.2, e->size(), QSize(ssss, ss)));
	fik->lab->setGeometry(Ut::setRGeometry(1-cw2, 0.3, fac3, 0.2, e->size(), QSize(ssss, ss)));
}

void Inter::moveEvent(QMoveEvent* e)
{

}

void Inter::mouseMoveEvent(QMouseEvent* e) 
{
	int l = static_cast<int>(Button::BType::Count);
	
	for (int i = 0; i < l;i++) {
		buttons[i]->mouseR(e);
	}
}

void Inter::fdret() {
	
	if (QObject::sender() != nullptr) {
		auto* fd = static_cast<QFileDialog*>(QObject::sender());
		if (fd->selectedFiles().size() > 0)
			cdr = fd->selectedFiles()[0].toStdString();
		qDebug() << cdr;
		fscr->dlay();
		fd->deleteLater();
	}

	auto cd = std::filesystem::path(cdr);
	std::filesystem::directory_iterator cdirit(cd);

	fscr->gfiles().clear();

	std::string ex[3] = {".wav",".ogg",".mp3"};

	for (const auto& f : cdirit) {
		if (f.is_regular_file() && std::find(std::begin(ex), std::end(ex), f.path().extension()) != std::end(ex)) {
			fscr->gfiles().append(new Filew(fscr->viewport(), f));
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
	
	im = new BImage(100, 100, type);

	mt = static_cast<Inter*>(parent())->mt;

	winit();
}

void Button::winit() 
{
	wt = std::thread([this]() {

		im->redraw(tone,0);

	});

	wt.detach();

	if (!bat) {
		bat = true;
		QTimer::singleShot(static_cast<int>(cw * 0.1), [this]() {

			repaint();
		});
	}
}

void Fscr::winit()
{

	if (!mbst) {
		mbst = true;

		QTimer::singleShot(20, [this]() {

			mmbrec();
		});
	}
}

void LKnob::winit()
{

	if (!mbst) {
		mbst = true;

		QTimer::singleShot(20, [this]() {

			repaint();
		});
	}
}

void Fscr::imdr(int si, int ei) {
	
	double ll = 0.2 * log(std::min(width(), height())) / log(1.04);;
	int l = ll * viewport()->children().size();

	wt = std::thread([this](int sin ,int ein, int ll) {

		for (int i = sin; i < ein; i++) {
			ims[i]->redraw(255*tone/ll, i);
		}
	}, si, ei, l);

	wt.detach();
}

void Fscr::gimg() {
	
	for (int i = 0;i < files.size();i++) {
		ims.push_back(new BImage(40, 10, -1));
	}

	int s = files.size();

	imdr(0,std::min(s,50));
}

Button::~Button()
{
	delete im;
	wterm();

}

QRect Ut::setRGeometry(double x, double y, double w, double h, QSize g, QSize g2)
{
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
}

void Button::mouseMoveEvent(QMouseEvent* e)
{
	
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

	targ = 128;
	fac = 0.3;
	cw = 40;
	pressing = false;
	emit pressed(e);

	if (e != nullptr) {

		auto pare = static_cast<Inter*>(parent());

		switch (this->getType()) {

		case(BType::Start):
			qDebug() << "start";
			pare->pl->play(pare->crf);
			break;

		case(BType::Stop):
			qDebug() << "stop";
			pare->pl->stop();
			break;

		case(BType::FileD):
			qDebug() << "filed";

			auto* fd = new QFileDialog(this, "Choose Directory", pare->cdr.c_str(), "Sound (*.ogg, *.wav, *.mp3)");
			fd->setFileMode(QFileDialog::FileMode::Directory);
			fd->setAcceptMode(QFileDialog::AcceptOpen);
			QObject::connect(fd, &QFileDialog::finished, pare, &Inter::fdret);
			fd->open();

			break;
		}
	}

	winit();
}

void Button::paintEvent(QPaintEvent* e)
{
	bat = false;
	if (e != nullptr) {
		
		tone += (targ - tone) * fac * 0.2;
		body = im->getimp();

		QImage body2(body, im->wi, im->he, QImage::Format_RGBA8888);

		QPainter painter(this);
		painter.drawImage(e->rect().topLeft(), body2.scaled(rect().size()));
		painter.end();
	}

	if (abs(targ - tone) > 1)
		winit();
}

QSize Button::sizeHint() const
{
	return QSize(width(), height());
}

QSize Filew::sizeHint() const
{
	return QSize(width(), height());
}


void Button::mouseR(QMouseEvent* e)
{
	mouseMoveEvent(e);
}

void Button::wterm()
{

	if (bw != nullptr && bw->alive) {
		bw->alive = false;
	}

	if (bw != nullptr && !bw->alive) {
		delete bw;
		bw = nullptr;
	}
}

void Fscr::wterm()
{

	if (wor != nullptr && wor->alive) {
		wor->alive = false;
	}

	if (wor != nullptr && !wor->alive) {
		delete wor;
		wor = nullptr;
	}
}

void Worker::anim(int w, double ta, double to, bool p)
{
	if (this == nullptr) return;

	QThread* mmm = mt;

	if (abs(ta - to) > 1 && !p) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(w * 0.02)));
	}

	if (mt != mmm) 
		return;

	emit repaint();
	deleteLater();
}

Worker::Worker(QWidget* p, QThread* mtt)
{
	alive = true;
	mt = mtt;
}

bool Worker::isValid()
{
	return (mt!=nullptr);
}

void Fscr::mmbrec()
{
	mbst = false;
	double drag = mousey - lp.y();
	scrollContentsBy(0, drag/10);
	
	if (mbs) winit();

}

void Worker::mmbrec2(bool m)
{

	if (this == nullptr) return;
	QThread* mmm = mt;

	if (!m) return;
	QThread::currentThread()->sleep(std::chrono::nanoseconds(2000000));

	if (mt != mmm)
		return;

	emit mmbe();
}

Filew::Filew(QWidget* p, std::filesystem::path fn) : QWidget(p)
{
	pat = fn;
}

Filew::~Filew()
{

}

void Filew::paintEvent(QPaintEvent* e)
{

}

Fscr::Fscr(QWidget* p) : QAbstractScrollArea(p)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setMouseTracking(true);

	winit();
}

Fscr::~Fscr()
{
	dlay();
	wterm();
}

void Fscr::resize(QSize s)
{
	QObjectList fl = viewport()->children();
	double ss = log(std::min(s.width(), s.height())) / log(1.04);
	int hs = ss * 0.2;


	for (int i = 0; i < fl.size();i++) {
		if (fl[i]->isWidgetType()) {
			Filew* w = static_cast<Filew*>(fl[i]);
			w->setGeometry(0, (i - 1) * hs, s.width(), hs);
		}
	}

}

void Fscr::dlay()
{
	QObjectList fl = viewport()->children();

	for (int i = 0; i < fl.size();i++) {
		if (fl[i]->isWidgetType()) {
			Filew* w = static_cast<Filew*>(fl[i]);
			w->deleteLater();
		}
	}

	for (int i = 0;i < ims.size();i++) {
		delete ims[i];
	}
	
	ims.clear();

}

QVector<Filew*>& Fscr::gfiles()
{
	return files;
}

void Fscr::scrollContentsBy(int dx, int dy)
{
	double ll =  0.2 * log(std::min(width(), height())) / log(1.04);;
	int l = static_cast<int>(ll*files.size());
	curvl += static_cast<int>(dy*ll/2);
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
	resize(e->size());
}

void Fscr::paintEvent(QPaintEvent* e)
{
	tone += (curvl - tone) * 0.3;

	QPainter painter(this->viewport());
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.setPen(QColor(0,0,0));
	painter.setFont(QFont("Helvetica",8,500));
	QTextOption to;
	to.setAlignment(Qt::AlignCenter);
	to.setWrapMode(QTextOption::WrapAnywhere);
	double ll = 0.2 * log(std::min(width(), height())) / log(1.04);
	int co = -1;
	int l = ll * viewport()->children().size();

	if (!(l > 0)) return;

	tone = fmod(tone,l);

	for (int ii = tone; ii < e->rect().height() + tone; ii++) {
		
		int i = fmod(ii + l, l);
		double i2 = fmod(ii - tone, l);

		int ci = floor(i / ll);
		int ei = fmod((e->rect().height()/ll+1),viewport()->children().size());
		
		if (ci != co) {
			
			imdr(ci, ci+1);
			QImage bs(ims[ci]->getimp(), 40, 10, QImage::Format_RGBA8888);
			painter.drawImage(QPointF(0,i2), bs.scaled(QSize(e->rect().width(), ll*0.9)));
			painter.drawText(QRect(0, i2 , e->rect().width(), ll), files[ci]->pat.filename().generic_string().c_str(), to);
			co = ci;
		}

	}
	
	painter.end();

	if (abs(curvl - tone) > 1)
		winit();
}

void Fscr::mousePressEvent(QMouseEvent* e)
{
	double ll = 0.2 * log(std::min(width(), height())) / log(1.04);
	int l = ll * viewport()->children().size();
	int cy = floor(fmod(curvl + e->pos().y() + l, l) / ll);

	if (l>0 && cy<=viewport()->children().size() && e->button() == Qt::MouseButton::LeftButton) {
		
		qDebug() << files[cy]->pat;
		static_cast<Inter*>(parent())->crf = files[cy]->pat;
		static_cast<Inter*>(parent())->pl->stop();
	}
	
	if (e->button() == Qt::MouseButton::MiddleButton) {

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
	if (mbs) {
		mousey = e->pos().y();
		winit();
	}
}

void Fscr::mouseReleaseEvent(QMouseEvent* e)
{
	if (mbs) {
		mbs = false;
		lp = e->pos();
	}
}

LKnob::LKnob(QWidget* p) : QWidget(p)
{
	setMouseTracking(true);
}

LKnob::~LKnob()
{
	lab->deleteLater();
}

void LKnob::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton) {

		val = static_cast<double>(e->pos().x()) / width();
		
		val = std::clamp(val, 0.0, 1.0);

		if (!mbs) {
			mbs = true;
			winit();
		}
	}
}

void LKnob::mouseMoveEvent(QMouseEvent* e)
{
	if (mbs) {
		val = static_cast<double>(e->pos().x()) / width();
		val = std::clamp(val, 0.0, 1.0);
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
	valt += (val - valt) * 0.4;
	QPainter painter(this);
	painter.setBackgroundMode(Qt::TransparentMode);
	std::vector<uint8_t> d = { 0,0,0,255 };
	QImage body(d.data(), 1, 1, QImage::Format_RGBA8888);
	
	for (int i = 0; i < e->rect().width(); i++) {
		
		int ma = 3;
		double c = std::clamp(2*log(100 *((static_cast<double>(i) / e->rect().width() + valt * 0.9))) / log(valt*0.09 + 1.01), 0.0, 255.0);

		if (abs(static_cast<double>(i) / e->rect().width() - valt) < 0.05)
		{
			ma = 0;
			c = 255 - c;
		}

		painter.setPen(QColor(c,c,c));
		painter.drawLine(QPointF(i,ma), QPointF(i,e->rect().height()-ma));

	}

	painter.end();

	if (abs(val - valt) > 0.01)
		winit();
}
