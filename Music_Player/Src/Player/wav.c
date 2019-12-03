#include "wav.h"
#include "fatfs.h"
#include "string.h"
#include "stdbool.h"
#include "Trace.h"

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

FIL pfile;
UINT br;

struct HEADER header;
unsigned int WaveDataLength;
uint16_t *data[2];
uint8_t Buffer[2][FullSize];
enum short_fmt s_fmt;
enum status {pause = 0, play};

BYTE pdata;

void wavPlayer(char *fname) {
	FRESULT res;
	pdata = 0;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		trace_printf("Opened\n");
		for (int i = 0; i < 2; ++i) {
			data[i] = Buffer[i];
		}
		//
		s_fmt = 0;
		pullHeader();
		//printFullHeader();

		if (checkHeader()) {
			printSimpifyFormat();

			//init(although most is done by main
			TIM_reINIT(36000000 / header.format.sample_rate - 1, 1);
			//

			//play
			f_lseek(&pfile, header.data.pStart);
			WaveDataLength = header.data.size;
			pullData();
			status = play;
			Start_DMA();
			pullData();
			//
		} else {
			trace_printf("Something wrong in header,plz check\n");
		}
	}
}

void pullHeader(void) {
	f_read(&pfile, header.riff, sizeof(header.riff), &br);
	f_read(&pfile, &header.overall_size, sizeof(header.overall_size), &br);
	f_read(&pfile, header.wave, sizeof(header.wave), &br);
	header.fmt_done = false;
	header.data_done = false;
	while (!f_eof(&pfile)) {
		uint32_t temp;
		f_read(&pfile, &temp, sizeof(temp), &br);
		switch (temp) {
		case 0x20746d66:
			//aka "fmt " in little endian, big endian"66 6d 74 20"
			pullFormatChunk();
			break;
		case 0x61746164:
			//aka "data" in little endian, big endian"64 61 74 61"
			pullDataChunk();
			break;
		case 0x5453494c:
			//aka "List" in little endian, big endian"4c 49 53 54"
			pullListChunk();
			break;
		default:
			f_read(&pfile, &temp, sizeof(temp), &br);
			f_lseek(&pfile, f_tell(&pfile) + temp);
			break;
		}

		if (header.fmt_done && header.data_done && header.list_done) {
			break;
		}

	};
}

void pullFormatChunk(void) {
	f_lseek(&pfile, f_tell(&pfile) - 4);
	f_read(&pfile, header.format.label, sizeof(header.format.label), &br);
	f_read(&pfile, &header.format.size, sizeof(header.format.size), &br);
	f_read(&pfile, &header.format.type, sizeof(header.format.type), &br);
	f_read(&pfile, &header.format.channels, sizeof(header.format.channels),
			&br);
	f_read(&pfile, &header.format.sample_rate,
			sizeof(header.format.sample_rate), &br);
	f_read(&pfile, &header.format.byterate, sizeof(header.format.byterate),
			&br);
	f_read(&pfile, &header.format.block_align,
			sizeof(header.format.block_align), &br);
	f_read(&pfile, &header.format.bits_per_sample,
			sizeof(header.format.bits_per_sample), &br);
	header.fmt_done = true;
}

void pullDataChunk(void) {
	f_lseek(&pfile, f_tell(&pfile) - 4);
	f_read(&pfile, header.data.label, sizeof(header.data.label), &br);
	f_read(&pfile, &header.data.size, sizeof(header.data.size), &br);
	header.data.pStart = f_tell(&pfile);
	f_lseek(&pfile, f_tell(&pfile) + header.data.size);
	header.data_done = true;
}

void pullListChunk(void) {
	f_lseek(&pfile, f_tell(&pfile) - 4);
	f_read(&pfile, header.list.label, sizeof(header.list.label), &br);
	f_read(&pfile, &header.list.size, sizeof(header.list.size), &br);
	f_lseek(&pfile, f_tell(&pfile) + header.list.size);
	header.list_done = true;
}

bool checkHeader(void) {
//check if the first four byte is equal to "RIFF"
	if (strncmp(header.riff, "RIFF", 4) != 0) {
		trace_printf("RIFF label wrong\n");
		return false;
	}
//check if that four byte is equal to "WAVE"
	if (strncmp(header.wave, "WAVE", 4) != 0) {
		trace_printf("WAVE label wrong\n");
		return false;
	}
	if (!(header.fmt_done && header.data_done)) {
		trace_printf("missing format or data tag\n");
		return false;
	}
//check if the format is PCM or not
	if (header.format.type != 1) {
		trace_printf("Not PCM\n");
		return false;
	}
//check the bits_per_sample is 8 or 16 or more
	switch (header.format.bits_per_sample) {
	case 8:
		s_fmt += 0;
		break;
	case 16:
		s_fmt += 2;
		break;
	default:
		trace_printf("bit/sample: %d\n", header.format.bits_per_sample);
		return false;
	}
	if ((header.format.channels == 1) || (header.format.channels == 2)) { //check if channels is 1 or 2 (mono or stereo)
		s_fmt += header.format.channels - 1;
	} else {
		trace_printf("More than two channel");
		return false;
	}
	return true;
}

void printSimpifyFormat(void) {
	switch (s_fmt) {
	case PCM_8_mono:
		trace_printf("8-bits Mono PCM\n");
		break;
	case PCM_8_stereo:
		trace_printf("8-bits Stereo PCM\n");
		break;
	case PCM_16_mono:
		trace_printf("16-bits Mono PCM\n");
		break;
	case PCM_16_stereo:
		trace_printf("16-bits Stereo PCM\n");
		break;

	}
}

void printHeader(void) {
	trace_printf("%s\n", header.riff);
	trace_printf("%d\n", header.overall_size);
	trace_printf("%s\n", header.wave);
	trace_printf("Format Done: \t%d\n", header.fmt_done);
	trace_printf("Data Done: \t%d\n", header.data_done);
	trace_printf("List Done: \t%d\n", header.list_done);
	trace_printf("%s:\n", header.format.label);
	trace_printf("\tSize:\t%d\n", header.format.size);
	trace_printf("\tType:\t%d\n", header.format.type);
	trace_printf("\tChannel:\t%d\n", header.format.channels);
	trace_printf("\tSample Rate:\t%d\n", header.format.sample_rate);
	trace_printf("\tByte Rate:\t%d\n", header.format.byterate);
	trace_printf("\tBlock Align:\t%d\n", header.format.block_align);
	trace_printf("\tBit per Sample:\t%d\n", header.format.bits_per_sample);
	trace_printf("%s:\n", header.data.label);
	trace_printf("\tSize:\t%d\n", header.data.size);
	trace_printf("\tPointer:\t%d\n", header.data.pStart);
	trace_printf("%s:\n", header.list.label);
	trace_printf("\tSize:\t%d\n", header.list.size);

}

void TIM_reINIT(uint16_t psc, uint16_t arr) {
//	I don't know why,but both number has to be non-zero
//	May be the pluse is not long enough
	HAL_TIM_Base_DeInit(&htim2);
	htim2.Init.Prescaler = (uint32_t) psc;
	htim2.Init.Period = (uint32_t) arr;
	HAL_TIM_Base_Init(&htim2);
}

void pullData(void) {
	if (s_fmt < 2) {
		f_read(&pfile, Buffer[pdata], HalfSize, &br);
	} else {
		f_read(&pfile, Buffer[pdata], FullSize, &br);
	}
//	for (int i = 0; i < 10; i++) {
//		trace_printf("%x\t", Buffer[pdata][i]);
//	}
//	trace_printf("\n");
	rearrData();
//	for (int i = 0; i < 10; i++) {
//		trace_printf("%x\t", Buffer[pdata][i]);
//	}
}

void rearrData(void) {
	/*
	 * When it is 16 bit, change the value to unsigned
	 * If the data is
	 * LRLRLRLRLR.....
	 * change to
	 * LLLLL....RRRRR....
	 */
	uint8_t temp[FullSize];
	memset(temp, 0, sizeof(temp));

	switch (s_fmt) {
	case PCM_8_mono:
		for (int i = 0; i < HalfSize; i++) {
			temp[2 * i] = Buffer[pdata][i];
		}
		memcpy(Buffer[pdata], temp, FullSize);
		break;
	case PCM_8_stereo:
		for (int i = 0; i < QuarSize; i++) {
			temp[2 * i] = Buffer[pdata][2 * i];
			temp[2 * i + HalfSize] = Buffer[pdata][2 * i + 1];
		}
		memcpy(Buffer[pdata], temp, FullSize);
		break;
	case PCM_16_mono:
		for (int i = 0; i < HalfSize; i++) {
			data[pdata][i] ^= 0x8000;
		}
		break;
	case PCM_16_stereo:
		for (int i = 0; i < QuarSize; i++) {
			temp[2 * i] = Buffer[pdata][4 * i];
			temp[2 * i + 1] = Buffer[pdata][4 * i + 1] ^ 0x80;
			temp[2 * i + HalfSize] = Buffer[pdata][4 * i + 2];
			temp[2 * i + HalfSize + 1] = Buffer[pdata][4 * i + 3] ^ 0x80;
		}
		memcpy(Buffer[pdata], temp, FullSize);
		break;
	}
}

void Start_DMA(void) {
	/*	8-bit mono:		data(0-511)
	 *  8-bit stereo:	data(0-255)(LEFT)	data(256-511)(RIGHT)
	 *  16-bit mono:	data(0-255)
	 *  16-bit stereo:	data(0-127)(LEFT)	data(128-255)(RIGHT)
	 */
	switch (s_fmt) {
	case PCM_8_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) Buffer[pdata],
		HalfSize,
		DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) Buffer[pdata],
		HalfSize,
		DAC_ALIGN_8B_R);
		break;
	case PCM_8_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) Buffer[pdata],
		QuarSize,
		DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &Buffer[pdata][HalfSize],
				QuarSize,
				DAC_ALIGN_8B_R);
		break;
	case PCM_16_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) Buffer[pdata],
		HalfSize,
		DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) Buffer[pdata],
		HalfSize,
		DAC_ALIGN_12B_L);
		break;
	case PCM_16_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) Buffer[pdata],
		QuarSize,
		DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &Buffer[pdata][HalfSize],
				QuarSize,
				DAC_ALIGN_12B_L);
		break;
	}
	HAL_TIM_Base_Start(&htim2);
	pdata = pdata ^ 0x01;
}

void closefile(void) {
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
	HAL_TIM_Base_Stop(&htim2);
	status = pause;
	if (f_close(&pfile) == FR_OK) {
		trace_printf("File Closed\n");
	};
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	if (WaveDataLength > 0) {
		HAL_TIM_Base_Stop(&htim2);
		if (s_fmt < 2) {
			if (WaveDataLength > 256) {
				WaveDataLength -= 256;
			} else {
				WaveDataLength = 0;
			}
		} else {
			if (WaveDataLength > 512) {
				WaveDataLength -= 512;
			} else {
				WaveDataLength = 0;
			}
		}

		Start_DMA();
		pullData();
	} else {
		closefile();
	}
}
