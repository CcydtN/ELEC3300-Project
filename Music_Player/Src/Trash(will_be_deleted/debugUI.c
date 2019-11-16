#include "lcd.h"
#include "debugUI.h"
#include <string.h>
#include "math.h"

static char line[SIZE][LENGTH];
static int start, end;
static OnOff trigger;

void DebugUI_INIT(OnOff input) {
	trigger = input;
	start = 0;
	end = SIZE - 1;
	if (trigger) {
		LCD_INIT();
	}
	char init[] = "*******debugUI Start********";
	DebugUI_push(init);
}

void DebugUI_update(void) {
	if (trigger) {
		int index = start;
		for (int i = 0; i < SIZE; i++) {
			LCD_DrawString(12, 10 + HEIGHT * i, line[index]);
			LCD_Clear(12 + WIDTH_EN_CHAR * strlen(line[index]), 10 + HEIGHT * i,
					216, HEIGHT_EN_CHAR, WHITE);
			index++;
			index %= SIZE;
		}
	}
}

void DebugUI_push(char *message) {
	if (start == end) {
		start = (start + 1) % SIZE;
	};
	int n = (int) fmin(strlen(message), LENGTH - 1);
	strncpy(line[end], message, n);
	line[end][n] = '\0';
	end = (end + 1) % SIZE;
	DebugUI_update();
	if (strlen(message) > LENGTH) {
		DebugUI_push(message + LENGTH - 1);
	}

}

void DebugUI_pushValue(int val) {
	if (start == end) {
		start = (start + 1) % SIZE;
	};
	sprintf(line[end], "%d", val);
	end = (end + 1) % SIZE;
	DebugUI_update();
}

void DebugUI_test(void) {
	int i;
	for (i = 0; i < SIZE; i++) {
		DebugUI_pushValue(i);
	}
	char test[LENGTH + 1];
	for (i = 0; i < sizeof(test); i++) {
		sprintf(test + i, "%x", i % 16);
	}
	DebugUI_push(test);
}
