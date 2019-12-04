#include <temp.h>
#include "fatfs.h"
#include "stdio.h"

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

FIL pfile;
UINT br;

short int output[2][2048];
OggVorbis_File vf;
int bitstream;
ov_callbacks callbacks;

vorbis_info *info;

int bytes_read;
int temp;
int pdata = 0;

size_t alt_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
	if (!f_eof(&pfile)) {
		f_read(datasource, ptr, size * nmemb, &br);
		return (br / size);
	} else
		return 0;
}
int alt_seek(void *datasource, ogg_int64_t offset, int whence) {
	FRESULT res;
	int curpos = 0;

	switch (whence) {
	case SEEK_SET:
		curpos = offset;
		break;
	case SEEK_CUR:
		curpos = f_tell(&pfile) + offset;
		break;
	case SEEK_END:
		curpos = f_size(&pfile) - offset;
		break;
	}

	res = f_lseek(datasource, curpos);
	return res;
}
int alt_close(void *datasource) {
	return f_close(datasource);
}
long alt_tell(void *datasource) {
	return f_tell(&pfile);
}

void oggPlayer(char *fname) {
	callbacks.read_func = alt_read;
	callbacks.seek_func = alt_seek;
	callbacks.close_func = alt_close;
	callbacks.tell_func = alt_tell;

	FRESULT res;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		trace_printf("Opened %s\n", fname);
//		f_read(&pfile, buffer, sizeof(buffer), &br);
		trace_printf("loaded\n");
		//temp = ov_open(buffer, &vf, NULL, 0);
		temp = ov_open_callbacks(&pfile, &vf, NULL, 0, callbacks);
		trace_printf("ov_open:\t%d\n", temp);

		info = ov_info(&vf, -1);
		trace_printf("rate:\t%d\n", info->rate);
		trace_printf("channels:\t%d\n", info->channels);

		pullData();
		pullData();
		TIM_reINIT(36000000 / info->rate - 1, 1);
		Start_DMA();
		pullData();
	}
}

void pullData(void) {
	bytes_read = ov_read(&vf, output[pdata], sizeof(output[pdata]), &bitstream);
	//trace_printf("%d\n", bytes_read);
	rearrData();
}

void rearrData(void) {
	unsigned short int temp[bytes_read / 2];
	switch (info->channels) {
	case 1:
		for (int i = 0; i < bytes_read / 2; i++) {
			output[pdata][i] ^= 0x8000;
		}
		break;
	case 2:
		for (int i = 0; i < bytes_read / 4; i++) {
			temp[i] = output[pdata][2 * i] ^ 0x8000;
			temp[i + bytes_read / 4] = output[pdata][2 * i + 1] ^ 0x8000;
		}
		memcpy(output[pdata], temp, bytes_read);
		break;
	}
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	HAL_TIM_Base_Stop(&htim2);
	TIM_reINIT(36000000 / info->rate - 1, 1);
	if (!f_eof(&pfile)) {
		Start_DMA();
		pullData();
	} else {
		closefile();
	}
}

void TIM_reINIT(uint16_t psc, uint16_t arr) {
//	I don't know why,but both number has to be non-zero
//	May be the pluse is not long enough
	HAL_TIM_Base_DeInit(&htim2);
	htim2.Init.Prescaler = (uint32_t) psc;
	htim2.Init.Period = (uint32_t) arr;
	HAL_TIM_Base_Init(&htim2);
}

void Start_DMA(void) {
	/*	8-bit mono:		data(0-511)
	 *  8-bit stereo:	data(0-255)(LEFT)	data(256-511)(RIGHT)
	 *  16-bit mono:	data(0-255)
	 *  16-bit stereo:	data(0-127)(LEFT)	data(128-255)(RIGHT)
	 */
	switch (info->channels) {
	case 1:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_12B_L);
		break;
	case 2:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 4,
				DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &output[pdata][bytes_read / 4], bytes_read / 4,
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
	ov_clear(&vf);
	trace_printf("Closed");
}
