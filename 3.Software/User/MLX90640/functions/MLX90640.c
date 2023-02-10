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
uint16_t emax, emin;// Effective maximum and minimum value (may be overriden by a fixed range)
uint16_t scale;		// How many steps of the color scale correspond to one step of To (0.1 °C)

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

/**********************************************************************************************************
*	函 数 名：Disp_TempPic
*	功能说明：显示温度图像
*	形	  参：
*	返 回 值：
**********************************************************************************************************/
#define scale_x   198  //0.19375 * 1024
#define scale_y   196  //0.19166 * 1024

void Disp_TempPic(void)
{
	uint16_t x, y, color, adr;
	uint16_t dst;
	uint16_t Pos_max, Pos_min;
	uint16_t fx, fy, cbufx[2], cbufy[2];
	uint8_t  sx, sy;

#ifdef PROFILE
	SysTick->VAL = 0;
#endif

	max = 0; min = 3400;	//测温范围是-40~300℃,温度值放大了10倍：-400~3000，并向上平移400：0~3400
	for (x = 0; x < 768; x++)
	{
		if (data2.mlx90640To[x] > max) {max = data2.mlx90640To[x]; Pos_max = x;}
		if (data2.mlx90640To[x] < min) {min = data2.mlx90640To[x]; Pos_min = x;}
	}

//	if (max - min > 2550 || max == min) {printf("max=%d, min=%d***************\r\n",max,min); return 1;}
	scale = 2530 / (max - min);
//	printf("max=%d,min=%d,scale=%d \r\n", max, min, scale);

	LCD_setwindow(0, 0, 319, 239);
	ILI9341_DC_SET;
	ILI9341_CS_CLR;
	/******************************************* 0-9行(0-4) 显示色条 **************************************************************
		0-31 32-287 288-319
		红外 色条	红外
	******************************************************************************************************************/
	fy = 0;
	adr = 0;
	for (y = 0; y < 5; y++)
	{
		sy = fy >> 10;						//SrcX中的整数

		cbufy[1] = fy & 0x3FF;				//v
		cbufy[0] = 1024 - cbufy[1];			//1-v

		fy += scale_y;

		/******************************************* 0-31列(0-15) *****************************/
		fx = 0;
		for (x = 0; x < 16; x++)
		{
			sx = fx >> 10;						//SrcX中的整数

			cbufx[1] = fx & 0x3FF;				//u
			cbufx[0] = 1024 - cbufx[1];			//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = ( data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;	//15L

			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		/******************************************* 32-285列 *****************************/
		for (x = 0; x < 254; x++)
		{
			data.DisBuf[adr++] = x;
		}

		/******************************************* 286-319列(143-159) *****************************/
		fx = 143 * scale_x;
		for (x = 143; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;						//SrcX中的整数
	
		  	cbufx[1] = fx & 0x3FF;				//u
		  	cbufx[0] = 1024 - cbufx[1];			//1-u
	
			fx += scale_x;
	
			color = sy * 32 + 31 - sx;
			dst = ( data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;
	
			dst = ((dst - min) * scale) / 10;
	
			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
		}
	}

	Disp_BatPower();

	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);	//73L
	}

	/******************************************* 10-19行(5-9) 显示MIN、MAX上半部分 ***************************************************************/
	adr = 0;
	for (; y < 10; y++)
	{
		sy = fy >> 10;						//SrcY中的整数

		cbufy[1] = fy & 0x3FF;				//v
		cbufy[0] = 1024 - cbufy[1];			//1-v

		fy += scale_y;

		fx = 0;
		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;					//SrcX中的整数

			cbufx[1] = fx & 0x3FF;			//u
			cbufx[0] = 1024 - cbufx[1];		//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = (data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
		}
	}

	Buf_ShowString(4, 2, "Min:", BUF_BLACK, 0);
	Buf_ShowString(230, 2, "Max:", BUF_BLACK, 0);
//	Buf_SmallFloatNum(36, 2, (ADCValue[0] * 660) >> 12, BUF_BLACK, 0, false);
	Buf_SmallFloatNum(36, 2, min - 400, BUF_BLACK, 0, false);
	Buf_SmallFloatNum(262, 2, max - 400, BUF_BLACK, 0, false);

	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);
	}

	/******************************************* 20-29行(10-14) 显示MIN、MAX下半部分 ***************************************************************/
	adr = 0;
	for (; y < 15; y++)
	{
		sy = fy >> 10;							//SrcY中的整数

		cbufy[1] = fy & 0x3FF;					//v
		cbufy[0] = 1024 - cbufy[1];				//1-v

		fy += scale_y;

		fx = 0;

		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;						//SrcX中的整数

			cbufx[1] = fx & 0x3FF;				//u
			cbufx[0] = 1024 - cbufx[1];			//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = (	data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
		}
	}

	Buf_ShowString(4, 0, "Min:", BUF_BLACK, 1);
	Buf_ShowString(230, 0, "Max:", BUF_BLACK, 1);
//	Buf_SmallFloatNum(36, 0, (ADCValue[0]*660) >> 12, BUF_BLACK, 1, false);
	Buf_SmallFloatNum(36, 0, min - 400, BUF_BLACK, 1, false);
	Buf_SmallFloatNum(262, 0, max - 400, BUF_BLACK, 1, false);

	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);
	}

	/******************************************* 30-115行(15-57) ***************************************************************/
	for (; y < 58; y++)
	{
		sy = fy >> 10;							//SrcY中的整数

		cbufy[1] = fy & 0x3FF;		 			//v
		cbufy[0] = 1024 - cbufy[1];				//1-v

		fy += scale_y;

		fx = 0;
		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;						//SrcX中的整数

			cbufx[1] = fx & 0x3FF;	   			//u
			cbufx[0] = 1024 - cbufx[1];			//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = (	data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			color = camColors[dst];
			data.DisBuf[x] = dst;
			ILI9341_Write_Data1(color);
			ILI9341_Write_Data1(color);
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			color=camColors[data.DisBuf[x]];
			ILI9341_Write_Data1(color);
			ILI9341_Write_Data1(color);
		}
	}

	/******************************************* 116-125行(58-62) ***************************************************************/
	adr = 0;
	for (; y < 63; y++)
	{
		sy = fy >> 10;							//SrcY中的整数

		cbufy[1] = fy & 0x3FF;		 			//v
		cbufy[0] = 1024 - cbufy[1];				//1-v

		fy += scale_y;

		fx = 0;
		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;						//SrcX中的整数

			cbufx[1] = fx & 0x3FF;	   			//u
			cbufx[0] = 1024 - cbufx[1];			//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = (	data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
		}
	}
/*
	fy=scale_y*y=196*60=11760
	cbufy[1] = fy & 0x3FF=496
	cbufy[0] = 1024 - cbufy[1]=528
	fx=scale_x*x=198*80=15840
	cbufx[1] = fx & 0x3FF=480
	cbufx[0] = 1024 - cbufx[1]=544
	sy=fy/1024=11
	sx=fx/1024=15
	color = sy * 32 + 31 - sx=368
	center = (	data2.mlx90640To[368] * cbufx[0] * cbufy[0] +
				data2.mlx90640To[400] * cbufx[0] * cbufy[1] +
				data2.mlx90640To[367] * cbufx[1] * cbufy[0] +
				data2.mlx90640To[399] * cbufx[1] * cbufy[1]) >> 20;
*/
//	center = (	data2.mlx90640To[368] * 544 * 528 +
//				data2.mlx90640To[400] * 544 * 496 +
//				data2.mlx90640To[367] * 480 * 528 +
//				data2.mlx90640To[399] * 480 * 496) >> 20;
	Buf_Fill(155, 5, 165, 5, BUF_WHITE);  //x 160
	Buf_Fill(160, 0, 160, 9, BUF_WHITE);  //y 120 中心+字标

	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);
	}

	/******************************************* 126-219行(63-109) ***************************************************************/
	for (; y < 110; y++)
	{
		sy = fy >> 10;									   //SrcY中的整数

		cbufy[1] = fy & 0x3FF;		 //v
		cbufy[0] = 1024 - cbufy[1];						 //1-v

		fy += scale_y;

		fx = 0;
		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;									 //SrcX中的整数

			cbufx[1] = fx & 0x3FF;	   //u
			cbufx[0] = 1024 - cbufx[1];					 //1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst =	 (	data2.mlx90640To[color	 ] * cbufx[0] * cbufy[0] +
						data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
						data2.mlx90640To[color - 1	] * cbufx[1] * cbufy[0] +
						data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			color=camColors[dst];
			data.DisBuf[x] = dst;
			ILI9341_Write_Data1(color);
			ILI9341_Write_Data1(color);
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			color=camColors[data.DisBuf[x]];
			ILI9341_Write_Data1(color);
			ILI9341_Write_Data1(color);
		}
	}

	/*************** 220-229行(110-114) 显示辐射系数、中心温度、外壳温度上半部分 ***********************************/
	adr = 0;
	for (; y < 115; y++)
	{
		sy = fy >> 10;							//SrcY中的整数

		cbufy[1] = fy & 0x3FF;		 			//v
		cbufy[0] = 1024 - cbufy[1];				//1-v

		fy += scale_y;

		fx = 0;
		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;						//SrcX中的整数

			cbufx[1] = fx & 0x3FF;	   			//u
			cbufx[0] = 1024 - cbufx[1];			//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = (	data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
		}
	}

	Buf_ShowString(4, 2, "e=0.", BUF_BLACK, 0);
	Buf_ShowNum(36, 2, emissivity * 100, BUF_BLACK, 0);								//辐射系数
//	Buf_SmallFloatNum(140, 2, center - 400, BUF_BLACK, 0, false);					//中心温度
	Buf_SmallFloatNum(140, 2, data2.mlx90640To[368] - 400, BUF_BLACK, 0, false);	//中心温度
	Buf_ShowString(240, 2, "Ta:", BUF_BLACK, 0);
	Buf_SmallFloatNum(264, 2, Ta * 10, BUF_BLACK, 0, false);						//外壳温度

#ifdef PROFILE
	Buf_ShowString(60, 2, "draw:", BUF_BLACK, 0);
	Buf_ShowNum(100, 2, diff, BUF_BLACK, 0);
	Buf_ShowString(120, 2, "ms", BUF_BLACK, 0);
#endif

	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);
	}

	/********************************* 230-239行(115-119) 显示辐射系数、中心温度、外壳温度下半部分 *****************************************************/
	adr = 0;
	for (; y < DST_MAX_Y; y++)
	{
		sy = fy >> 10;							//SrcY中的整数

		cbufy[1] = fy & 0x3FF;		 			//v
		cbufy[0] = 1024 - cbufy[1];				//1-v

		fy += scale_y;

		fx = 0;
		for (x = 0; x < DST_MAX_X; x++)
		{
			sx = fx >> 10;						//SrcX中的整数

			cbufx[1] = fx & 0x3FF;	   			//u
			cbufx[0] = 1024 - cbufx[1];			//1-u

			fx += scale_x;

			color = sy * 32 + 31 - sx;
			dst = (	data2.mlx90640To[color	   ] * cbufx[0] * cbufy[0] +
					data2.mlx90640To[color + 32] * cbufx[0] * cbufy[1] +
					data2.mlx90640To[color -  1] * cbufx[1] * cbufy[0] +
					data2.mlx90640To[color + 31] * cbufx[1] * cbufy[1]) >> 20;

			dst = ((dst - min) * scale) / 10;

			data.DisBuf[adr++] = dst;
			data.DisBuf[adr++] = dst;
		}

		for (x = 0; x < DST_MAX_X; x++)
		{
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
			data.DisBuf[adr++] = data.DisBuf[adr - 320];
		}
	}

	Buf_ShowString(4, 0, "e=0.", BUF_BLACK, 1);
	Buf_ShowNum(36, 0, emissivity * 100, BUF_BLACK, 1);								//辐射系数
//	Buf_SmallFloatNum(140, 0, center - 400, BUF_BLACK, 1, false);					//中心温度
	Buf_SmallFloatNum(140, 0, data2.mlx90640To[368] - 400, BUF_BLACK, 1, false);	//中心温度
	Buf_ShowString(240, 0, "Ta:", BUF_BLACK, 1);
	Buf_SmallFloatNum(264, 0, Ta * 10, BUF_BLACK, 1, false);						//外壳温度

#ifdef PROFILE
	Buf_ShowString(60, 0, "draw:", BUF_BLACK, 1);
	Buf_ShowNum(100, 0, diff, BUF_BLACK, 1);
	Buf_ShowString(120, 0, "ms", BUF_BLACK, 1);
#endif

	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1(camColors[data.DisBuf[x]]);
	}
	/******************************************* 结束 ***************************************************************/
	if (Pos_max > 31)
	{
		x = Pos_x[Pos_max % 32] * 2;
		y = Pos_y[Pos_max / 32] * 2;
		LCD_Fill(x - 7, y, x + 7, y, MAGENTA);	  //x
		LCD_Fill(x, y - 7, x, y + 7, MAGENTA);	  //y 最大值中心+字标
	}
	if (Pos_min > 31)
	{
		x = Pos_x[Pos_min % 32] * 2;
		y = Pos_y[Pos_min / 32] * 2;
		LCD_Fill(x - 7, y, x + 7, y, GREEN);	  //x
		LCD_Fill(x, y - 7, x, y + 7, GREEN);	  //y 最小值中心+字标
	}

#ifdef PROFILE
	diff = (0xFFFFFF - SysTick->VAL) / 11888;	// counting down
#endif
}


// Draw specified lines of the thermal image to the display buffer or directly to the display.
// Performs bilinear interpolation from the 32x24 to 320x240 pixels and maps temperature values to colors.
void draw_thermal(uint8_t start, uint8_t end, bool direct) {
	uint16_t dst_x, dst_y;
	uint16_t src_x, src_y;
	uint16_t src_color, weight_x[2], weight_y[2];
	uint16_t dst_color, dst_addr;

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
				dst_color = ((dst_color - emin) * scale) / 200;
			}

			// Drawing is done at half resolution, each pixel and line is duplicated
			data.DisBuf[dst_addr++] = dst_color;
			data.DisBuf[dst_addr++] = dst_color;

			src_x += SRC_STEP_X;
		}

		if (direct) {
			// Skip the buffer and write directly to the LCD
			for (uint8_t i = 0; i < 2; i++) {
			/*	for (dst_x = 0; dst_x < 2 * DST_MAX_X; dst_x++) {
					//ILI9341_Write_Data1(camColors[data.DisBuf[dst_x]]);
				}*/
				// Load 4 values at the same time (in total about 2 ms faster).
				for (dst_x = 0; dst_x < 2 * DST_MAX_X; dst_x = dst_x + 4) {
					const uint32_t temp = *((uint32_t*)(&data.DisBuf[dst_x]));
					ILI9341_Write_Data1(camColors[(temp & 0xff)]);
					ILI9341_Write_Data1(camColors[(temp >> 8) & 0xff]);
					ILI9341_Write_Data1(camColors[(temp >> 16) & 0xff]);
					ILI9341_Write_Data1(camColors[(temp >> 24)]);
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
void Disp_TempNew(void)
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
		// Automatic range
		emin = min;
		emax = max;
	} else {
		// Fixed range, use user-defined values
		emin = (range_low + 40) * 10;
		emax = (range_high + 40) * 10;
	}

	// Set the temperature scale: how many steps of the color scale correspond to one step of To (0.1 °C).
	// (Only 253 colors are available for temperature mapping; multiplied by 200 for increased precision).
	scale = 50600 / (emax - emin);

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
