#include <wav.h>
#include "fatfs.h"
#include "debugUI.h"
#include "string.h"

static struct HEADER header;
static FIL pfile;
static UINT br;

void wav_INIT(char *fname) {

	FRESULT res;

	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK) {
		DebugUI_push("Opened");
		//
		getHeader();
		printHeader();
		//
		res = f_close(&pfile);
		if (res == FR_OK) {
			DebugUI_push("Closed");
		};

	}
}

void getHeader(void) {
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

	//Some wav have extra information we don't need right now;
	while (!(header.data_chunk_header[0] == 'd'
			&& header.data_chunk_header[1] == 'a'
			&& header.data_chunk_header[2] == 't'
			&& header.data_chunk_header[3] == 'a')) {
		pfile.fptr += header.data_size;
		f_read(&pfile, header.data_chunk_header,
				sizeof(header.data_chunk_header), &br);
		f_read(&pfile, &header.data_size, sizeof(header.data_size), &br);
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
