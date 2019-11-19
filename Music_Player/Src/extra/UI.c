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
	count = getDirList();
	cursor = 0;
	pageStart = 0;
	pageEnd = (count < 12) ? count : 12;
	strcpy(path, "/");
	fileListUpdate();
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

void fileListUpdate() {
	for (int i = 0; i <= 12; ++i) {
		LCD_OpenWindow(0, 8 + 18 * i, 240, 16);
		if (i == cursor) {
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
	trace_printf("%d", cursor);
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
	trace_printf("%d", cursor);
}
