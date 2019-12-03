#ifndef _WAV_H_
#define _WAV_H_

//Array size constant
#define FullSize	512
#define HalfSize	256
#define QuarSize	128

enum short_fmt {
	PCM_8_mono = 0, PCM_8_stereo, PCM_16_mono, PCM_16_stereo
};

enum stat {
	Pause = 0, Play
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
//End of Header

void wavPlayer(char *fname);

void pullHeader(void);

void pullFormatChunk(void);

void pullDataChunk(void);

void pullListChunk(void);

bool checkHeader(void);

void printSimpifyFormat(void);

void printHeader(void);

void TIM_reINIT(uint16_t psc, uint16_t arr);

void DMA_reINIT(void);

void pullData(void);

void rearrData(void);

void Start_DMA(void);

void closefile(void);

#endif /* SRC_EXTRA_WAV_H_ */
