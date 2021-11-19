#include "bmp.h"
#include "string.h"

//uint16_t databuf[640];				//���ݻ�������ַ
//BMP���뺯��
//����ǰLCD��Ļ��ָ�������ͼ,��Ϊ16λ��ʽ��BMP�ļ� RGB565��ʽ.
//����Ϊrgb565����Ҫ����,��Ҫ����ԭ���ĵ�ɫ��λ����������.���������Ѿ�����������.
//����Ϊrgb555��ʽ����Ҫ��ɫת��,��ʱ��ȽϾ�,���Ա���Ϊ565������ٵİ취.
//filename:���·��
//x,y:����Ļ�ϵ���ʼ����  
//mode:ģʽ.0,�����������ļ��ķ�ʽ����;1,���֮ǰ�����ļ�,�򸲸�֮ǰ���ļ�.���û��,�򴴽��µ��ļ�.
//����ֵ:0,�ɹ�;����,������.  
uint8_t bmp_encode(void)
{				
	FIL f_bmp;
	uint16_t bmpheadsize;			//bmpͷ��С	   	
 	BITMAPINFO hbmp;			//bmpͷ	 
	uint8_t res=0;
	int16_t tx,ty;				   	//ͼ��ߴ�   
 	char fname[7]="00.bmp"; 
	char ShowInfo[22]="Save Picture...";
	UINT bw;
//	uint16_t r=0,g=0,b=0;
      
	bmpheadsize=sizeof(hbmp);//�õ�bmp�ļ�ͷ�Ĵ�С      
	hbmp.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);//��Ϣͷ��С
	hbmp.bmiHeader.biWidth=320;	 	                //bmp�Ŀ��
	hbmp.bmiHeader.biHeight=240; 	                //bmp�ĸ߶�
	hbmp.bmiHeader.biPlanes=1;	 		            //��Ϊ1
	hbmp.bmiHeader.biBitCount=16;	 	            //bmpΪ16λɫbmp
	hbmp.bmiHeader.biCompression=BI_BITFIELDS;      //ÿ�����صı�����ָ�������������
 	hbmp.bmiHeader.biSizeImage=hbmp.bmiHeader.biHeight*hbmp.bmiHeader.biWidth*hbmp.bmiHeader.biBitCount/8;//bmp��������С
 				   
	hbmp.bmfHeader.bfType=((uint16_t)'M'<<8)+'B';   //BM��ʽ��־
	hbmp.bmfHeader.bfSize=bmpheadsize+hbmp.bmiHeader.biSizeImage;//����bmp�Ĵ�С
   	hbmp.bmfHeader.bfOffBits=bmpheadsize;           //����������ƫ��

	hbmp.RGB_MASK[0]=0X00F800;	 		//��ɫ����
	hbmp.RGB_MASK[1]=0X0007E0;	 		//��ɫ����
	hbmp.RGB_MASK[2]=0X00001F;	 		//��ɫ����

	while(1)
	{
		res=f_open(&f_bmp,(const TCHAR*)fname,FA_READ|FA_WRITE);//���Դ�֮ǰ���ļ�
		if(res==FR_OK)
		{
			f_close(&f_bmp);
			res=(fname[0]-'0')*10+(fname[1]-'0')+1;
			fname[0]=res/10+'0';
			fname[1]=res%10+'0';		
		}
		else
		{
			res=f_open(&f_bmp,(const TCHAR*)fname,FA_WRITE|FA_CREATE_NEW);
			if(res==FR_OK)
				break;
			else
				return 1;
		}
	}
 
	res=f_write(&f_bmp,(uint8_t*)&hbmp,bmpheadsize,&bw);//д��BMP�ײ�  

	LCD_setwindow(0,0,319,239); 
	
	bmpheadsize=0;
	for(ty=239;ty>=224;ty--)
	{
		for(tx=0;tx<320;tx++)
		{
			data2.databuf[bmpheadsize+tx]=LCD_ReadPoint(tx,ty);//��ȡ������ֵ   0x0460; //
		}

		if( (ty%2)==0 )
		{
			res=f_write(&f_bmp,(uint8_t*)data2.databuf,1280,&bw);//д������
			bmpheadsize=0;
		}
		else
			bmpheadsize=320;
	}
	
	strcat(ShowInfo,fname);
	LCD_ShowString(0,224,ShowInfo,WHITE,BLACK);
	LCD_setwindow(0,0,319,239); 
	
	bmpheadsize=0;
	for(ty=223;ty>=0;ty--)
	{
		for(tx=0;tx<320;tx++)
		{
			data2.databuf[bmpheadsize+tx]=LCD_ReadPoint(tx,ty);//��ȡ������ֵ   0x0460; //
		}

		if( (ty%2)==0 )
		{
			res=f_write(&f_bmp,(uint8_t*)data2.databuf,1280,&bw);//д������
			bmpheadsize=0;
		}
		else
			bmpheadsize=320;
	}
	
//	for(ty=239;ty>=0;ty--)
//	{
//		LCD_SetCursor(0,ty);	    
//		ILI9341_Write_Cmd(0x2E);//9341/6804/3510 ���Ͷ�GRAMָ��
//		ILI9341_DC_SET;
//		ILI9341_WR_SET;
//		ILI9341_CS_CLR;
//		
//		GPIO_CTL0(GPIOB) = 0x88888888; //PB0-7  �������� 
//		GPIO_CTL1(GPIOB) = 0x88888888; //PB8-15 ��������
//		GPIO_OCTL(ILI9341_DATA_PORT) = 0X0000;     //ȫ�����0
		
//		for(tx=0;tx<320;tx++)
//		{					
//			ILI9341_RD_CLR;       
//			r=DATAIN;  
//			ILI9341_RD_SET;
//			ILI9341_RD_SET;
//			
//			ILI9341_RD_CLR;       
//			r=DATAIN;  
//			ILI9341_RD_SET;
//			ILI9341_RD_SET;
//			
//			ILI9341_RD_CLR;       
//			b=DATAIN;  
//			ILI9341_RD_SET;
//			
//			g=r&0XFF;		       
//			g<<=8;
//			
//			data2.databuf[tx]=(((r>>11)<<11)|((g>>10)<<5)|(b>>11));
//		}
//		GPIO_CTL0(GPIOB) = 0X33333333; //PB0-7  �������
//		GPIO_CTL1(GPIOB) = 0X33333333; //PB8-15 �������
//		GPIO_OCTL(ILI9341_DATA_PORT) = 0XFFFF;    //ȫ�������	

//		res=f_write(&f_bmp,(uint8_t*)data2.databuf,640,&bw);//д������
//	}
	f_close(&f_bmp);
    
	return res;
}



//	for(ty=239;ty>=0;ty--)
//	{
//		for(tx=0;tx<320;tx++)
//		{
//			data2.databuf[tx]=LCD_ReadPoint(tx,ty);//��ȡ������ֵ   0x0460; //
//		}

//		res=f_write(&f_bmp,(uint8_t*)data2.databuf,640,&bw);//д������
//	}

