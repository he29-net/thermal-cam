#include "MLX90640.h"


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

//色条颜色，共256色
//const uint16_t camColors[] = {       
//0x480F,0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,
//0x3810,0x3010,0x3010,0x3010,0x2810,0x2810,0x2810,0x2810,  
//0x2010,0x2010,0x2010,0x1810,0x1810,0x1811,0x1811,0x1011,
//0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,0x0011,
//0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,
//0x00B2,0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,
//0x0152,0x0172,0x0192,0x0192,0x01B2,0x01D2,0x01F3,0x01F3,
//0x0213,0x0233,0x0253,0x0253,0x0273,0x0293,0x02B3,0x02D3,
//0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,0x0394,
//0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,
//0x0474,0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,
//0x0554,0x0554,0x0574,0x0574,0x0573,0x0573,0x0573,0x0572,
//0x0572,0x0572,0x0571,0x0591,0x0591,0x0590,0x0590,0x058F,
//0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,0x05AD,
//0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,
//0x05C9,0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,
//0x05E6,0x05E6,0x05E5,0x05E5,0x0604,0x0604,0x0604,0x0603,
//0x0603,0x0602,0x0602,0x0601,0x0621,0x0621,0x0620,0x0620,
//0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,0x1E40,
//0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,
//0x3E60,0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,
//0x5E80,0x5E80,0x6680,0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,
//0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,0x8EC0,0x96C0,0x96C0,
//0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,0xBEE0,
//0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,
//0xDEE0,0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,
//0xE600,0xE5E0,0xE5C0,0xE5A0,0xE580,0xE560,0xE540,0xE520,
//0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,0xE460,0xEC40,0xEC20,
//0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,0xEB20,
//0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,
//0xF200,0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,
//0xF0E0,0xF0C0,0xF0A0,0xF080,0xF060,0xF040,0xF020,0xF800,
////0xffff,0x0000
//};
const uint16_t camColors[] = {       
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,
0x3810,0x3010,0x3010,0x3010,0x2810,0x2810,0x2810,0x2810,  
0x2010,0x2010,0x2010,0x1810,0x1810,0x1811,0x1811,0x1011,
0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,0x0011,
0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,
0x00B2,0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,
0x0152,0x0172,0x0192,0x0192,0x01B2,0x01D2,0x01F3,0x01F3,
0x0213,0x0233,0x0253,0x0253,0x0273,0x0293,0x02B3,0x02D3,
0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,0x0394,
0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,
0x0474,0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,
0x0554,0x0554,0x0574,0x0574,0x0573,0x0573,0x0573,0x0572,
0x0572,0x0572,0x0571,0x0591,0x0591,0x0590,0x0590,0x058F,
0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,0x05AD,
0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,
0x05C9,0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,
0x05E6,0x05E6,0x05E5,0x05E5,0x0604,0x0604,0x0604,0x0603,
0x0603,0x0602,0x0602,0x0601,0x0621,0x0621,0x0620,0x0620,
0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,0x1E40,
0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,
0x3E60,0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,
0x5E80,0x5E80,0x6680,0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,
0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,0x8EC0,0x96C0,0x96C0,
0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,0xBEE0,
0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,
0xDEE0,0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,
0xE600,0xE5E0,0xE5C0,0xE5A0,0xE580,0xE560,0xE540,0xE520,
0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,0xE460,0xEC40,0xEC20,
0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,0xEB20,
0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,
0xF200,0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,
0xF0E0,0xF0C0,0xF0A0,0xF080,0xF060,0xF040,0xF020,
0x0000,0xffff
};

//                       31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  
const uint8_t Pos_x[] = {159,155,150,145,140,134,129,124,119,114,109,103,98 ,93 ,88 ,83 ,78 ,72 ,67 ,62 ,57 ,52 ,47 ,41 ,36 ,31 ,26 ,21 ,16 ,10 ,5  ,0}; 	
//                       0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23 	
const uint8_t Pos_y[] = {0  ,5  ,10 ,16 ,21 ,26 ,31 ,37 ,42 ,47 ,52 ,57 ,63 ,68 ,73 ,78 ,84 ,89 ,94 ,99 ,104,110,115,119};                                	
/**********************************************************************************************************
*	函 数 名：Disp_TempPic
*	功能说明：显示温度图像
*	形    参：
*	返 回 值：
**********************************************************************************************************/
#define SRC_MAX_X 32
#define SRC_MAX_Y 24
#define DST_MAX_X 160
#define DST_MAX_Y 120
#define scale_x   198  //0.19375*1024
#define scale_y   196  //0.19166*1024

int Disp_TempPic(void)
{
	uint16_t x,y,color,adr;	
	uint16_t max,min,scale,dst;
	uint16_t Pos_max,Pos_min;	
	uint16_t fx,fy,cbufx[2],cbufy[2];
	uint8_t  sx,sy;	
	
	max=0;min=3400;       //测温范围是-40~300℃,温度值放大了10倍：-400~3000，并向上平移400：0~3400
	for(x=0;x<768;x++)
	{
		if(data2.mlx90640To[x]>max) { max=data2.mlx90640To[x]; Pos_max=x;}
		if(data2.mlx90640To[x]<min) { min=data2.mlx90640To[x]; Pos_min=x;}
	}
	
//	if(max-min>2550 || max==min) {printf("max=%d,min=%d***************\r\n",max,min); return 1;}
	scale=2530/(max-min);  
//	printf("max=%d,min=%d,scale=%d \r\n",max,min,scale);
	
	LCD_setwindow(0,0,319,239);  
	ILI9341_DC_SET;
	ILI9341_CS_CLR;		
/******************************************* 0-9行(0-4) 显示色条 **************************************************************   
	0-31 32-287 288-319		
	红外 色条   红外
******************************************************************************************************************/	
	fy=0; adr=0;	
    for (y = 0; y < 5; y++)
    {
        sy = fy>>10;                                       //SrcX中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
        /******************************************* 0-31列(0-15) *****************************/		
		fx=0;
        for (x = 0; x < 16; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] =fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   
			
			dst=( (dst-min)*scale )/10;  //15L

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;
        }

		/******************************************* 32-285列 *****************************/
        for (x = 0; x < 254; x++)
        {		
			data.DisBuf[adr++]=x;
        }

		/******************************************* 286-319列(143-159) *****************************/
		fx=143*scale_x;
        for (x = 143; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;
        }

        for (x = 0; x < DST_MAX_X; x++)
        {
			data.DisBuf[adr++]=data.DisBuf[adr-320]; 
			data.DisBuf[adr++]=data.DisBuf[adr-320];
        }		
    }
	
	Disp_BatPower();
		
	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1( camColors[ data.DisBuf[x] ] );	//73L	 	
	}	
	
/******************************************* 10-19行(5-9) 显示MIN、MAX上半部分 ***************************************************************/
	adr=0;
    for (; y < 10; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;
        }
		
        for (x = 0; x < DST_MAX_X; x++)
        {
			data.DisBuf[adr++]=data.DisBuf[adr-320]; 
			data.DisBuf[adr++]=data.DisBuf[adr-320];
        }		
    }

	Buf_ShowString( 4, 2,"Min:",BUF_BLACK,0);	
	Buf_ShowString(230, 2,"Max:",BUF_BLACK,0);
//	Buf_SmallFloatNum(36 ,2,(ADCValue[0]*660)>>12,BUF_BLACK,0);
	Buf_SmallFloatNum(36 ,2,min-400,BUF_BLACK,0);
	Buf_SmallFloatNum(262,2,max-400,BUF_BLACK,0);
				
	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1( camColors[ data.DisBuf[x] ] );			
	}	
/******************************************* 20-29行(10-14) 显示MIN、MAX下半部分 ***************************************************************/
	adr=0;
    for (; y < 15; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;
        }
		
        for (x = 0; x < DST_MAX_X; x++)
        {
			data.DisBuf[adr++]=data.DisBuf[adr-320]; 
			data.DisBuf[adr++]=data.DisBuf[adr-320];
        }		
    }

	Buf_ShowString( 4, 0,"Min:",BUF_BLACK,1);	
	Buf_ShowString(230, 0,"Max:",BUF_BLACK,1);
//	Buf_SmallFloatNum(36 ,0,(ADCValue[0]*660)>>12,BUF_BLACK,1);
	Buf_SmallFloatNum(36 ,0,min-400,BUF_BLACK,1);
	Buf_SmallFloatNum(262,0,max-400,BUF_BLACK,1);
				
	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1( camColors[ data.DisBuf[x] ] );			
	}		
/******************************************* 30-115行(15-57) ***************************************************************/		
    for (; y < 58; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			color=camColors[dst];	
			data.DisBuf[x]=dst;
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
	adr=0;
    for (; y < 63; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;    

			dst=( (dst-min)*scale )/10;

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;			
        }
		
        for (x = 0; x < DST_MAX_X; x++)
        {
			data.DisBuf[adr++]=data.DisBuf[adr-320]; 
			data.DisBuf[adr++]=data.DisBuf[adr-320];
        }		
    }	
/*
	fy=scale_y*y=196*60=11760
	cbufy[1] = fy&0x3FF=496         
	cbufy[0] = 1024-cbufy[1]=528     
	fx=scale_x*x=198*80=15840
	cbufx[1] = fx&0x3FF=480         
	cbufx[0] = 1024-cbufx[1]=544  
	sy=fy/1024=11
	sx=fx/1024=15
	color=sy*32+31-sx=368
	center = (  data2.mlx90640To[ 368 ] * cbufx[0] * cbufy[0] + 
				data2.mlx90640To[ 400 ] * cbufx[0] * cbufy[1] + 
				data2.mlx90640To[ 367 ] * cbufx[1] * cbufy[0] + 
				data2.mlx90640To[ 399 ] * cbufx[1] * cbufy[1] )>>20;  	
*/	
//	center = (  data2.mlx90640To[ 368 ] * 544 * 528 + 
//				data2.mlx90640To[ 400 ] * 544 * 496 + 
//				data2.mlx90640To[ 367 ] * 480 * 528 + 
//				data2.mlx90640To[ 399 ] * 480 * 496 )>>20;  
	Buf_Fill(155,5,165,5,BUF_WHITE);  //x 160 
	Buf_Fill(160,0,160,9,BUF_WHITE);  //y 120 中心+字标	
				
	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1( camColors[ data.DisBuf[x] ] );			
	}		
/******************************************* 126-219行(63-109) ***************************************************************/		
    for (; y < 110; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			color=camColors[dst];	
			data.DisBuf[x]=dst;
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
/******************************************* 220-229行(110-114) 显示辐射系数、中心温度、外壳温度上半部分 ***************************************************************/
	adr=0;
    for (; y < 115; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;
        }
		
        for (x = 0; x < DST_MAX_X; x++)
        {
			data.DisBuf[adr++]=data.DisBuf[adr-320]; 
			data.DisBuf[adr++]=data.DisBuf[adr-320];
        }		
    }

	Buf_ShowString(4,2,"e=0.",BUF_BLACK,0);
	Buf_ShowNum(36,2,emissivity*100,BUF_BLACK,0);	                    //辐射系数
//	Buf_SmallFloatNum(140 ,2,center-400,BUF_BLACK,0);	                //中心温度 
	Buf_SmallFloatNum(140 ,2,data2.mlx90640To[368]-400,BUF_BLACK,0);	//中心温度 	
	Buf_ShowString(240, 2,"Ta:",BUF_BLACK,0);
	Buf_SmallFloatNum(264 ,2,Ta*10,BUF_BLACK,0);                        //外壳温度
				
	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1( camColors[ data.DisBuf[x] ] );			
	}	
/******************************************* 230-239行(115-119) 显示辐射系数、中心温度、外壳温度下半部分 ***************************************************************/
	adr=0;
    for (; y < DST_MAX_Y; y++)
    {
        sy = fy>>10;                                       //SrcY中的整数 	
		
        cbufy[1] = fy&0x3FF;         //v
        cbufy[0] = 1024-cbufy[1];                      //1-v
		
		fy+=scale_y;
		
		fx=0;
        for (x = 0; x < DST_MAX_X; x++)
        {
			sx = fx>>10;                                   //SrcX中的整数		
			
            cbufx[1] = fx&0x3FF;     //u			
            cbufx[0] = 1024-cbufx[1];                  //1-u
					
			fx+=scale_x;

			color=sy*32+31-sx;
			dst =    (  data2.mlx90640To[ color    ] * cbufx[0] * cbufy[0] + 
					    data2.mlx90640To[ color+32 ] * cbufx[0] * cbufy[1] + 
					    data2.mlx90640To[ color-1  ] * cbufx[1] * cbufy[0] + 
					    data2.mlx90640To[ color+31 ] * cbufx[1] * cbufy[1] )>>20;   

			dst=( (dst-min)*scale )/10;

			data.DisBuf[adr++]=dst;
			data.DisBuf[adr++]=dst;
        }
		
        for (x = 0; x < DST_MAX_X; x++)
        {
			data.DisBuf[adr++]=data.DisBuf[adr-320]; 
			data.DisBuf[adr++]=data.DisBuf[adr-320];
        }		
    }

	Buf_ShowString(4,0,"e=0.",BUF_BLACK,1);
	Buf_ShowNum(36,0,emissivity*100,BUF_BLACK,1);	                    //辐射系数
//	Buf_SmallFloatNum(140 ,0,center-400,BUF_BLACK,1);	                //中心温度 
	Buf_SmallFloatNum(140 ,0,data2.mlx90640To[368]-400,BUF_BLACK,1);	//中心温度 	
	Buf_ShowString(240, 0,"Ta:",BUF_BLACK,1);
	Buf_SmallFloatNum(264 ,0,Ta*10,BUF_BLACK,1);                        //外壳温度
				
	for (x = 0; x < adr; x++)
	{
		ILI9341_Write_Data1( camColors[ data.DisBuf[x] ] );			
	}		
/******************************************* 结束 ***************************************************************/
	if(Pos_max>31)
	{
		x=Pos_x[Pos_max%32]*2;
		y=Pos_y[Pos_max/32]*2;
		LCD_Fill(x-7,y,x+7,y,MAGENTA);    //x 
		LCD_Fill(x,y-7,x,y+7,MAGENTA);    //y 最大值中心+字标
	}
	if(Pos_min>31)
	{
		x=Pos_x[Pos_min%32]*2;
		y=Pos_y[Pos_min/32]*2;
		LCD_Fill(x-7,y,x+7,y,GREEN);      //x 
		LCD_Fill(x,y-7,x,y+7,GREEN);      //y 最小值中心+字标	
	}	
}

//uint16_t color_add;
//void Disp_test()
//{
//	uint16_t x,y,color;
//	
//	LCD_setwindow(0,0,319,239); 
//	ILI9341_DC_SET;
//	ILI9341_CS_CLR;
//	for(y=0;y<240;y++)
//	{
//		color_add=0;
//		for(x=0;x<320;x++)
//		{			
//			color_add+=10;
//			DATAOUT(color_add);
//			ILI9341_WR_CLR;
//			ILI9341_WR_SET;
//		}
//	}
//	
//	for(x=0;x<320;x++)
//	{
//		data2.databuf[x]=LCD_ReadPoint(x,0);
//	}
//}

//	color_add+=300;
////	LED_IO_HIGH();
//	
//	LCD_setwindow(0,0,319,239); 
//	ILI9341_DC_SET;
//	ILI9341_CS_CLR;
//	for(y=0;y<240;y++)
//		for(x=0;x<320;x++)
//		{			
//			
//			DATAOUT(color_add);
//			ILI9341_WR_CLR;
//			ILI9341_WR_SET;
////			ILI9341_CS_SET;
//		}
//	
////	LED_IO_LOW();
//	delay_1ms(500);	
