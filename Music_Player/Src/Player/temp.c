#include <temp.h>
#include "fatfs.h"

FIL pfile;
UINT br;
BYTE buffer[1024];

short int output[2048];
OggVorbis_File *vf;
char *initial;
long ibytes;
ov_callbacks callbacks;
int *bitstream;

vorbis_info *info;

int bytes_read;

void oggPlayer(char *fname) {
	FRESULT res;
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		trace_printf("Opened\n");
		f_read(&pfile, buffer, sizeof(buffer), &br);
		ov_open_callbacks(buffer, vf, initial, ibytes, callbacks);
		//bytes_read = ov_read(vf, output, sizeof(output), bitstream);
		ov_raw_tell(vf);
		info = ov_info(vf, -1);
		trace_printf("rate:\t%d\n", info->rate);
		trace_printf("channels:\t%d\n", info->channels);
		ov_clear(vf);
	}
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {

}
