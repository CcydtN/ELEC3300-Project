#include "player.h"
#include "ff.h"
#include "ffconf.h"
#include "fatfs.h"
#include "string.h"
#include "Trace.h"

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

void player(char *fname) {
	checkExtension(fname);
	FRESULT res;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (type == wav) {

	} else if (type == ogg) {
		callbacks.read_func = alt_read;
		callbacks.seek_func = alt_seek;
		callbacks.close_func = alt_close;
		callbacks.tell_func = alt_tell;
		ov_open_callbacks(&pfile, &vf, NULL, 0, callbacks);

		info = ov_info(&vf, -1);
		pullData();
	}
	TIM_reINIT(36000000 / getRate() - 1, 1);
	pullData();
	Start_DMA();
	pullData();
}

void checkExtension(char *fname) {
	char *temp = strrchr(fname, '.');
	if (strncmp(temp, ".wav", 4)) {
		type = wav;
	} else if (strncmp(temp, ".ogg", 4)) {
		type = ogg;
	} else {
		type = unsupported;
	}
}

void getRate() {

}

void closefile(void) {
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
	HAL_TIM_Base_Stop(&htim2);
	if (type == wav) {
		f_close(&pfile);
	} else if (type == ogg) {

	}
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	if (type == wav) {

	} else if (type == ogg) {

	}
}
