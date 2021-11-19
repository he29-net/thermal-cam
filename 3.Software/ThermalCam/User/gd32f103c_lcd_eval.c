/*!
    \file    gd32f103c_lcd_eval.c
    \brief   LCD driver functions

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

#include "gd32f103c_lcd_eval.h"
#include "16x8.h"

void exmc_lcd_init(void)
{
	ILI9341_BK_OFF;
	
	//ʱ��ʹ��
    rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	rcu_periph_clock_enable(RCU_GPIOE);
	
	//��ʼ��
    gpio_init(ILI9341_CS_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ILI9341_CS_PIN);  
	gpio_init(ILI9341_DC_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ILI9341_DC_PIN); 
	gpio_init(ILI9341_WR_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ILI9341_WR_PIN);
	gpio_init(ILI9341_RD_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ILI9341_RD_PIN);
	gpio_init(ILI9341_BK_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ILI9341_BK_PIN);
	gpio_init(ILI9341_DATA_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ILI9341_DATA_PIN);	
}

void ILI9341_Write_Cmd ( uint16_t usCmd )
{
	ILI9341_CS_CLR;//��ʼƬѡ      
	ILI9341_DC_CLR;//д����
	ILI9341_RD_SET;//��ֹ��
	DATAOUT(usCmd);//�������
	ILI9341_WR_CLR;//д�뿪ʼ
	ILI9341_WR_SET;//д�����
	ILI9341_CS_SET;//����Ƭѡ        	
}

/**
  * @brief  ��ILI9341��ȡ����
  * @param  ��
  * @retval ��ȡ��������
  */	
uint16_t ILI9341_Read_Data ( void )
{
	uint16_t data;
 	GPIO_CTL0(GPIOB) = 0x88888888; //PB0-7  �������� 
	GPIO_CTL1(GPIOB) = 0x88888888; //PB8-15 ��������
	GPIO_OCTL(ILI9341_DATA_PORT) = 0X0000;     //ȫ�����0

	ILI9341_DC_SET;
	ILI9341_WR_SET;

	ILI9341_CS_CLR;
	//��ȡ����
	ILI9341_RD_CLR;    
    
	data=DATAIN;  
	ILI9341_RD_SET;
	ILI9341_CS_SET; 

	GPIO_CTL0(GPIOB) = 0X33333333; //PB0-7  �������
	GPIO_CTL1(GPIOB) = 0X33333333; //PB8-15 �������
	GPIO_OCTL(ILI9341_DATA_PORT) = 0XFFFF;    //ȫ�������	
	return data;  
}

void lcd_init(void)
{
	/*  Power control B (CFh)  ���Ŀ���B */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xCF);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x81);
	ILI9341_Write_Data(0x30);
	
	/*  Power on sequence control (EDh) ��Դ���п��� */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xED);
	ILI9341_Write_Data(0x64);
	ILI9341_Write_Data(0x03);
	ILI9341_Write_Data(0x12);
	ILI9341_Write_Data(0x81);
	
	/*  Driver timing control A (E8h) ����ʱ�����A */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xE8);
	ILI9341_Write_Data(0x85);
	ILI9341_Write_Data(0x10);
	ILI9341_Write_Data(0x78);
	
	/*  Power control A (CBh) ���Ŀ���A */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xCB);
	ILI9341_Write_Data(0x39);
	ILI9341_Write_Data(0x2C);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x34);
	ILI9341_Write_Data(0x02);
	
	/* Pump ratio control (F7h) �ñȿ��� */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xF7);
	ILI9341_Write_Data(0x20);
	
	/* Driver timing control B ����ʱ�����B */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xEA);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x00);
	
	/* Frame Rate Control (In Normal Mode/Full Colors) (B1h) ֡�ʿ��� */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xB1);
	ILI9341_Write_Data(0x00);  //�ڲ�ʱ�Ӳ���Ƶ
	ILI9341_Write_Data(0x1B);  //֡��Ϊ70Hz
	
	/*  Display Function Control (B6h) */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xB6);
	ILI9341_Write_Data(0x0A);
	ILI9341_Write_Data(0xA2);
	
	/* Power Control 1 (C0h) */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xC0);
	ILI9341_Write_Data(0x35);
	
	/* Power Control 2 (C1h) */
	delay_1ms(5);
	ILI9341_Write_Cmd(0xC1);
	ILI9341_Write_Data(0x11);
	
	/* VCOM Control 1(C5h) */
	ILI9341_Write_Cmd(0xC5);
	ILI9341_Write_Data(0x45);
	ILI9341_Write_Data(0x45);
	
	/*  VCOM Control 2(C7h)  */
	ILI9341_Write_Cmd(0xC7);
	ILI9341_Write_Data(0xA2);
	
	/* Enable 3G (F2h) */
	ILI9341_Write_Cmd(0xF2);
	ILI9341_Write_Data(0x00);
	
	/* Gamma Set (26h) */
	ILI9341_Write_Cmd(0x26);
	ILI9341_Write_Data(0x01);
	delay_1ms(5);
	
	/* Positive Gamma Correction */
	ILI9341_Write_Cmd(0xE0); //Set Gamma
	ILI9341_Write_Data(0x0F);
	ILI9341_Write_Data(0x26);
	ILI9341_Write_Data(0x24);
	ILI9341_Write_Data(0x0B);
	ILI9341_Write_Data(0x0E);
	ILI9341_Write_Data(0x09);
	ILI9341_Write_Data(0x54);
	ILI9341_Write_Data(0xA8);
	ILI9341_Write_Data(0x46);
	ILI9341_Write_Data(0x0C);
	ILI9341_Write_Data(0x17);
	ILI9341_Write_Data(0x09);
	ILI9341_Write_Data(0x0F);
	ILI9341_Write_Data(0x07);
	ILI9341_Write_Data(0x00);
	
	/* Negative Gamma Correction (E1h) */
	ILI9341_Write_Cmd(0XE1); //Set Gamma
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x19);
	ILI9341_Write_Data(0x1B);
	ILI9341_Write_Data(0x04);
	ILI9341_Write_Data(0x10);
	ILI9341_Write_Data(0x07);
	ILI9341_Write_Data(0x2A);
	ILI9341_Write_Data(0x47);
	ILI9341_Write_Data(0x39);
	ILI9341_Write_Data(0x03);
	ILI9341_Write_Data(0x06);
	ILI9341_Write_Data(0x06);
	ILI9341_Write_Data(0x30);
	ILI9341_Write_Data(0x38);
	ILI9341_Write_Data(0x0F);
	
//	/* memory access control set �洢�����ʿ��� */
//	delay_1ms(5);
//	ILI9341_Write_Cmd(0x36); 	
//	ILI9341_Write_Data(0x68);    
//	delay_1ms(5);
	
//	/* column address control set �е�ַ���� */
//	ILI9341_Write_Cmd(0X2A); 
//	ILI9341_Write_Data(0x00);
//	ILI9341_Write_Data(0x00);  //�п�ʼ��ַ  0
//	ILI9341_Write_Data(0x01);
//	ILI9341_Write_Data(0x3F);  //�н�����ַ  319
//	
//	/* page address control set �е�ַ���� */
//	delay_1ms(5);
//	ILI9341_Write_Cmd(0X2B); 
//	ILI9341_Write_Data(0x00);
//	ILI9341_Write_Data(0x00);  //�п�ʼ��ַ  0
//	ILI9341_Write_Data(0x00);
//	ILI9341_Write_Data(0xEF);  //�н�����ַ  239
	
	/*  Pixel Format Set (3Ah)  */
	delay_1ms(5);
	ILI9341_Write_Cmd(0x3a); 
	ILI9341_Write_Data(0x55);
	
	/* Sleep Out (11h)  �˳�˯��ģʽ */
	ILI9341_Write_Cmd(0x11);	
	delay_1ms(5);
	delay_1ms(5);
	
	/* Display ON (29h) ����ʾ */
	ILI9341_Write_Cmd(0x29); 
	
//	/* write gram start */
//	ILI9341_Write_Cmd(0x2C);	
}

/* ����Һ��GRAM��ɨ�跽�� 
 * �����óɲ�ͬ��ɨ��ģʽʱ, page(��x) �� column(��y) ��ֵ�ǻ�ı��
 */
void Lcd_GramScan(void)
{	
	ILI9341_Write_Cmd(0x36); 
	ILI9341_Write_Data(0x68);	//68
	
	ILI9341_Write_Cmd(0X2A); 
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x01);
	ILI9341_Write_Data(0x3F);	

	ILI9341_Write_Cmd(0X2B); 
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0xEF);			
	
	/* write gram start */
	ILI9341_Write_Cmd(0x2C);
}

//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	 
	ILI9341_Write_Cmd(0X2A); 
	ILI9341_Write_Data(Xpos>>8); 
	ILI9341_Write_Data(Xpos&0XFF);	 
	ILI9341_Write_Cmd(0X2B); 
	ILI9341_Write_Data(Ypos>>8); 
	ILI9341_Write_Data(Ypos&0XFF);
}
//��ȡ��ĳ�����ɫֵ	 
//x,y:����
//����ֵ:�˵����ɫ
//uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
//{
// 	uint16_t r=0,g=0,b=0;
//	   
//	LCD_SetCursor(x,y);	    
//	ILI9341_Write_Cmd(0X2E);//9341/6804/3510 ���Ͷ�GRAMָ��
// 
// 	ILI9341_Read_Data();									//dummy Read	   
//	Delay_50ns(2);	  
// 	r=ILI9341_Read_Data();  		  						//ʵ��������ɫ

//	Delay_50ns(2);	  
//	b=ILI9341_Read_Data(); 
//	g=r&0XFF;		       //����9341/5310/5510,��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
//	g<<=8;
//	
//	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510��Ҫ��ʽת��һ��
//}	

uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;
	   
	LCD_SetCursor(x,y);	    
	ILI9341_Write_Cmd(0X2E);
 
 	GPIO_CTL0(GPIOB) = 0x88888888; //PB0-7  �������� 
	GPIO_CTL1(GPIOB) = 0x88888888; //PB8-15 ��������
	GPIO_OCTL(ILI9341_DATA_PORT) = 0X0000;     //ȫ�����0

	ILI9341_DC_SET;
	ILI9341_WR_SET;
	ILI9341_CS_CLR;
	ILI9341_RD_CLR;   
	r=DATAIN; 
	ILI9341_RD_SET;

	ILI9341_RD_CLR;  
	r=DATAIN; 
	ILI9341_RD_SET;

	ILI9341_RD_CLR;   
	b=DATAIN;  
	ILI9341_RD_SET;
	ILI9341_CS_SET; 

	GPIO_CTL0(GPIOB) = 0X33333333; //PB0-7  �������
	GPIO_CTL1(GPIOB) = 0X33333333; //PB8-15 �������
	GPIO_OCTL(ILI9341_DATA_PORT) = 0XFFFF;    //ȫ�������	
	
	g=r&0XFF;		       //����9341/5310/5510,��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
	g<<=8;
	
	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510��Ҫ��ʽת��һ��
}	
//================================================================================================
//	ʵ�ֹ��ܣ�	���ô���
//	���������	x0,y0,x1,y1    x�Ὺʼ��y�Ὺʼ��x�������y�����
//================================================================================================
void LCD_setwindow(unsigned int x0,unsigned int y0,unsigned int x1,unsigned int y1)
{
	ILI9341_Write_Cmd(0x2a);   
	ILI9341_Write_Data(x0>>8);
	ILI9341_Write_Data(x0&0xff);
	ILI9341_Write_Data(x1>>8);
	ILI9341_Write_Data(x1&0xff);

	ILI9341_Write_Cmd(0x2b);   
	ILI9341_Write_Data(y0>>8);
	ILI9341_Write_Data(y0&0xff);
	ILI9341_Write_Data(y1>>8);
	ILI9341_Write_Data(y1&0xff);

	ILI9341_Write_Cmd(0x2C);		
}

//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{   
    unsigned int i,j; 

 	LCD_setwindow(sx,sy,ex,ey);

	for(i=sy;i<=ey;i++)
	{
	    for(j=sx;j<=ex;j++) 
		{  
			ILI9341_Write_Data(color);				
		}	
	}
}

void LCD_Showchar(unsigned int x, unsigned int y,unsigned char c, unsigned int color,unsigned int b_color)
{  
	unsigned int  s_x ,s_y, temp ;
  	unsigned int j;
  	c -= 32;

	LCD_setwindow(x,y,x+7,y+15);
	for( s_y=0 ; s_y<16 ; s_y++ )
	{
	   j=c;
	   j=j*16+s_y;
	   temp=font16x8[j];
	   for( s_x=0 ; s_x < 8 ; s_x++ )
	   {
			if( (temp&(0x80>>(s_x))) == (0x80>>(s_x)) )
			{
				ILI9341_Write_Data(color);
			}
			else
			{
				ILI9341_Write_Data(b_color);	
			}				
	   }		
	}
}

void LCD_ShowString(unsigned int x, unsigned int y,const char *s, unsigned int color,unsigned int b_color)
{  
	while(*s!='\0')
	{
		LCD_Showchar(x, y,*s, color,b_color);
		x=x+8;
		s++;
	}
}	

/**********************************************************************************************************
*	�� �� ����Buf_Showchar
*	����˵����
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/
void Buf_Showchar(unsigned int x, unsigned int y,unsigned char c, unsigned int color,uint8_t UpOrDn)
{  
	unsigned int  s_x ,s_y, temp;
	unsigned int j;
	c -= 32;

	if(UpOrDn==0)  //�ϰ벿��
	{
		for( s_y=0 ; s_y<8 ; s_y++ )
		{
			j=c;
			j=j*16+s_y;
			temp=font16x8[j];
			for( s_x=0 ; s_x < 8 ; s_x++ )
			{
				if(  ( temp&(0x80>>s_x) ) == (0x80>>s_x)  )
					data.DisBuf[(s_y+y)*320+x+s_x]=color;			
			}		
		}	
	}
	else   //�°벿��
	{
		for( s_y=8 ; s_y<16 ; s_y++ )
		{
			j=c;
			j=j*16+s_y;
			temp=font16x8[j];
			for( s_x=0 ; s_x < 8 ; s_x++ )
			{
				if(  ( temp&(0x80>>s_x) ) == (0x80>>s_x)  )
					data.DisBuf[(s_y-8+y)*320+x+s_x]=color;			
			}		
		}	
	}
}

/**********************************************************************************************************
*	�� �� ����Buf_ShowString
*	����˵����
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/
void Buf_ShowString(unsigned int x, unsigned int y,const char *s, unsigned int color,uint8_t UpOrDn)
{  
	while(*s!='\0')
	{
		Buf_Showchar(x, y,*s, color,UpOrDn);
		x=x+8;
		s++;
	}
}	

/**********************************************************************************************************
*	�� �� ����Buf_ShowNum
*	����˵������ʾ���֣���Χ��-999~999
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/
void Buf_ShowNum(unsigned int x, unsigned int y,int num, unsigned int color,uint8_t UpOrDn)
{
	if(num<0)
	{
		num = -num;
		Buf_Showchar(x,y,'-',color,UpOrDn);
		x+=8;
	}
	
	if(num>=100)
	{
		Buf_Showchar(x, y,num/100+'0', color,UpOrDn);
		Buf_Showchar(x+8, y,(num/10)%10+'0', color,UpOrDn);
		Buf_Showchar(x+16, y,num%10+'0', color,UpOrDn);
	}
	else if(num>=10)
	{
		Buf_Showchar(x, y,num/10+'0', color,UpOrDn);
		Buf_Showchar(x+8, y,num%10+'0', color,UpOrDn);
	}
	else
		Buf_Showchar(x, y,num+'0', color,UpOrDn);
}

/**********************************************************************************************************
*	�� �� ����Buf_SmallFloatNum
*	����˵������ʾ�¶�ֵ���¶ȷ�Χ-40~300���¶�ֵ�Ŵ���10����-400~3000
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/
void Buf_SmallFloatNum(int x,int y,int16_t num,unsigned int color,uint8_t UpOrDn)
{
	if(num<0)
	{
		num = -num;
		Buf_Showchar(x,y,'-',color,UpOrDn);
		x+=8;
	}

	if(num>=1000)
	{
		Buf_Showchar(x, y,num/1000+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,(num/100)%10+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,(num%100)/10+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,'.', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,num%10+'0', color,UpOrDn);
	}
    else if(num>=100)
	{
		Buf_Showchar(x, y,num/100+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,(num/10)%10+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,'.', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,num%10+'0', color,UpOrDn);
	}
	else if(num>=10)
	{
		Buf_Showchar(x, y,num/10+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,'.', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,num%10+'0', color,UpOrDn);
	}
	else
	{
		Buf_Showchar(x, y,0+'0', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,'.', color,UpOrDn);
		x+=8;
		Buf_Showchar(x, y,num+'0', color,UpOrDn);		
	}

	x+=8;
	Buf_ShowString(x, y,"'C", color,UpOrDn);
}

void Buf_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint8_t color)
{   
    unsigned int i,j; 

	for(i=sy;i<=ey;i++)
	{
	    for(j=sx;j<=ex;j++) 
		{  
			data.DisBuf[i*320+j]=color;				
		}	
	}
}

/**********************************************************************************************************
*	�� �� ����Disp_BatPower
*	����˵������ʾ����
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/
void Disp_BatPower(void)
{
	uint16_t i,tmp;
	
	Buf_Fill(294,0,294,9,BUF_BLACK);
	Buf_Fill(312,0,312,9,BUF_BLACK);
	Buf_Fill(294,0,312,0,BUF_BLACK);
	Buf_Fill(294,9,312,9,BUF_BLACK);
	Buf_Fill(291,3,291,6,BUF_BLACK);
	Buf_Fill(292,3,292,6,BUF_BLACK);
	Buf_Fill(293,3,293,6,BUF_BLACK);
	
	if(CRG_STA_READ())  //���ݵ�������ʾ���� 	
	{
		Buf_Fill(295,1,311,8,BUF_WHITE);  //296-310  15
		
		tmp=((ADCValue[0]*660)>>12);
		if(tmp<539) tmp=539;
		else if(tmp>642) tmp=642;
		tmp=(tmp-539)*15/103;
		
		for(i=0;i<tmp;i++)
		{
			Buf_Fill(310-i,2,310-i,7,BUF_BLACK);
		}	
	}	
	else            //���ڳ��
	{
		Buf_Fill(295,1,311,8,BUF_GREEN);
	}
}
