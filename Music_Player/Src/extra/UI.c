#include <DIR.h>
#include "UI.h"
#include "lcd.h"
#include "string.h"
#include "Trace.h"
#include "ffconf.h"
#include "player.h"

char path[256];

char currentList[20][_MAX_LFN];
char playingList[20][_MAX_LFN];
char playingPath[256];
int playingCursor;
unsigned int cursor, pageStart, pageEnd;
int count;

void UI_INIT(void) {
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

	Update_Button();
}

void Update_Button() {
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
	if (getStatus() == Pause) {
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
	} else {
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
	char temp[27];
	for (int i = 0; i <= 12; ++i) {
		LCD_OpenWindow(0, 8 + 18 * i, 240, 16);
		strncpy(temp, currentList[pageStart + i], 26);
		temp[27] = '\0';
		if (i == cursor - pageStart) {
			LCD_FillColor(240 * 16, BLACK);
			LCD_DrawString_Reversed(8, 8 + i * 18, temp);
		} else {
			LCD_FillColor(240 * 16, WHITE);
			LCD_DrawString(8, 8 + i * 18, temp);
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
	if (GPIO_Pin == GPIO_PIN_13) {
		int temp = dir_open(currentList[cursor]);
		if (temp != -1) {
			count = temp;
//			trace_printf("Num of list element:\t%d\n", count);
			cursor = 0;
			pageStart = 0;
			pageEnd = (count < 12) ? count : 12;
			fileListUpdate();
			Update_Button();
		} else {
			closefile();
			char file[_MAX_LFN];
			strcpy(file, path);
			if (strcmp(path, "/") != 0)
				strcat(file, "/");

			playingCursor = cursor;
			strcat(file, currentList[playingCursor]);

			player(file);
			setStatus(1);

			memcpy(playingList, currentList, 20 * _MAX_LFN);
			strcpy(playingPath, path);
			Update_Button();

			LCD_OpenWindow(0, 296, 61, 16);
			LCD_FillColor(61 * 16, WHITE);
			LCD_OpenWindow(0, 278, 240, 16);
			LCD_FillColor(240 * 16, WHITE);
			LCD_DrawString(8, 278, currentList[cursor]);
		}
	}
}
