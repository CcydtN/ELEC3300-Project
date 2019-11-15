#include <wav.h>
#include "fatfs.h"
#include "debugUI.h"
#include "string.h"
#include "main.h"

static FIL pfile;
static UINT br;
static struct HEADER header;
static unsigned int WaveDataLength;
static unsigned short int data[2][512];
static BYTE Buffer[512];
static enum format Format;

static BYTE pdata = 0;

void wavPlayer(char *fname) {
	FRESULT res;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		DebugUI_push("Opened");
		//
		getHeader();
		printHeader();
		if (checkHeader() == OK) {
			DebugUI_pushValue(Format);

			//init(although most is done by main
			TIM_reINIT(36000000 / header.sample_rate - 1, 1);
			//

			//play
			WaveDataLength = header.data_size;
			getSample();
			Start_DMA();
			getSample();
			//

		}
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
	//check if the first four byte is equal to "RIFF"
	if (strncmp(header.riff, "RIFF", 4) != 0) {
		return FORMAT_ERROR;
	}
	//check if that four byte is equal to "WAVE"
	if (strncmp(header.wave, "WAVE", 4) != 0) {
		return FORMAT_ERROR;
	}
	//check if that four byte is equal to "fmt "(include the space)
	if (strncmp(header.fmt_chunk_marker, "fmt ", 4) != 0) {
		return FORMAT_ERROR;
	}
	//check if the format is PCM or not
	if (header.format_type != 1) {
		DebugUI_push("Not PCM");
		return FORMAT_ERROR;
	}
	//check the bits_per_sample is 8 or 16 or more
	switch (header.bits_per_sample) {
	case 8:
		Format += 0;
		break;
	case 16:
		Format += 2;
		break;
	default:
		DebugUI_push("bit/sample:");
		DebugUI_pushValue(header.bits_per_sample);
		return FORMAT_ERROR;
	}
	if ((header.channels == 1) || (header.channels == 2)) { //check if channels is 1 or 2 (mono or stereo)
		Format += header.channels - 1;
	} else {
		DebugUI_push("Channel:");
		DebugUI_pushValue(header.channels);
		return FORMAT_ERROR;
	}
	return OK;
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
	HAL_TIM_Base_DeInit(&htim2);
	htim2.Init.Prescaler = (uint32_t) psc;
	htim2.Init.Period = (uint32_t) arr;
	HAL_TIM_Base_Init(&htim2);
}

void getSample(void) {
	/*	8-bit mono:		data(0-511)
	 *  8-bit stereo:	data(0-255)(LEFT)	data(256-511)(RIGHT)
	 *  16-bit mono:	data(0-255)
	 *  16-bit stereo:	data(0-127)(LEFT)	data(128-255)(RIGHT)
	 */
	f_read(&pfile, Buffer, FullSize, &br);
	switch (Format) {
	case PCM_8_mono:
		for (int i = 0; i < FullSize; i++) {
			data[pdata][i] = Buffer[i];
		}
		break;
	case PCM_8_stereo:
		for (int i = 0; i < HalfSize; i++) {
			data[pdata][i] = Buffer[2 * i];
			data[pdata][i + HalfSize] = Buffer[2 * i + 1];
		}
		break;
	case PCM_16_mono:
		for (int i = 0; i < HalfSize; i++) {
			data[pdata][i] =
					((Buffer[2 * i + 1] << 8 | Buffer[2 * i]) ^ 0x8000);
		}
		break;
	case PCM_16_stereo:
		for (int i = 0; i < QuarSize; i += 4) {
			data[pdata][i] =
					((Buffer[4 * i + 1] << 8 | Buffer[4 * i]) ^ 0x8000);
			data[pdata][i + QuarSize] = ((Buffer[4 * i + 3] << 8
					| Buffer[4 * i + 2]) ^ 0x8000);
		}
		break;

	}
}

void Start_DMA(void) {
	/*	8-bit mono:		data(0-511)
	 *  8-bit stereo:	data(0-255)(LEFT)	data(256-511)(RIGHT)
	 *  16-bit mono:	data(0-255)
	 *  16-bit stereo:	data(0-127)(LEFT)	data(128-255)(RIGHT)
	 */
	switch (Format) {
	case PCM_8_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) data[pdata],
		FullSize,
		DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) data[pdata],
		FullSize,
		DAC_ALIGN_8B_R);
		break;
	case PCM_8_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) data[pdata],
		HalfSize,
		DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &data[pdata][HalfSize],
				HalfSize,
				DAC_ALIGN_8B_R);
		break;
	case PCM_16_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) data[pdata],
		HalfSize,
		DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) data[pdata],
		HalfSize,
		DAC_ALIGN_12B_L);
		break;
	case PCM_16_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) data[pdata],
		QuarSize,
		DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &data[pdata][QuarSize], QuarSize,
				DAC_ALIGN_12B_L);
		break;
	}
	HAL_TIM_Base_Start(&htim2);
	pdata = pdata ^ 0x01;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	if (WaveDataLength > 0) {
		HAL_TIM_Base_Stop(&htim2);
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
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
	HAL_TIM_Base_Stop_IT(&htim2);
	if (f_close(&pfile) == FR_OK) {
		DebugUI_push("Closed");
	};
}
