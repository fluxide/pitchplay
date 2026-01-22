
#include "mainwin.h"
#include "player.h"
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "miniaudio.h"
#include "sndfile.h"
#include "mpg123.h"
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <fstream>

APlay::APlay(QWidget* p)
{
	//miniaudio device init. input values to the backend are always float 32
	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = 2;
	config.sampleRate = 44100;
	config.dataCallback = playcb;
	config.pUserData = this;

	ma_device_init(NULL, &config, &device);
	parent = p;
}

APlay::~APlay()
{
	ov_clear(&ogg);
	ma_device_uninit(&device);		
	delete[] ppinr;
	delete[] ppinl;
	delete[] ppout;
}

void APlay::play(std::filesystem::path f)
{
	//open file first, if its not empty
	if (f.generic_string()=="") return;
	curf = f;
	FILE* fi = nullptr;
	fopen_s(&fi,f.generic_string().c_str(), "rb");

#ifdef _WIN32
	_setmode(_fileno(fi), _O_BINARY);
#endif

	//dont start the device twice
	if (!ma_device_is_started(&device)) {


		//in all formats, the entire file is read once and recorded into a vector

		if (f.extension() == ".ogg") { //read ogg with vorbis
		
			std::memset(&ogg, 0, sizeof(ogg));
			ov_open_callbacks(fi, &ogg, NULL, 0, OV_CALLBACKS_NOCLOSE);

			vi = ov_info(&ogg, -1);

			if (vi != nullptr) {
				chn = vi->channels;
			}
			else {
				chn = 1;
			}

			int bs = 0;
			char di[4096 * 8];

			while (1) {

				long r = ov_read(&ogg, di, 4096 * 8, 0, 2, 1, &bs);

				if (r <= 0) break;
				data.insert(data.end(), di, di + r);
				tot += r;
			}
			ts = 2; //data is int16_t type
		}

		if (f.extension() == ".mp3") { //read mp3 with mpg123
		
			mpg123_handle* mp = mpg123_new(nullptr,nullptr);
			mpg123_open(mp,f.generic_string().c_str());

			long rate;
			int enc;
			mpg123_getformat(mp,&rate,&chn,&enc);
			mpg123_format(mp, rate, chn, MPG123_ENC_SIGNED_16);

			int16_t di[4096 * 4];

			while (1) {
				size_t r;
				int cc = mpg123_read(mp, di, 4096 * 4, &r);

			    if (cc == MPG123_DONE) break;
				data.insert(data.end(), (char*)di, (char*)di + r);
				tot += r;
			}
			ts = 2;
		}

		if (f.extension() == ".wav") { //read wav with sndfile
		
			SF_INFO inf{};
			SNDFILE* wf = sf_open(f.generic_string().c_str(), SFM_READ, &inf);
			float di[4096];
			chn = inf.channels;

			while (1) {

				long r = sf_read_float(wf, di, 4096);

				if (r <= 0) break;
				data.insert(data.end(), (char*)di, (char*)di + 4*r);
				tot += 4*r;
			}

			ts = 4; //data is integer type
		}


		unsigned int ii = 0;
		ppinr = new char[tot/2];
		ppinl = new char[tot/2];

		//channels are separated for convenience, both channels should be the same if the track will be mono
		for (unsigned int i = 0; i < tot;i += ts*2) {

			for (int iii = 0; iii < ts; iii++) {
				ppinl[ii + iii] = data.data()[i+iii];

			}
			
			for (int iii = 0; iii < ts; iii++) {
				ppinr[ii + iii] = data.data()[i+iii+ts*(chn-1)];

			}

			ii += ts;
		}

		data.clear();

		ppout = new char[4096*ts];
		std::memset(ppout, 0, 4096*ts);

		//start device
		ma_device_start(&device);
	}
}

void APlay::stop()
{
	//dont stop the device twice
	if (ma_device_is_started(&device)) {
		ma_device_stop(&device);
		read = 0;
		tot = 0;
		data.clear();
		delete[] ppinl;
		delete[] ppinr;
		delete[] ppout;
	}
}

void playcb(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	APlay* p = static_cast<APlay*>(pDevice->pUserData);
	double pitch = std::clamp(static_cast<Inter*>(p->parent)->pitk->val/100,0.1,2.0); //the parameters are converted to float for processing. the pitch value is clamped
	double volume = static_cast<Inter*>(p->parent)->vk->val/100;
	double fade_in = static_cast<Inter*>(p->parent)->fik->val/10;
	double panning = static_cast<Inter*>(p->parent)->pk->val/100;
	char* out = static_cast<char*>(pOutput);

	assert(p->ts != 0); //ts cant be 0

	char* sampr = (char*)p->ppinr;
	char* sampl = (char*)p->ppinl;
	float* sampout = (float*)p->ppout; //output is always float
	double ii = 0; //secondary index for pitching

	//does very fine sinc interpolation
	auto fr = [](char* buf, double p, int tot, int ts) {

		int h = 4;
		int fp = floor(p);
		double f = p - fp;

		double sum = 0.0;
		double norm = 0.0;

		for (int i = 1 - h;i < h;i++) {

			int idx = fp + i;
			idx = (idx + tot) % tot;

			double x = i - f;
			double w = (1+cos(2*M_PI*x/h))/2;
			double s = x==0.0 ? w : sin(x*M_PI)/x*M_PI * w;

			//input buffer is cast to the correct type based on ts
			if (ts == 2) 
				sum += ((int16_t*)buf)[idx] * s;
			else if (ts == 4)
				sum += ((float*)buf)[idx] * s;
			norm += s;
		}

		return norm != 0.0 ? sum / norm : 0.0;
	};

	for (unsigned int i = 0; i < 2*frameCount; i+=2) {
		
		double ci = ii + p->read; //current read index
		double fivol = volume * std::clamp((ci / pitch) / (44100 * fade_in + 0.1),0.0,1.0); //volume, with fade in handled
		//processed data is cast to float, and interleaved. panning is done linearly
		int quiet = p->ts == 2 ? pow(2, 8 * p->ts - 1) : 1;
		sampout[i] = static_cast<float>(((1 - panning) * fivol * fr(sampl, ci, p->tot/p->ts/2, p->ts))/quiet);
		sampout[i+1] = static_cast<float>((panning*fivol* fr(sampr, ci, p->tot/ p->ts / 2, p->ts)) /quiet);
		ii+= pitch; //advance the reader by pitch
	}

	p->read += ii; //advance total read index after one loop
	p->read = fmod(p->read,p->tot/(p->ts * 2)-2*frameCount-4); //loop back safely

	std::memcpy(out, p->ppout, 8 * static_cast<size_t>(frameCount)); //write output
}