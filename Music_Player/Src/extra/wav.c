#include <wav.h>
#include "fatfs.h"
#include "debugUI.h"
#include "string.h"
#include "main.h"
#include "stm32f1xx_hal_tim.h"

static struct HEADER header;
static FIL pfile;
static UINT br;
static unsigned int WaveDataLength;
static BYTE wavBuffer[2][512];
static int pBuffer, pData, dataDone;
extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac_ch1;
extern TIM_HandleTypeDef htim2;

void wav_INIT(char *fname) {

	FRESULT res;
	RESULT R;
	pBuffer = 0;
	pData = 0;
	dataDone = 0;

	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		DebugUI_push("Opened");
		//

		R = getHeader();
		if (R == OK) {
			R = checkHeader();
			DebugUI_pushValue(R);
		} else {
			DebugUI_pushValue(R);
		}
		WaveDataLength = header.data_size;
		//

		//
		res = f_close(&pfile);
		if (res == FR_OK) {
			DebugUI_push("Closed");
		};

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
	while (strcmp(header.data_chunk_header, "data")) {
		pfile.fptr += header.data_size;
		f_read(&pfile, header.data_chunk_header,
				sizeof(header.data_chunk_header), &br);
		f_read(&pfile, &header.data_size, sizeof(header.data_size), &br);
		if (f_eof(&pfile) == 0) {
			return END_OF_FILE;
		}
	}
	f_read(&pfile, wavBuffer[0], sizeof(wavBuffer[0]), &br);
	return OK;
}

RESULT checkHeader(void) {
	if (!(strcmp(header.riff, "RIFF") //check if the first four byte is equal to "RIFF"
			|| strcmp(header.wave, "WAVE") //check if that four byte is equal to "WAVE"
			|| strcmp(header.fmt_chunk_marker, "fmt ") //check if that four byte is equal to "fmt "(include the space)
			|| (header.format_type | 0x1) //check if the format is PCM or not
			|| ((header.channels | 0x1) || (header.channels | 0x10)) //check if channels is 1 or 2 (mono or stereo)
			|| ((header.bits_per_sample | 0x1000)
					|| (header.bits_per_sample | 0x100)) //check if bits per sample is 8 or 16
	)) {
		return OK;
	} else {
		return FORMAT_ERROR;
	}
}

void printHeader(void) {
	DebugUI_push(header.riff);
	DebugUI_pushValue(header.overall_size);
	DebugUI_push(header.wave);
	DebugUI_push(header.fmt_chunk_marker);
	DebugUI_pushValue(header.length_of_fmt);
	DebugUI_pushValue(header.format_type);
	DebugUI_pushValue(header.channels);
	DebugUI_pushValue(header.sample_rate);
	DebugUI_pushValue(header.byterate);
	DebugUI_pushValue(header.block_align);
	DebugUI_pushValue(header.bits_per_sample);
	DebugUI_push(header.data_chunk_header);
	DebugUI_pushValue(header.data_size);
}

void wav_play(void) {
	while (WaveDataLength != 0) {
		f_read(&pfile, wavBuffer[1], sizeof(wavBuffer[1]), &br);
		while (!dataDone) {
			dataDone = 0;
			pBuffer = 1;
		}
		if (WaveDataLength < 512) {
			WaveDataLength = 0;
		} else {
			WaveDataLength -= 512;
		};
		/*
		 Disable DMA , Config DMA to transfer 512
		 bytes from wavBuffer2 to DAC ch1 8bit
		 DHR register, and enable DMA
		 */
		f_read(&pfile, wavBuffer[0], sizeof(wavBuffer[1]), &br);
		while (!dataDone) {
			dataDone = 0;
			pBuffer = 0;
		}
		if (WaveDataLength < 512) {
			WaveDataLength = 0;
		} else {
			WaveDataLength -= 512;
		};
		/*
		 Disable DMA , Config DMA to transfer 512
		 bytes from wavBuffer2 to DAC ch1 8bit
		 DHR register, and enable DMA
		 */
	}
}

void TIM_reINIT(void) {
	HAL_TIM_Base_DeInit(&htim2);
	htim2.Init.Prescaler = 70;
	htim2.Init.Period = 8;
	HAL_TIM_Base_Init(&htim2);
}
