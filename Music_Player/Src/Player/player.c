#include "player.h"
#include "ff.h"
#include "ffconf.h"
#include "fatfs.h"
#include "string.h"
#include "Trace.h"

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

enum f_type type;

FIL pfile;
UINT br;

short int output[2][1024];

//For ogg
OggVorbis_File vf;
int bitstream;
ov_callbacks callbacks;
vorbis_info *info;

//For wav
struct HEADER header;

enum short_fmt s_fmt;
int bytes_read, bytes_finish, temp, pdata;

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

int player(char *fname) {
	checkExtension(fname);
	trace_printf("%d\n", type);
	FRESULT res;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK && type != 0) {
		pdata = 0;
		s_fmt = 0;
		bytes_finish = 0;
		if (type == wav) {
			pullHeader();
			if (checkHeader()) {
				TIM_reINIT(header.format.sample_rate);
				f_lseek(&pfile, header.data.pStart);
			} else {
				return 1;
			}
		} else if (type == ogg) {
			callbacks.read_func = alt_read;
			callbacks.seek_func = alt_seek;
			callbacks.close_func = alt_close;
			callbacks.tell_func = alt_tell;
			ov_open_callbacks(&pfile, &vf, NULL, 0, callbacks);

			info = ov_info(&vf, -1);
			s_fmt = 1 + info->channels;
			pullData();
		} else {
			return 1;
		}
		printINFO();
		TIM_reINIT(getRate());
		pullData();
		Start_DMA();
		pullData();
	} else {
		return 1;
	}
	return 0;
}

void checkExtension(char *fname) {
	char *ext = strrchr(fname, '.');
	trace_printf("%s\n", ext);
	if (strncmp(ext, ".wav", 4) == 0) {
		type = wav;
	} else if (strncmp(ext, ".ogg", 4) == 0) {
		type = ogg;
	} else {
		type = unsupported;
	}
}

int getRate(void) {
	if (type == wav) {
		return header.format.byterate;
	} else if (type == ogg) {
		return info->rate;
	}
}

void printINFO(void) {
	if (type == wav) {
		trace_printf("WAV\n");
	} else if (type == ogg) {
		trace_printf("OGG\n");
	}
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

void closefile(void) {
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
	HAL_TIM_Base_Stop(&htim2);
	if (type == wav) {
		f_close(&pfile);
	} else if (type == ogg) {
		ov_clear(&vf);
	}
}

void pullData(void) {
	if (type == wav) {
		f_read(&pfile, output[pdata], sizeof(output[pdata]), &br);
		bytes_read = br;
	} else if (type == ogg) {
		bytes_read = ov_read(&vf, output[pdata], sizeof(output[pdata]),
				&bitstream);
	}
	dataProcess();
}

void dataProcess(void) {
	uint8_t *temp8;
	unsigned short int *temp16;
	switch (s_fmt) {
	case PCM_8_mono:
		break;
	case PCM_8_stereo:
		temp8 = malloc(bytes_read);
		for (int i = 0; i < bytes_read / 2; i++) {
			temp8[i] = output[pdata][i];
			temp8[i + bytes_read / 2] = output[pdata][i + 1];
		}
		memcpy(output[pdata], temp8, bytes_read);
		break;
	case PCM_16_mono:
		for (int i = 0; i < bytes_read / 2; i++) {
			output[pdata][i] ^= 0x8000;
		}
		break;
	case PCM_16_stereo:
		temp16 = malloc(bytes_read);
		for (int i = 0; i < bytes_read / 4; i++) {
			temp16[i] = output[pdata][2 * i] ^ 0x8000;
			temp16[i + bytes_read / 4] = output[pdata][2 * i + 1] ^ 0x8000;
		}
		memcpy(output[pdata], temp16, bytes_read);
		break;
	}
}

void Start_DMA(void) {
	switch (s_fmt) {
	case PCM_8_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read,
				DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) output[pdata],
				bytes_read,
				DAC_ALIGN_8B_R);
		break;
	case PCM_8_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &(output[pdata][bytes_read / 2]), bytes_read / 2,
				DAC_ALIGN_8B_R);
		break;
	case PCM_16_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_12B_L);
		break;
	case PCM_16_stereo:
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

void TIM_reINIT(uint16_t sampleRate) {
//	I don't know why,but both number has to be non-zero
//	May be the pluse is not long enough
	HAL_TIM_Base_DeInit(&htim2);
	htim2.Init.Prescaler = 36000000 / sampleRate - 1;
	htim2.Init.Period = 1;
	HAL_TIM_Base_Init(&htim2);
}

bool End(void) {
	if (type == wav) {
		if (bytes_finish > header.data.size) {
			return 1;
		} else {
			bytes_finish += bytes_read;
		}
	} else if (type == ogg) {
		return f_eof(&pfile);
	} else {
		return 0;
	}
}

getTime() {

}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	HAL_TIM_Base_Stop(&htim2);
	if (!End()) {
		Start_DMA();
		pullData();
	}
}
