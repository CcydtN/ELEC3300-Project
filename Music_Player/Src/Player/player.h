#ifndef SRC_PLAYER_PLAYER_H_
#define SRC_PLAYER_PLAYER_H_

#include "stdbool.h"

//For OGG
#include "config_types.h"
#include "ivorbiscodec.h"
#include "ivorbisfile.h"

#include "vorbisfile.c"
#include "info.c"
#include "framing.c"
#include "bitwise.c"
#include "mapping0.c"
#include "floor0.c"
#include "floor1.c"
#include "codebook.c"
#include "res012.c"
#include "dsp.c"
#include "floor_lookup.c"
#include "mdct.c"

//For WAV

enum short_fmt {
	PCM_8_mono = 0, PCM_8_stereo, PCM_16_mono, PCM_16_stereo
};
// WAVE file header format
struct FORMAT {
	//"fmt "(with a space)
	char label[4];
	//size of chunk
	unsigned int size;
	// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t type;
	// no.of channels
	uint16_t channels;
	// sampling rate (blocks per second)
	unsigned int sample_rate;
	// SampleRate * NumChannels * BitsPerSample/8
	unsigned int byterate;
	// NumChannels * BitsPerSample/8
	uint16_t block_align;
	// bits per sample, 8- 8bits, 16- 16 bits etc
	uint16_t bits_per_sample;
};

struct DATA {
	//"data"
	char label[4];
	//total size of data = NumSamples* NumChannels*BitPerSample/8
	unsigned int size;
	//Start pointer, for data reading in later
	DWORD pStart;
};

struct LIST {
	//"LIST"
	char label[4];
	//size of chunk
	unsigned int size;
};

struct HEADER {
	//"RIFF"
	char riff[4];
	// overall size of file in bytes
	unsigned int overall_size;
	//"WAVE"
	char wave[4];
	//format chunk
	struct FORMAT format;
	// True after format is read
	bool fmt_done;
	//data chunk
	struct DATA data;
	// True after data is read
	bool data_done;
	//List chunk
	struct LIST list;
	//True after data is read
	bool list_done;

};

enum {
	unsupported = 0, wav, ogg
} f_type;

void player(char *fname);

#endif /* SRC_PLAYER_PLAYER_H_ */
