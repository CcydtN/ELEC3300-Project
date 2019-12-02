#include <DIR.h>
#include "UI.h"
#include "lcd.h"
#include "string.h"
#include "Trace.h"
#include "wav.h"

extern char path[512];
extern char currentList[20][_MAX_LFN];
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

	const unsigned char icon[4][32] = { { 0xff, 0xff, 0xff, 0xff, 0xf3, 0xef,
			0xf3, 0xcf, 0xf3, 0x8f, 0xf3, 0x0f, 0xf2, 0x0f, 0xf0, 0x0f, 0xf0,
			0x0f, 0xf2, 0x0f, 0xf3, 0x0f, 0xf3, 0x8f, 0xf3, 0xcf, 0xf3, 0xef,
			0xff, 0xff, 0xff, 0xff }, //LastSong

			{ 0xff, 0xff, 0xff, 0xff, 0xf1, 0x8f, 0xf1, 0x8f, 0xf1, 0x8f, 0xf1,
					0x8f, 0xf1, 0x8f, 0xf1, 0x8f, 0xf1, 0x8f, 0xf1, 0x8f, 0xf1,
					0x8f, 0xf1, 0x8f, 0xf1, 0x8f, 0xf1, 0x8f, 0xff, 0xff, 0xff,
					0xff }, //play

			{ 0xff, 0xff, 0xf3, 0xff, 0xf1, 0xff, 0xf0, 0xff, 0xf0, 0x7f, 0xf0,
					0x3f, 0xf0, 0x1f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x1f, 0xf0,
					0x3f, 0xf0, 0x7f, 0xf0, 0xff, 0xf1, 0xff, 0xf3, 0xff, 0xff,
					0xff }, // Pause

			{ 0xff, 0xff, 0xff, 0xff, 0xef, 0xf3, 0xcf, 0xf3, 0x8f, 0xf3, 0x0f,
					0xf3, 0x0f, 0x0f2, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf2, 0x0f,
					0xf3, 0x8f, 0xf3, 0xcf, 0xf3, 0xef, 0xf3, 0xff, 0xff, 0xff,
					0xff } }; //LastSong

	uint8_t ucTemp, ucPage, ucColumn;

	for (int i = 0; i < 2; ++i) {
		LCD_OpenWindow(50 * i + 62, 296, 16, 16);
		LCD_Write_Cmd( CMD_SetPixel);
		for (ucPage = 0; ucPage < 32; ucPage++) {
			ucTemp = icon[i][ucPage];
			for (ucColumn = 0; ucColumn < 8; ucColumn++) {
				if (ucTemp & 0x80)
					LCD_Write_Data(0xFFFF);
				else
					LCD_Write_Data(0x001F);
				ucTemp <<= 1;

			}
		}
	}
	LCD_OpenWindow(162, 296, 16, 16);
	LCD_Write_Cmd( CMD_SetPixel);
	ucTemp = icon[3][ucPage];
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
}

void fileListUpdate() {
	for (int i = 0; i <= 12; ++i) {
		LCD_OpenWindow(0, 8 + 18 * i, 240, 16);
		if (i == cursor - pageStart) {
			LCD_FillColor(240 * 16, BLACK);
			LCD_DrawString_Reversed(8, 8 + i * 18, currentList[pageStart + i]);
		} else {
			LCD_FillColor(240 * 16, WHITE);
			LCD_DrawString(8, 8 + i * 18, currentList[pageStart + i]);
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
		} else {
			closefile();
			char file[_MAX_LFN];
			strcpy(file, path);
			if (strcmp(path, "/") != 0)
				strcat(file, "/");
			strcat(file, currentList[cursor]);
			trace_printf("%s\n", file);
			wavPlayer(file);
			LCD_OpenWindow(0, 278, 240, 16);
			LCD_FillColor(240 * 16, WHITE);
			LCD_DrawString(8, 278, currentList[cursor]);
		}
	}
}
