#include <DIR.h>
#include "UI.h"
#include "lcd.h"
#include "string.h"
#include "Trace.h"
#include "wav.h"

extern char path[512];
extern char currentList[20][_MAX_LFN];
char playingList[20][_MAX_LFN];
char playingPath[512];
int playingCursor;
extern enum stat status;
unsigned int cursor, pageStart, pageEnd;
int count;

void UI_INIT(void) {
    status = Pause;
	LCD_INIT();
	strcpy(path, "/");
	count = getDirList();
	cursor = 0;
	pageStart = 0;
	pageEnd = (count < 12) ? count : 12;
	fileListUpdate();
	LCD_DrawLine(19, 262, 19, 273, BLACK);
	LCD_DrawLine(221, 262, 221, 273, BLACK);
	LCD_DrawLine(20, 267, 220, 267, BLACK);
	LCD_DrawLine(20, 268, 220, 268, BLACK);

	Update_Button(status);
}

void Update_Button(int status) {
	uint8_t ucTemp, ucPage, ucColumn;

	LCD_OpenWindow(62, 296, 16, 16);
	LCD_Write_Cmd( CMD_SetPixel);
	if (cursor != 0) {
		for (ucPage = 0; ucPage < 32; ucPage++) {
			ucTemp = icon[0][ucPage];
			for (ucColumn = 0; ucColumn < 8; ucColumn++) {
				if (ucTemp & 0x80)
					LCD_Write_Data(0xFFFF);
				else
					LCD_Write_Data(0x001F);
				ucTemp <<= 1;
			}
		}
	} else {
		for (ucPage = 0; ucPage < 32; ucPage++) {
			ucTemp = icon[0][ucPage];
			for (ucColumn = 0; ucColumn < 8; ucColumn++) {
				if (ucTemp & 0x80)
					LCD_Write_Data(0xFFFF);
				else
					LCD_Write_Data(0x5AEB);
				ucTemp <<= 1;
			}
		}
	}
	LCD_OpenWindow(162, 296, 16, 16);
	LCD_Write_Cmd( CMD_SetPixel);
	ucTemp = icon[3][ucPage];
	if (cursor != count - 1) {
		for (ucColumn = 0; ucColumn < 8; ucColumn++) {
			for (ucPage = 0; ucPage < 32; ucPage++) {
				ucTemp = icon[3][ucPage];
				for (ucColumn = 0; ucColumn < 8; ucColumn++) {
					if (ucTemp & 0x01)
						LCD_Write_Data(0xFFFF);
					else
						LCD_Write_Data(0x001F);
					ucTemp >>= 1;
				}
			}
		}
	} else {
		for (ucColumn = 0; ucColumn < 8; ucColumn++) {
			for (ucPage = 0; ucPage < 32; ucPage++) {
				ucTemp = icon[3][ucPage];
				for (ucColumn = 0; ucColumn < 8; ucColumn++) {
					if (ucTemp & 0x01)
						LCD_Write_Data(0xFFFF);
					else
						LCD_Write_Data(0x5AEB);
					ucTemp >>= 1;
				}
			}
		}
	}
	LCD_OpenWindow(112, 296, 16, 16);
	LCD_Write_Cmd( CMD_SetPixel);
	if (status == Pause) {
		for (ucPage = 0; ucPage < 32; ucPage++) {
			ucTemp = icon[1][ucPage];
			for (ucColumn = 0; ucColumn < 8; ucColumn++) {
				if (ucTemp & 0x80)
					LCD_Write_Data(0xFFFF);
				else
					LCD_Write_Data(0x001F);
				ucTemp <<= 1;
			}
		}
	} else if (status == Play) {
		for (ucPage = 0; ucPage < 32; ucPage++) {
			ucTemp = icon[2][ucPage];
			for (ucColumn = 0; ucColumn < 8; ucColumn++) {
				if (ucTemp & 0x80)
					LCD_Write_Data(0xFFFF);
				else
					LCD_Write_Data(0x001F);
				ucTemp <<= 1;
			}
		}
	}
}

void fileListUpdate() {
	for (int i = 0; i <= 12; ++i) {
		LCD_OpenWindow(0, 8 + 18 * i, 240, 16);
		if (i == cursor - pageStart) {
		char [27] temp;
		strncpy(temp, currentList[i]);
		strcat(temp,'\0');

			LCD_FillColor(240 * 16, BLACK);
			LCD_DrawString_Reversed(8, 8 + i * 18, temp[pageStart + i]);
		} else {
			LCD_FillColor(240 * 16, WHITE);
			LCD_DrawString(8, 8 + i * 18, temp[pageStart + i]);
		}
	}
}

void cursorUP(void) {
	if (cursor > 0) {
		--cursor;
		if ((cursor < pageStart + 6) && (pageStart > 0)) {
			--pageStart;
			--pageEnd;
		}
		fileListUpdate();
	}
	//trace_printf("%d\n", cursor);
}

void cursorDown(void) {
	if (cursor < count - 1) {
		++cursor;
		if ((cursor > pageEnd - 6) && (pageEnd < count - 1)) {
			++pageStart;
			++pageEnd;
		}
		fileListUpdate();
	}
	//trace_printf("%d\n", cursor);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_14) {
		int temp = dir_open(currentList[cursor]);
		if (temp != -1) {
			count = temp;
			trace_printf("%d\n", count);
			cursor = 0;
			pageStart = 0;
			pageEnd = (count < 12) ? count : 12;
			fileListUpdate();
			Update_Button(status);
		} else {
			closefile();
			char file[_MAX_LFN];
			strcpy(file, path);
			if (strcmp(path, "/") != 0)
				strcat(file, "/");
			strcat(file, currentList[cursor]);
			trace_printf("%s\n", file);
			wavPlayer(file);
			status = play;

			memcpy(playingList, currentList, 20 * _MAX_LFN);
			strcpy(playingPath, path);
			playingCursor = cursor;
			Update_Button(status);

			LCD_OpenWindow(0, 296, 61, 16);
			LCD_FillColor(61 * 16, WHITE);
			LCD_OpenWindow(0, 278, 240, 16);
			LCD_FillColor(240 * 16, WHITE);
			LCD_DrawString(8, 278, currentList[cursor]);
		}
	}
}
