#ifndef SRC_EXTRA_WAV_H_
#define SRC_EXTRA_WAV_H_

typedef enum {
	OK = 0, END_OF_FILE, FORMAT_ERROR
} RESULT;

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

// WAVE file header format
struct HEADER {
	char riff[4];						// RIFF string
	unsigned int overall_size;				// overall size of file in bytes
	char wave[4];						// WAVE string
	char fmt_chunk_marker[4];	// fmt string with trailing null char
	unsigned int length_of_fmt;					// length of the format data
	uint16_t format_type;// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;						// no.of channels
	unsigned int sample_rate;				// sampling rate (blocks per second)
	unsigned int byterate;		// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;					// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;	// bits per sample, 8- 8bits, 16- 16 bits etc
	char data_chunk_header[4];		// DATA string or FLLR string
	unsigned int data_size;	// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

void wavPlayer(char *fname);
RESULT getHeader(void);
RESULT checkHeader(void);
void printHeader(void);
void TIM_reINIT(uint16_t psc, uint16_t arr);
void getSample(void);
void Start_DMA(void);
void closefile(void);

#endif /* SRC_EXTRA_WAV_H_ */
