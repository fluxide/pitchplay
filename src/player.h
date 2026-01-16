#pragma once

#include <header.h>
#include "miniaudio.h"
#include <vorbis/vorbisfile.h>
#include <mainwin.h>
#include <mpg123.h>

//audio player class
class APlay {

public:

	APlay(QWidget* p);
	~APlay();

	void play(std::filesystem::path f);
	void stop();

	char* ppinr; //post process, in, right
	char* ppinl; //post process, in, left
	char* ppout; //post process, out, interleaved
	std::filesystem::path curf = ""; //current file
	OggVorbis_File ogg; //ogg file if needed
	vorbis_info* vi = nullptr; //ogg file info
	QWidget* parent = nullptr; 
	double read = 0; //read position for playback
	unsigned long tot = 0;  //total size of the pcm buffer in bytes
	std::vector<char> data; //byte array containing the song
	int ts = 0; //type size (size of the input data type in bytes, its either 2 or 4)
	int chn = 0; //channels (no more than 2 supported)


private:

	ma_device device; //miniaudio playback device

};

//global scope defined play time callback function, used by miniaudio
void playcb(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);