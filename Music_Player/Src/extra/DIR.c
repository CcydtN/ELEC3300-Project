#include <DIR.h>
#include "ff.h"
#include "ffconf.h"
#include "string.h"
#include "Trace.h"
#include "stdlib.h"

extern char path[512];
extern char currentList[20][_MAX_LFN / 2];

int getDirList(void) {
	DIR dirs;
	FRESULT res;
	FILINFO Finfo;

	TCHAR LFN[_MAX_LFN];
	uint8_t count = 0;
	char *fn;

	Finfo.lfname = LFN;
	Finfo.lfsize = _MAX_LFN;
	memset(currentList, 0, sizeof(currentList));
	if (strcmp(path, "/") != 0) {
		strcpy(currentList[0], "..");
		++count;
	}
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
			++count;
		}
		f_closedir(&dirs);
	}
	sort(count);
	return count;
}

int dir_open(char *pName) {
	FRESULT res = f_chdir(pName);
	trace_printf("%d\n", res);
	if (res == FR_OK) {
		f_getcwd(path, _MAX_LFN);
		return getDirList();
	} else {
		return -1;
	}
}

void sort(int n) {
	for (int i = 0; i < n; ++i) {
		char temp[_MAX_LFN];
		strcpy(temp, currentList[i]);
		int j = i - 1;
		while (j >= 0 && strcmp(temp, currentList[j]) < 0) {
			strcpy(currentList[j + 1], currentList[j]);
			--j;
		}
		strcpy(currentList[j + 1], temp);
	}
}

