/*pitchplay
  play sound files, rpgmaker style.

  this program is an audio player with a few utilities. originally made for the collective unconscious project,
  as a simple tool for developers to preview ogg files in varying pitches. it also offers a copy command utility.

  made by fluxide, 2026.
*/

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