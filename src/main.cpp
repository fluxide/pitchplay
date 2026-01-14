
#define MINIAUDIO_IMPLEMENTATION

#include "header.h"
#include "mainwin.h"

#include <string>
#include <iostream>

int main(int argc, char* argv[]) {

	QApplication a(argc, argv);
	a.setStyle("windows");

	QWidget* i = static_cast<QWidget*>(new Inter(nullptr));
	
	i->show();

	return a.exec();
}