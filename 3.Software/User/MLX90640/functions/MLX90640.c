#include "MLX90640.h"
#include "config.h"

/*
MLX90640 的测量速率最高可以达到 64Hz，但越快的速率时的噪声会越大，导致灵敏度下降，手册上给出的指标是 1Hz 时可以区分出 0.1℃。
测温范围是-40~300℃。
测温精度和成像的区域有关，靠近中间位置是±0.5~1.0℃，最外侧 4 个角是±2.0℃，其它区域约是±1.0℃。
传感器上电后有个热平衡时间，大约是 5 分钟，未达到热平衡时精度会差一些。
*/

#define ILI9341_Write_Data1(data){\
DATAOUT(data);\
ILI9341_WR_CLR;\
ILI9341_WR_SET;\
}

//						 31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9	 8	 7	 6	 5	 4	 3	 2	 1	0
const uint8_t Pos_x[] = {159,155,150,145,140,134,129,124,119,114,109,103,98, 93, 88, 83, 78, 72, 67, 62, 57, 52, 47, 41, 36, 31, 26, 21, 16, 10, 5, 0};
//						 0	 1	 2	 3	 4	 5	 6	 7	 8	 9	 10  11  12  13  14  15  16  17  18  19  20  21  22  23
const uint8_t Pos_y[] = {0,  5,  10, 16, 21, 26, 31, 37, 42, 47, 52, 57, 63, 68, 73, 78, 84, 89, 94, 99, 104,110,115,119};

uint16_t max, min;	// Maximum and minimum value found in current frame (global to avoid redundant parameters)
uint32_t emax, emin;// Effective maximum and minimum value (may be overriden by a fixed range)
uint32_t scale;		// How many steps of the color scale correspond to one step of To (0.1 °C)

#ifdef PROFILE
int16_t diff = 0;
#endif

#define SRC_MAX_X 32
#define SRC_MAX_Y 24
#define DST_MAX_X 160
#define DST_MAX_Y 120

// Set fixed-point precision used for bilinear interpolation.
const uint8_t FIXED_POINT = 8;
// One-pixel step in the destination image equals SRC_STEP_ pixels in the source (fixed-point, fractional number).
const uint16_t SRC_STEP_X = 0.19375 * (1 << FIXED_POINT); // original: 198  //0.19375 * 1024
const uint16_t SRC_STEP_Y = 0.19166 * (1 << FIXED_POINT); // original: 196  //0.19166 * 1024

// Draw specified lines of the thermal image to the display buffer or directly to the display.
// Performs bilinear interpolation from the 32x24 to 320x240 pixels and maps temperature values to colors.
void draw_thermal(uint8_t start, uint8_t end, bool direct) {
	uint32_t dst_x, dst_y;
	uint32_t src_x, src_y;
	uint32_t src_color, weight_x[2], weight_y[2];
	uint32_t dst_color, dst_addr;

	if (!direct && end - start + 1 > 10) {
		for (dst_x = 0; dst_x < 2 * DST_MAX_X; dst_x++) {
			data.DisBuf[dst_addr++] = 253;		// red (or other hottest non-GUI color)
		}
		return;		// out of range for buffered draw
	}

	dst_addr = 0;
	start /= 2;					// drawing is done at half resolution, adjust the coordinates
	end /= 2;
	src_y = start * SRC_STEP_Y;	// exact, fixed-point representation of target y coordinate in the source

	for (dst_y = start; dst_y <= end; dst_y++) {
		weight_y[1] = src_y & ((1 << FIXED_POINT) - 1);	// v
		weight_y[0] = (1 << FIXED_POINT) - weight_y[1];	// 1 - v

		src_x = 0;
		for (dst_x = 0; dst_x < DST_MAX_X; dst_x++)
		{
			weight_x[1] = src_x & ((1 << FIXED_POINT) - 1);	// u
			weight_x[0] = (1 << FIXED_POINT) - weight_x[1];	// 1 - u

			// Find a source "anchor pixel" by discarding decimal points of precise coordinates
			src_color = (src_y >> FIXED_POINT) * 32 + 31 - (src_x >> FIXED_POINT);
			// Perform bilinear interpolation between the anchor and 3 other pixels next ot it
			dst_color = (
				data2.mlx90640To[src_color	   ] * weight_x[0] * weight_y[0] +
				data2.mlx90640To[src_color + 32] * weight_x[0] * weight_y[1] +
				data2.mlx90640To[src_color -  1] * weight_x[1] * weight_y[0] +
				data2.mlx90640To[src_color + 31] * weight_x[1] * weight_y[1]) >> (2 * FIXED_POINT);

			// Color scale mapping (one bit of temperature difference equals [scale] steps in the color scale LUT).
			if (dst_color < emin) {
				dst_color = 0;
			} else if (dst_color > emax) {
				dst_color = 253;
			} else {
				dst_color = ((dst_color - emin) * scale) / 256;
			}

			// Drawing is done at half resolution, each pixel and line is duplicated
			// (unless in direct drawing mode, in which case duplication happens more efficiently later)
			data.DisBuf[dst_addr++] = dst_color;
			if (!direct) data.DisBuf[dst_addr++] = dst_color;

			src_x += SRC_STEP_X;
		}

		if (direct) {
			// Skip the buffer and write directly to the LCD
			for (uint8_t i = 0; i < 2; i++) {
			/*	for (dst_x = 0; dst_x < 2 * DST_MAX_X; dst_x++) {
					//ILI9341_Write_Data1(camColors[data.DisBuf[dst_x]]);
				}*/
				// Load 4 values at the same time (in total about 2 ms faster) and duplicate after CAM lookup.
				for (dst_x = 0; dst_x < DST_MAX_X; dst_x = dst_x + 4) {
					const uint32_t temp = *((uint32_t*)(&data.DisBuf[dst_x]));
					const uint32_t w1 = camColors[(temp & 0xff)];
					const uint32_t w2 = camColors[(temp >> 8) & 0xff];
					const uint32_t w3 = camColors[(temp >> 16) & 0xff];
					const uint32_t w4 = camColors[(temp >> 24)];
					ILI9341_Write_Data1(w1);
					ILI9341_Write_Data1(w1);
					ILI9341_Write_Data1(w2);
					ILI9341_Write_Data1(w2);
					ILI9341_Write_Data1(w3);
					ILI9341_Write_Data1(w3);
					ILI9341_Write_Data1(w4);
					ILI9341_Write_Data1(w4);
				}
			}
			dst_addr = 0;	// Reset the buffer so it does not overflow (direct writes tend to be long)
		} else {
			// Just duplicate the finished line
			for (dst_x = 0; dst_x < 2 * DST_MAX_X; dst_x++) {
				data.DisBuf[dst_addr++] = data.DisBuf[dst_addr - 2 * DST_MAX_X];
			}
		}

		src_y += SRC_STEP_Y;
	}
}


void write_buffer(uint8_t lines) {
	// 7600 cycles (0.64 ms) per 10 lines
/*	for (uint16_t x = 0; x < lines * 320; x++) {
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);
	}
*/
	// 6900 cycles (0.58 ms) per 10 lines
	for (uint16_t x = 0; x < lines * 320; x = x + 4) {
		const uint32_t temp = *((uint32_t*)(&data.DisBuf[x]));
		ILI9341_Write_Data1(camColors[(temp & 0xff)]);
		ILI9341_Write_Data1(camColors[(temp >> 8) & 0xff]);
		ILI9341_Write_Data1(camColors[(temp >> 16) & 0xff]);
		ILI9341_Write_Data1(camColors[(temp >> 24)]);
	}
}


// Draw the thermal image and GUI overlay to the display.
// Due to memory limitation, the work buffer is very small and data must be dumped to the LCD every 10 lines.
void display_image(void)
{
	uint16_t x, y;
	uint16_t Pos_max, Pos_min;

#ifdef PROFILE
	SysTick->VAL = 0;
#endif

	// The datasheet states measurement range is -40 to 300 °C; sensor returns 10 * (x + 40), i.e. 0 to 3400.
	// 测温范围是-40~300℃,温度值放大了10倍：-400~3000，并向上平移400：0~3400
	// However, when testing, temperature of at least 553 °C was seen → suspected range 0 to 8162.

	// Find the minimum and maximum across all measured values (~1700 cycles / 0.15 ms)
	max = 0; min = 3400;
	for (x = 0; x < 768; x++)
	{
		if (data2.mlx90640To[x] > max) {max = data2.mlx90640To[x]; Pos_max = x;}
		if (data2.mlx90640To[x] < min) {min = data2.mlx90640To[x]; Pos_min = x;}
	}

	// Set the effective minumum and maximum
	if (range_low < -40 || range_high < -40) {
		// Automatic range (including a mild averaging to counter the noise; stronger at higher rates)
		emin = (refresh_rate / 2 * emin + min) / (refresh_rate / 2 + 1);
		emax = (refresh_rate / 2 * emax + max) / (refresh_rate / 2 + 1);
	} else {
		// Fixed range, use user-defined values
		emin = (range_low + 40) * 10;
		emax = (range_high + 40) * 10;
	}

	// Set the temperature scale: how many steps of the color scale correspond to one step of To (0.1 °C).
	// (Only 253 colors are available for temperature mapping; multiplied by 256 for increased precision).
	scale = 64768 / (emax - emin);

	// Prepare the display to receive a new full frame
	LCD_setwindow(0, 0, 319, 239);
	ILI9341_DC_SET;
	ILI9341_CS_CLR;

	// Start drawing.
	// There is no space for a frame buffer in the ARM, so everything is done in thin 10 pixel strips that fit.
	// Most of the time is taken by draw_thermal() and write_buffer() calls.
	// Indirect draw of 10 lines takes about 0.9 ms, the subsequent buffer dump takes around 0.64 ms (total 1.54 ms).
	// The direct mode is a bit more efficient and needs only 1.32 ms per 10 lines (about 15700 cycles).
	// In total, there are 2 direct draws of 85 lines = 22.5 ms and 6 indirect draws of 10 lines = 9.25 ms.
	// Battery drawing takes around 230 cycles, color scale around 0.15 ms, one string ~0.08 ms (both halves).

	// Top GUI
	// 0..9: Battery icon, color scale, menu text.
	draw_thermal(0, 9, false);
	Disp_BatPower();
	// color scale (without last two values used for white / black GUI elements)
	for (y = 3; y <= 8; y++) {
		uint16_t adr = 320 * y + 30;
		for (x = 0; x < 254; x++) {
			data.DisBuf[adr++] = x;
		}
	}
	// Draw compact menu text (selected action). Only 4 lower-case characters will fit.
	// Set menu_tweak to 0 if you need to use letters that reach the bottom line (gpqy).
	// Otherwise set menu_tweak to 2 to avoid cutting the top of tall letters (bdfhiklt). "j" won't fit either way.
	Buf_ShowString(287, menu_tweak, menu_text, BUF_BLACK, 2);
	write_buffer(10);

	// 10..19: Top half of min / max temperature readings and menu value.
	draw_thermal(10, 19, false);
	Buf_SmallFloatNum(30, 2, emin - 400, BUF_BLACK, 0, false);
//	Buf_SmallFloatNum(96, 2, (ADCValue[0] * 660) >> 12, BUF_BLACK, 0, false);	// battery voltage debug
	Buf_SmallFloatNum(228, 2, emax - 400, BUF_BLACK, 0, true);
	Buf_ShowString(287, 2, menu_value, BUF_BLACK, 0);
	write_buffer(10);


	// 20..29: Bottom half of min / max temperature readings and menu value.
	draw_thermal(20, 29, false);
	Buf_SmallFloatNum(30, 0, emin - 400, BUF_BLACK, 1, false);
//	Buf_SmallFloatNum(96, 0, (ADCValue[0]*660) >> 12, BUF_BLACK, 1, false);
	Buf_SmallFloatNum(228, 0, emax - 400, BUF_BLACK, 1, true);
	Buf_ShowString(287, 0, menu_value, BUF_BLACK, 1);
	write_buffer(10);


	// Middle section: direct draw without GUI
	draw_thermal(30, 115, true);

	// Draw cross to the buffer (direct draw to display would cause visible blinking)
	draw_thermal(116, 125, false);
	Buf_Fill(156, 5, 164, 5, BUF_WHITE);  //x 160
	Buf_Fill(160, 1, 160, 9, BUF_WHITE);  //y 120 中心+字标
	write_buffer(10);

	draw_thermal(126, 219, true);	// direct draw without GUI


	// Bottom GUI
	draw_thermal(220, 229, false);
	// draw emissivity, profiler, Tc, Ta
	Buf_ShowString(4, 2, "e=0.", BUF_BLACK, 0);
	Buf_ShowNum(36, 2, emissivity * 100, BUF_BLACK, 0);						//辐射系数
#ifdef PROFILE
	Buf_ShowString(60, 2, "draw:", BUF_BLACK, 0);
	Buf_ShowNum(100, 2, diff, BUF_BLACK, 0);
	Buf_ShowString(120, 2, "ms", BUF_BLACK, 0);
	Buf_ShowString(190, 2, "s:", BUF_BLACK, 0);
	Buf_ShowNum(202, 2, sensor_diff, BUF_BLACK, 0);
	Buf_ShowString(225, 2, "ms", BUF_BLACK, 0);
#endif
//	Buf_SmallFloatNum(140, 2, center - 400, BUF_BLACK, 0, false);					//中心温度
	Buf_SmallFloatNum(140, 2, data2.mlx90640To[368] - 400, BUF_BLACK, 0, false);	//中心温度	/ center temperature
	Buf_ShowString(240, 2, "Ta:", BUF_BLACK, 0);
	Buf_SmallFloatNum(264, 2, Ta * 10, BUF_BLACK, 0, false);						//外壳温度	/ ambient temperature
	write_buffer(10);

	draw_thermal(230, 239, false);
	// draw emissivity, profiler, Tc, Ta
	Buf_ShowString(4, 0, "e=0.", BUF_BLACK, 1);
	Buf_ShowNum(36, 0, emissivity * 100, BUF_BLACK, 1);						//辐射系数
#ifdef PROFILE
	Buf_ShowString(60, 0, "draw:", BUF_BLACK, 1);
	Buf_ShowNum(100, 0, diff, BUF_BLACK, 1);
	Buf_ShowString(120, 0, "ms", BUF_BLACK, 1);
	Buf_ShowString(190, 0, "s:", BUF_BLACK, 1);
	Buf_ShowNum(202, 0, sensor_diff, BUF_BLACK, 1);
	Buf_ShowString(225, 0, "ms", BUF_BLACK, 1);
#endif
//	Buf_SmallFloatNum(140, 0, center - 400, BUF_BLACK, 1, false);					//中心温度
	Buf_SmallFloatNum(140, 0, data2.mlx90640To[368] - 400, BUF_BLACK, 1, false);	//中心温度
	Buf_ShowString(240, 0, "Ta:", BUF_BLACK, 1);
	Buf_SmallFloatNum(264, 0, Ta * 10, BUF_BLACK, 1, false);						//外壳温度
	write_buffer(10);


	// Final overlay: min / max coordinates (direct draw: they move around so blinking is not noticeable)
	// Only draw in auto mode (when max = emax, min = emin). Very cheap (~180 cycles).
	if (max == emax && Pos_max > 31)
	{
		x = Pos_x[Pos_max % 32] * 2;
		y = Pos_y[Pos_max / 32] * 2;
		LCD_Fill(x - 7, y, x + 7, y, MAGENTA);	  //x
		LCD_Fill(x, y - 7, x, y + 7, MAGENTA);	  //y 最大值中心+字标
	}
	if (min == emin && Pos_min > 31)
	{
		x = Pos_x[Pos_min % 32] * 2;
		y = Pos_y[Pos_min / 32] * 2;
		LCD_Fill(x - 7, y, x + 7, y, GREEN);	  //x
		LCD_Fill(x, y - 7, x, y + 7, GREEN);	  //y 最小值中心+字标
	}

#ifdef PROFILE
	diff = (0xFFFFFF - SysTick->VAL) / 11888;
#endif
}



// display repeating pattern
void Disp_test(void)
{
	uint16_t x, y;

	LCD_setwindow(0, 0, 319, 239);
	ILI9341_DC_SET;
	ILI9341_CS_CLR;

	for (y = 0; y < 240; y++)
	{
		for (x = 0; x < 320; x++)
		{
			DATAOUT(((32 - x/20 + y/16) << 10) + (y/8 << 5) + x/10);
			ILI9341_WR_CLR;
			ILI9341_WR_SET;
		}
	}
}
