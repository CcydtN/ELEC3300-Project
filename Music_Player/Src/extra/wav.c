#include <wav.h>
#include "fatfs.h"
#include "debugUI.h"
#include "string.h"
#include "main.h"
#include "math.h"

static FIL pfile;
static UINT br;
static struct HEADER header;
static unsigned int WaveDataLength;
static unsigned short int data[2][512];
static BYTE Buffer[512];

static BYTE pdata = 0;

void wavPlayer(char *fname) {
	FRESULT res;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		DebugUI_push("Opened");
		//
		getHeader();
		printHeader();
		checkHeader();
		//

		//init(although most is done by main
		TIM_reINIT(36000000 / header.sample_rate - 1, 1);
		//

		//play
		WaveDataLength = header.data_size;
		getSample();
		Start_DMA();
		getSample();
		//

		//
	}
}

RESULT getHeader(void) {
	f_read(&pfile, header.riff, sizeof(header.riff), &br);
	f_read(&pfile, &header.overall_size, sizeof(header.overall_size), &br);
	f_read(&pfile, header.wave, sizeof(header.wave), &br);
	f_read(&pfile, header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker),
			&br);
	f_read(&pfile, &header.length_of_fmt, sizeof(header.length_of_fmt), &br);
	f_read(&pfile, &header.format_type, sizeof(header.format_type), &br);
	f_read(&pfile, &header.channels, sizeof(header.channels), &br);
	f_read(&pfile, &header.sample_rate, sizeof(header.sample_rate), &br);
	f_read(&pfile, &header.byterate, sizeof(header.byterate), &br);
	f_read(&pfile, &header.block_align, sizeof(header.block_align), &br);
	f_read(&pfile, &header.bits_per_sample, sizeof(header.bits_per_sample),
			&br);
	f_read(&pfile, header.data_chunk_header, sizeof(header.data_chunk_header),
			&br);
	f_read(&pfile, &header.data_size, sizeof(header.data_size), &br);
	//Some wav have extra information we don't need right now
	//It can be on top or bottom...
	while (strncmp(header.data_chunk_header, "data", 4)) {
		pfile.fptr += header.data_size;
		f_read(&pfile, header.data_chunk_header,
				sizeof(header.data_chunk_header), &br);
		f_read(&pfile, &header.data_size, sizeof(header.data_size), &br);
		if (f_eof(&pfile) == 1) {
			return END_OF_FILE;
		}
	}
	return OK;
}

RESULT checkHeader(void) {
	if (strncmp(header.riff, "RIFF", 4) == 0) { //check if the first four byte is equal to "RIFF"
		if (strncmp(header.wave, "WAVE", 4) == 0) { //check if that four byte is equal to "WAVE"
			if (strncmp(header.fmt_chunk_marker, "fmt ", 4) == 0) { //check if that four byte is equal to "fmt "(include the space)
				if (header.format_type == 1) { //check if the format is PCM or not
					if ((header.channels == 1) || (header.channels == 2)) { //check if channels is 1 or 2 (mono or stereo)
						if ((header.bits_per_sample == 8)
								|| (header.bits_per_sample == 16)) { //check if bits per sample is 8 or 16
							return OK;
						}
					}
				}
			}
		}
	}
	return FORMAT_ERROR;
}

void printHeader(void) {
	//DebugUI_push(header.riff);
	//DebugUI_pushValue(header.overall_size);
	//DebugUI_push(header.wave);
	//DebugUI_push(header.fmt_chunk_marker);
	//DebugUI_pushValue(header.length_of_fmt);
	if (header.format_type == 1) {
		DebugUI_push("PCM");
	} else {
		DebugUI_push("Unsupported format");
	};
	if (header.channels == 1) {
		DebugUI_push("Mono");
	} else {
		DebugUI_push("Stereo");
	};
	DebugUI_pushValue(header.sample_rate);
	//DebugUI_pushValue(header.byterate);
	//DebugUI_pushValue(header.block_align);
	if (header.bits_per_sample == 8) {
		DebugUI_push("8-bits unsigned");
	} else {
		DebugUI_push("16-bits signed");
	};
	//DebugUI_push(header.data_chunk_header);
	//DebugUI_pushValue(header.data_size);
}

void TIM_reINIT(uint16_t psc, uint16_t arr) {
//	I don't know why,but both number has to be non-zero
	DebugUI_pushValue(HAL_TIM_Base_DeInit(&htim2));
	htim2.Init.Prescaler = (uint32_t) psc;
	htim2.Init.Period = (uint32_t) arr;
	DebugUI_pushValue(HAL_TIM_Base_Init(&htim2));
}

void getSample(void) {
	/*Four case:
	 *	8bit mono data(0-511)
	 *	16 bit mono data(0-255)
	 *	8bit stereo data(0-255)->Left	data(256-511)->Right
	 *	16bit stereo data(0-127)->Left  data(128-255)->Right
	 */
	f_read(&pfile, Buffer, sizeof(Buffer), &br);
	//16-bits mono
	for (int i = 0; i < sizeof(Buffer) / 2; i++) {
		data[pdata][i] = (Buffer[2 * i + 1] << 8 | Buffer[2 * i]) ^ 0x8000;
	}
}

void Start_DMA(void) {
	/*Four case:
	 *	8bit mono data(0-511)
	 *	16 bit mono data(0-255)
	 *	8bit stereo data(0-255)->Left	data(256-511)->Right
	 *	16bit stereo data(0-127)->Left  data(128-255)->Right
	 */
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) data[pdata], 256,
	DAC_ALIGN_12B_L);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) data[pdata], 256,
	DAC_ALIGN_12B_L);

	HAL_TIM_Base_Start_IT(&htim2);
	pdata = pdata ^ 0x01;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	if (WaveDataLength > 0) {
		HAL_TIM_Base_Stop_IT(&htim2);
		if (WaveDataLength > 512) {
			WaveDataLength -= 512;
		} else {
			WaveDataLength = 0;
		}
		Start_DMA();
		getSample();
	} else {
		closefile();
	}
}

void closefile(void) {
	if (f_close(&pfile) == FR_OK) {
		DebugUI_push("Closed");
	};
}
