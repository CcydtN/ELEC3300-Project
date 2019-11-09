#include "ffconf.h"
#include "string.h"
#include "scanFile.h"

static FRESULT scan_files(char *path /* Pointer to the working buffer with start path */
) {
	DIR dirs;
	FRESULT res;
	int i;
	char *fn;
	FILINFO Finfo;
	res = f_opendir(&dirs, path);
	if (res == FR_OK) {
		DebugUI_push("Opened Dir");
		DebugUI_push(path);
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
				//char *temp;
				//sprintf(temp, "%s%s", path, fn);
				DebugUI_push("");
				DebugUI_push(path);
				DebugUI_push(fn);
			}
		}
	}

	return res;
}
