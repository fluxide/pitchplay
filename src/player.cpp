
#include "mainwin.h"
#include "player.h"
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "miniaudio.h"
#include "sndfile.h"
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <fstream>

APlay::APlay(QWidget* p)
{
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
	if (f.generic_string()=="") return;
	curf = f;
	FILE* fi = nullptr;
	fopen_s(&fi,f.generic_string().c_str(), "rb");

#ifdef _WIN32
	_setmode(_fileno(fi), _O_BINARY);
#endif

	if (!ma_device_is_started(&device)) {



		if (f.extension() == ".ogg") {
		
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
			ts = 2;
		}

		if (f.extension() == ".mp3") {
		
			return;
		}

		if (f.extension() == ".wav") {
		
			SF_INFO inf{};
			SNDFILE* wf = sf_open(f.generic_string().c_str(), SFM_READ, &inf);
			int di[4096];
			chn = inf.channels;

			while (1) {

				long r = sf_read_int(wf, di, 4096);

				if (r <= 0) break;
				data.insert(data.end(), (char*)di, (char*)di + 4*r);
				tot += 4*r;
			}

			ts = 4;
		}


		unsigned int ii = 0;
		ppinr = new char[tot/2];
		ppinl = new char[tot/2];

		for (unsigned int i = 0; i < tot;i += ts*chn) {

			for (int iii = 0; iii < ts; iii++) {
				ppinl[ii + iii] = data.data()[i+iii];

			}
			
			for (int iii = 0; iii < ts; iii++) {
				ppinr[ii + iii] = data.data()[i+iii+ts*(chn-1)];

			}

			ii += ts;
		}

		ppout = new char[11 * tot * ts];
		for (unsigned int i = 0;i < 11 * tot * ts;i++) ppout[i] = 0;

		ma_device_start(&device);
	}
}

void APlay::stop()
{
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
	double pitch = std::clamp(static_cast<Inter*>(p->parent)->pitk->val/100,0.1,2.0);
	double volume = static_cast<Inter*>(p->parent)->vk->val/100;
	double fade_in = static_cast<Inter*>(p->parent)->fik->val/10;
	double panning = static_cast<Inter*>(p->parent)->pk->val/100;
	char* out = static_cast<char*>(pOutput);

	assert(p->ts != 0);

	char* sampr = (char*)p->ppinr;
	char* sampl = (char*)p->ppinl;
	float* sampout = (float*)p->ppout;
	double ii = 0;

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

			if (ts == 2) 
				sum += ((int16_t*)buf)[idx] * s;
			else if (ts == 4)
				sum += ((int*)buf)[idx] * s;
			norm += s;
		}

		return norm != 0.0 ? sum / norm : 0.0;
	};

	for (unsigned int i = 0; i < 2*frameCount; i+=2) {

		double ci = ii + p->read;
		sampout[i] = static_cast<float>(((1 - panning) * volume * fr(sampl, ci, p->tot/p->ts/2, p->ts))/pow(2,8*p->ts-1));
		sampout[i+1] = static_cast<float>((panning*volume* fr(sampr, ci, p->tot/ p->ts / 2, p->ts)) / pow(2, 8 * p->ts-1));
		ii+= pitch;
	}

	p->read += ii;
	p->read = fmod(p->read,p->tot/(p->ts * 2)-2*frameCount-4);

	std::memcpy(out, p->ppout, 8 * static_cast<size_t>(frameCount));
}