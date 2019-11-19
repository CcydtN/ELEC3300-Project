#include <DIR.h>
#include "UI.h"
#include "lcd.h"
#include "string.h"
#include "Trace.h"

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
	pageEnd = (count < 13) ? count : 13;
	for (int i = pageStart; i <= pageEnd; ++i) {
		LCD_DrawString(8, 8 + i * 18, currentList[i]);
	}
	LCD_DrawLine(19, 262, 19, 273, BLACK);
	LCD_DrawLine(221, 262, 221, 273, BLACK);
	LCD_DrawLine(20, 267, 220, 267, BLACK);
	LCD_DrawLine(20, 268, 220, 268, BLACK);

	const unsigned char icon[3][32] = { { 0xff, 0xff, 0xff, 0xff, 0xf3, 0xef,
			0xf3, 0xcf, 0xf3, 0x8f, 0xf3, 0x0f, 0xf2, 0x0f, 0xf0, 0x0f, 0xf0,
			0x0f, 0xf2, 0x0f, 0xf3, 0x0f, 0xf3, 0x8f, 0xf3, 0xcf, 0xf3, 0xef,
			0xff, 0xff, 0xff, 0xff },

	{ 0xff, 0xff, 0xff, 0xff, 0xe7, 0x7f, 0xe7, 0x3f, 0xe7, 0x1f, 0xe7, 0x0f,
			0xe7, 0x07, 0xe7, 0x03, 0xe7, 0x03, 0xe7, 0x07, 0xe7, 0x0f, 0xe7,
			0x1f, 0xe7, 0x3f, 0xe7, 0x7f, 0xff, 0xff, 0xff, 0xff },

	{ 0xff, 0xff, 0xff, 0xff, 0xef, 0xf3, 0xcf, 0xf3, 0x8f, 0xf3, 0x0f, 0xf3,
			0x0f, 0x0f2, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf2, 0x0f, 0xf3, 0x8f,
			0xf3, 0xcf, 0xf3, 0xef, 0xf3, 0xff, 0xff, 0xff, 0xff } };

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
	ucTemp = icon[0][ucPage];
	for (ucColumn = 0; ucColumn < 8; ucColumn++) {
		for (ucPage = 0; ucPage < 32; ucPage++) {
			ucTemp = icon[2][ucPage];
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

void cursorUP(void) {
	if (cursor != 0) {
		cursor--;
		if ((2 * cursor < pageStart + pageEnd) && (pageStart != 0)) {
			pageStart--;
			pageEnd--;
		}
	}
}

void cursorDown(void) {
	if (cursor != 20) {
		cursor++;
		if ((2 * cursor > pageStart + pageEnd) && (pageEnd != count)) {
			pageStart--;
			pageEnd--;
		}
	}
}
