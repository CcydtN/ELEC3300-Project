#include <DIR.h>
#include "ff.h"
#include "ffconf.h"
#include "string.h"
#include "Trace.h"

extern char path[512];
extern char currentList[20][_MAX_LFN];

void dir_INIT(void) {
	strcpy(path, "/");
	getDirList();
}

void getDirList(void) {
	DIR dirs;
	FRESULT res;
	FILINFO Finfo;

	TCHAR LFN[_MAX_LFN];
	uint8_t count = 1;
	char *fn;

	Finfo.lfname = LFN;
	Finfo.lfsize = _MAX_LFN;
	memset(currentList, 0, sizeof(currentList));

	strcpy(currentList[0], "..");

	res = f_opendir(&dirs, path);
	if (res == FR_OK) {
		trace_printf("Opened Dir\n");
		trace_printf("Current Path:\t%s\n", path);

		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if (_FS_RPATH && fn[0] == '.')
				continue;
			if (strcmp(fn, "System Volume Information") == 0)
				continue;

			if (Finfo.fattrib & AM_DIR) {
				strcpy(currentList[count], "./");
				strncpy(&currentList[count][2], fn, Finfo.lfsize);
			} else {
				strncpy(currentList[count], fn, Finfo.lfsize);
			}
			count++;
		}
		f_closedir(&dirs);
	}
}

void dir_open(char *pName) {
	FRESULT res = f_chdir(pName);
	if (res == FR_OK) {
		f_getcwd(path, _MAX_LFN);
		getDirList();
	} else {

	}
}
