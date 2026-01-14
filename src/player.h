#pragma once

#include <header.h>
#include "miniaudio.h"
#include <vorbis/vorbisfile.h>
#include <mainwin.h>

class APlay {

public:

	APlay(QWidget* p);
	~APlay();

	void play(std::filesystem::path f);
	void stop();

	char* ppin;
	char* ppout;
	int ii=0;
	std::filesystem::path curf = "";
	OggVorbis_File ogg;
	vorbis_info* vi = nullptr;
	ma_data_converter conv;
	ma_resampler a;
	QWidget* parent = nullptr;
	double read = 0;
	unsigned int atot = 0;
	unsigned long tot = 0;
	std::vector<char> data;


private:

	ma_device device;

};

void playcb(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
