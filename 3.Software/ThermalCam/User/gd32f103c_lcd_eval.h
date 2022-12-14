/*!
    \file    gd32f103c_lcd_eval.h
    \brief   LCD driver header file

    \version 2014-12-26, V1.0.0, demo for GD32F10x
    \version 2017-06-20, V2.0.0, demo for GD32F10x
    \version 2018-07-31, V2.1.0, demo for GD32F10x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#ifndef GD32F103C_LCD_EVAL_H
#define GD32F103C_LCD_EVAL_H

#include "bsp.h"

//ēé
#define ILI9341_CS_PORT      GPIOA
#define ILI9341_CS_PIN       GPIO_PIN_3

//DCå¼č
#define ILI9341_DC_PORT      GPIOA
#define ILI9341_DC_PIN       GPIO_PIN_2

//åä½æč½
#define ILI9341_WR_PORT      GPIOC
#define ILI9341_WR_PIN       GPIO_PIN_15

//čÆ»ä½æč½
#define ILI9341_RD_PORT      GPIOC
#define ILI9341_RD_PIN       GPIO_PIN_14

//čåå¼č
#define ILI9341_BK_PORT      GPIOA
#define ILI9341_BK_PIN       GPIO_PIN_8

//ę°ę®äæ”å·ēŗæ
#define ILI9341_DATA_PORT    GPIOB
#define ILI9341_DATA_PIN     GPIO_PIN_ALL

/********äæ”å·ēŗæę§å¶ēøå³ēå®***************/
#define	ILI9341_CS_SET  	 GPIO_BOP(ILI9341_CS_PORT) = (uint32_t)ILI9341_CS_PIN  //ēéē«Æå£
#define	ILI9341_DC_SET		 GPIO_BOP(ILI9341_DC_PORT) = (uint32_t)ILI9341_DC_PIN  //ę°ę®/å½ä»¤
#define	ILI9341_WR_SET		 GPIO_BOP(ILI9341_WR_PORT) = (uint32_t)ILI9341_WR_PIN  //åę°ę®
#define	ILI9341_RD_SET		 GPIO_BOP(ILI9341_RD_PORT) = (uint32_t)ILI9341_RD_PIN  //čÆ»ę°ę®

#define	ILI9341_CS_CLR  	 GPIO_BC(ILI9341_CS_PORT) = (uint32_t)ILI9341_CS_PIN   //ēéē«Æå£
#define	ILI9341_DC_CLR		 GPIO_BC(ILI9341_DC_PORT) = (uint32_t)ILI9341_DC_PIN   //ę°ę®/å½ä»¤
#define	ILI9341_WR_CLR		 GPIO_BC(ILI9341_WR_PORT) = (uint32_t)ILI9341_WR_PIN   //åę°ę®
#define	ILI9341_RD_CLR		 GPIO_BC(ILI9341_RD_PORT) = (uint32_t)ILI9341_RD_PIN   //čÆ»ę°ę®

//ę°ę®ēŗæč¾å„č¾åŗ
#define DATAOUT(x) 	         GPIO_OCTL(ILI9341_DATA_PORT) = (uint32_t)x            //ę°ę®č¾åŗ
#define DATAIN     	         (uint16_t)(GPIO_ISTAT(ILI9341_DATA_PORT))             //ę°ę®č¾å„

#define	ILI9341_BK_ON    	 GPIO_BOP(ILI9341_BK_PORT) = (uint32_t)ILI9341_BK_PIN  //čå
#define	ILI9341_BK_OFF  	 GPIO_BC(ILI9341_BK_PORT) = (uint32_t)ILI9341_BK_PIN   //čå

//åę°ę®å½ę°
//#define ILI9341_Write_Data(data){\
//(*(volatile uint32_t *)(uint32_t)(0x40010810U)) = (uint32_t)GPIO_PIN_2;\
//(*(volatile uint32_t *)(uint32_t)(0x40010814U)) = (uint32_t)GPIO_PIN_3;\
//(*(volatile uint32_t *)(uint32_t)(0x40010C0CU)) = data;\
//(*(volatile uint32_t *)(uint32_t)(0x40011014U)) = (uint32_t)GPIO_PIN_15;\
//(*(volatile uint32_t *)(uint32_t)(0x40011010U)) = (uint32_t)GPIO_PIN_15;\
//(*(volatile uint32_t *)(uint32_t)(0x40010810U)) = (uint32_t)GPIO_PIN_3;\
//}

#define ILI9341_Write_Data(data){\
ILI9341_DC_SET;\
ILI9341_CS_CLR;\
DATAOUT(data);\
ILI9341_WR_CLR;\
ILI9341_WR_SET;\
ILI9341_CS_SET;\
}


#define      BUF_WHITE                     255
#define      BUF_BLACK                     254
#define      BUF_GREEN                     144

#define      WHITE		 		           0xFFFF	   //ē½č²
#define      BLACK                         0x0000	   //é»č²
#define      GREY                          0xF7DE	   //ē°č²
#define      BLUE                          0x001F	   //čč²
#define      BLUE2                         0x051F	   //ęµčč²
#define      RED                           0xF800	   //ēŗ¢č²
#define      MAGENTA                       0xF81F	   //ēŗ¢ē“«č²ļ¼ę“ēŗ¢č²
#define      GREEN                         0x07E0	   //ē»æč²
#define      CYAN                          0x7FFF	   //čē»æč²ļ¼éč²
#define      YELLOW                        0xFFE0	   //é»č²
#define      BRED                          0xF81F
#define      GRED                          0xFFE0
#define      GBLUE                         0x07FF

void ILI9341_Write_Cmd (uint16_t usCmd);
void exmc_lcd_init(void);
void lcd_init(void);
void Lcd_GramScan(void);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_setwindow(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);
void LCD_ShowString(unsigned int x, unsigned int y, const char *s, unsigned int color, unsigned int b_color);

#endif /* GD32F103C_LCD_EVAL_H */
