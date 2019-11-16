#include "UI.h"
#include "lcd.h"
#include "string.h"

void UI_INIT(void) {
	LCD_INIT();
	LCD_DrawBox(0, 200, 240, 120, RED, CYAN);
}
