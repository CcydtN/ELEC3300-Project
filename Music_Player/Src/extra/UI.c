#include "UI.h"
#include "lcd.h"
#include "string.h"
#include "Trace.h"

void UI_INIT(void) {
	LCD_INIT();
//	LCD_DrawBox(0, 200, 240, 120, RED, CYAN);
	LCD_DrawBox(8, 8, 224, 16, RED, CYAN);   //1st song  ARR[0]
	LCD_DrawBox(8, 26, 224, 16, RED, CYAN);  //2nd song  ARR[1]
	LCD_DrawBox(8, 44, 224, 16, RED, CYAN);  //3rd song  ARR[2]
	LCD_DrawBox(8, 62, 224, 16, RED, CYAN);  //4th song  ARR[3]
	LCD_DrawBox(8, 80, 224, 16, RED, CYAN);  //5th song  ARR[4]
	LCD_DrawBox(8, 98, 224, 16, RED, CYAN);  //6th song  ARR[5]
	LCD_DrawBox(8, 116, 224, 16, RED, CYAN); //7th song  ARR[6]
	LCD_DrawBox(8, 134, 224, 16, RED, CYAN); //8th song  ARR[7]
	LCD_DrawBox(8, 152, 224, 16, RED, CYAN); //9th song  ARR[8]
	LCD_DrawBox(8, 170, 224, 16, RED, CYAN); //10th song ARR[9]
	LCD_DrawBox(8, 188, 224, 16, RED, CYAN); //11th song ARR[10]
	LCD_DrawBox(8, 206, 224, 16, RED, CYAN); //12th song ARR[11]
	LCD_DrawBox(8, 224, 224, 16, RED, CYAN); //13th song ARR[12]
	LCD_DrawBox(8, 242, 224, 16, RED, CYAN); //14th song ARR[13]
	LCD_DrawBox(8, 260, 224, 16, RED, CYAN); //bar
	LCD_DrawBox(8, 278, 224, 16, RED, CYAN); //name of current song
	LCD_DrawBox(8, 296, 224, 16, RED, CYAN); //buttons
	LCD_DrawLine(20, 262, 20, 273, BLACK);
	LCD_DrawLine(220, 262, 220, 273, BLACK);
	LCD_DrawLine(20, 267, 220, 267, BLACK);
	LCD_DrawLine(20, 268, 220, 268, BLACK);
	trace_printf("WTF\n");
}
