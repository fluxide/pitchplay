
#include "mainwin.h"
#include "player.h"
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "miniaudio.h"
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <fstream>

APlay::APlay(QWidget* p)
{
	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_s16;
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
	delete[] ppin;
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

	if (f.extension() == ".ogg") {
		if (!ma_device_is_started(&device)) {
			std::memset(&ogg, 0, sizeof(ogg));
			ov_open_callbacks(fi, &ogg, NULL, 0, OV_CALLBACKS_NOCLOSE);
			
			vi = ov_info(&ogg, -1);
			int chn = 2;
			int r = 44100;

			if (vi != nullptr) {
				chn = vi->channels;
				r = vi->rate;
			}

			int bs = 0;
			char di[4096*4];

			while (1) {

				r = ov_read(&ogg, di, 4096*4, 0, 2, 1, &bs);
				
				if (r <= 0) break;
				data.insert(data.end(),di,di+r);
				tot += r;
			}

			ppin = data.data();
			ppout = new char[11 * tot];
			for (unsigned int i = 0;i < 11* tot;i++) ppout[i] = 0;

			ma_device_start(&device);
		}
	}

	if (f.extension() == ".mp3") {
		

	}

	if (f.extension() == ".wav") {
		

	}
}

void APlay::stop()
{
	if (ma_device_is_started(&device)) {
		ma_device_stop(&device);
		read = 0;
		tot = 0;
		data.clear();
		std::memset(ppin,0,tot);
		delete[] ppout;
	}
}

void playcb(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	APlay* p = static_cast<APlay*>(pDevice->pUserData);
	double pitch = std::clamp(floor(static_cast<Inter*>(p->parent)->pitk->val*200)/100,0.1,2.0);
	double volume = static_cast<Inter*>(p->parent)->vk->val;
	double fade_in = static_cast<Inter*>(p->parent)->fik->val;
	char* out = static_cast<char*>(pOutput);

	int16_t* samp = (int16_t*)p->ppin;
	int16_t* sampout = (int16_t*)p->ppout;
	int ii = 0;

	for (float i = 0; i < 2*frameCount*pitch; i+=pitch) {

		double ci = i + p->read;
		int ri = floor(ci);
		int ri2 = floor(ci+pitch);
		sampout[ii] = static_cast<int16_t>(volume*((ci-ri)*(samp[ri+1]-samp[ri])+samp[ri]));
		ii++;
	}

	ii *= pitch;
	p->read += ii;
	p->read = fmod(p->read,p->tot/2-2*frameCount*pitch);

	std::memcpy(out, p->ppout, 4 * static_cast<size_t>(frameCount));
}