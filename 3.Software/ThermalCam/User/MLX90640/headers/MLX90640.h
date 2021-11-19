#ifndef __MLX90640_H
#define	__MLX90640_H

#include "bsp.h"

#define  FPS2HZ   0x02
#define  FPS4HZ   0x03
#define  FPS8HZ   0x04
#define  FPS16HZ  0x05
#define  FPS32HZ  0x06

#define  MLX90640_ADDR 0x33
#define	 RefreshRate FPS4HZ  //推荐用4Hz，干扰少
#define  TA_SHIFT 8          //Default shift for MLX90640 in open air

extern float Ta;
extern float emissivity;
//extern uint16_t mlx90640To[768];
//extern uint16_t mlx90640_Zoom10[834];
//extern uint32_t BatteryVal;
//extern uint8_t  BatChrg_Sta;  //电池充电状态，1表示正在充电


int Disp_TempPic(void);
void Disp_Color_Bar(void);

void Buf_Showchar(unsigned int x, unsigned int y,unsigned char c, unsigned int color,uint8_t UpOrDn);
void Buf_ShowString(unsigned int x, unsigned int y,const char *s, unsigned int color,uint8_t UpOrDn);
void Buf_ShowNum(unsigned int x, unsigned int y,int num, unsigned int color,uint8_t UpOrDn);
void Buf_SmallFloatNum(int x,int y,int16_t num,unsigned int color,uint8_t UpOrDn);
void Buf_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint8_t color);
void Disp_BatPower(void);
void Disp_test();

#endif
