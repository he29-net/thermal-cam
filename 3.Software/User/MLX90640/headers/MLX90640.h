#ifndef __MLX90640_H
#define	__MLX90640_H

#include "bsp.h"

// Define refresh rates of the sensor.
// This rate applies to individual subpages; two subpages are needed for a full frame, so display FPS will be half.
#define  FPS05HZ  0x00	// 0.5 Hz
#define  FPS1HZ   0x01
#define  FPS2HZ   0x02
#define  FPS4HZ   0x03
#define  FPS8HZ   0x04
#define  FPS16HZ  0x05
#define  FPS32HZ  0x06
#define  FPS64HZ  0x07

#define  MLX90640_ADDR 		0x33
#define  DefaultRefreshRate	FPS8HZ		// 推荐用4Hz，干扰少 / 4 Hz is a good compromise between speed and noise
#define  MinimumRefreshRate	FPS1HZ		// Minimum refresh rate is limited by user preference
#define  MaximumRefreshRate	FPS16HZ		// Maximum refresh rate is limited by conversion and drawing code performance
#define  TA_SHIFT 			8			// Default shift for MLX90640 in open air

extern float Ta;
extern float emissivity;
extern uint8_t refresh_rate;

void draw_thermal(uint8_t start, uint8_t end, bool direct);
void write_buffer(uint8_t lines);
void display_image(void);

void Buf_Showchar(unsigned int x, unsigned int y,unsigned char c, unsigned int color,uint8_t UpOrDn);
void Buf_ShowString(unsigned int x, unsigned int y,const char *s, unsigned int color,uint8_t UpOrDn);
void Buf_ShowNum(unsigned int x, unsigned int y,int num, unsigned int color,uint8_t UpOrDn);
void Buf_SmallFloatNum(int x,int y,int16_t num,unsigned int color,uint8_t UpOrDn, bool alignRight);
void Buf_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint8_t color);
void Disp_BatPower(void);
void Disp_test(void);

#endif
