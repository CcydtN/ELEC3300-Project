#include "ffconf.h"
#include "string.h"
#include "scanFile.h"
#include "Trace.h"

char path[512];
char currentList[20][255];

FRESULT scan_files(char *path /* Pointer to the working buffer with start path */
) {
	DIR dirs;
	FRESULT res;
	int i;
	char *fn;
	FILINFO Finfo;
	res = f_opendir(&dirs, path);
	if (res == FR_OK) {
		trace_printf("Opened Dir\n");
		trace_printf("%s\n", path);
		i = strlen(path);
		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {

			if (_FS_RPATH && Finfo.fname[0] == '.')
				continue;
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if (Finfo.fattrib & AM_DIR) {
				path[i] = '/';
				strcpy(path + i + 1, fn);
				res = scan_files(path);
				path[i] = '\0';
				if (res != FR_OK)
					break;
			} else {
				trace_printf("%s%s", path, fn);
			}
		}
	}
	return res;
}

void list_INIT() {
	strcpy(path, "");
	getList();
}

void getList(void) {
	DIR dirs;
	FRESULT res;
	int i;
	char *fn;
	FILINFO Finfo;
	uint8_t count = 1;

	strcpy(currentList[0], "/..");

	res = f_opendir(&dirs, path);
	if (res == FR_OK) {
		trace_printf("Opened Dir");
		trace_printf("%s", path);
		i = strlen(path);

		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
			if (_FS_RPATH && Finfo.fname[0] == '.')
				continue;
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
				fn = Finfo.fname;
	#endif
			if (Finfo.fattrib & AM_DIR) {
				path[i] = '/';
				strcpy(currentList[count], "./");
				strcpy(&currentList[count][3], fn);
				strcpy(path + i + 1, fn);
				path[i] = '\0';
				if (res != FR_OK)
					break;
			} else {
				strcpy(currentList[count], fn);
			}
			count++;
		}
	}
}
