#include "bsp.h"

UnionData data;
UnionData2 data2;
//uint16_t mlx90640To[768];       //测温范围：-40~300 ，放大10倍：-400~3000，并向上平移400：0~3400 
float Ta;
float emissivity=0.95;  

paramsMLX90640 mlx90640;
FATFS fs;

usbd_core_handle_struct  usb_device_dev = 
{
    .dev_desc = (uint8_t *)&device_descripter,
    .config_desc = (uint8_t *)&configuration_descriptor,
    .strings = usbd_strings,
    .class_init = msc_init,
    .class_deinit = msc_deinit,
    .class_req_handler = msc_req_handler,
    .class_data_handler = msc_data_handler
};

int main(void)  
{  	
	uint16_t statusRegister;
	
	delay_1ms(200);
	mGPIO_Init();
	ADC_DMA_Init();
	SPI_Flash_Init();	
	f_mount(&fs,"0:",1); 	
	
	exmc_lcd_init();                          /* configure the EXMC access mode */
	lcd_init();                               /* initialize the LCD */	
	Lcd_GramScan();	

	MLX90640_I2CInit();	
	MLX90640_SetRefreshRate(MLX90640_ADDR, RefreshRate); 
	MLX90640_SetChessMode(MLX90640_ADDR);	
	MLX90640_DumpEE(MLX90640_ADDR, data.mlx90640_Zoom10);
	MLX90640_ExtractParameters(data.mlx90640_Zoom10, &mlx90640);	
	
	for(statusRegister=0;statusRegister<3;statusRegister++)
	{
		MLX90640_GetFrameData(MLX90640_ADDR, data.mlx90640_Zoom10);
		Ta = MLX90640_GetTa(data.mlx90640_Zoom10, &mlx90640);    //读取MLX90640 外壳温度	
		MLX90640_CalculateTo(data.mlx90640_Zoom10, &mlx90640, emissivity , Ta - TA_SHIFT, data2.mlx90640To);	
		Disp_TempPic();
	}
	ILI9341_BK_ON;
	
    while(1)
	{		
		MLX90640_I2CRead(MLX90640_ADDR, 0x8000, 1, &statusRegister);
		if(statusRegister & 0x0008)
		{
			MLX90640_GetFrameData(MLX90640_ADDR, data.mlx90640_Zoom10);
			Ta = MLX90640_GetTa(data.mlx90640_Zoom10, &mlx90640);    //读取MLX90640 外壳温度	
			MLX90640_CalculateTo(data.mlx90640_Zoom10, &mlx90640, emissivity , Ta - TA_SHIFT, data2.mlx90640To);
	//		MLX90640_BadPixelsCorrection(mlx90640.brokenPixels, mlx90640To, 1, &mlx90640);
	//		MLX90640_BadPixelsCorrection(mlx90640.outlierPixels, mlx90640To, 1, &mlx90640);		
			Disp_TempPic();
		}
		
		//0  736  1932  2883 4073
		if(ADCValue[1]<300)
		{
			ILI9341_BK_OFF;
			break;
		}
		else if(ADCValue[1]<2350&&ADCValue[1]>1750)
		{
			bmp_encode();
			MLX90640_GetFrameData(MLX90640_ADDR, data.mlx90640_Zoom10);
			Ta = MLX90640_GetTa(data.mlx90640_Zoom10, &mlx90640);    //读取MLX90640 外壳温度	
			MLX90640_CalculateTo(data.mlx90640_Zoom10, &mlx90640, emissivity , Ta - TA_SHIFT, data2.mlx90640To);
		}
    }
	
    rcu_config();                            /* system clocks configuration */	 
	usbd_core_init(&usb_device_dev);         /* USB device configuration */
	nvic_config();                           /* NVIC configuration */
	USB_PULLUP_H();                          /* enabled USB pull-up */
	usb_device_dev.status = USBD_CONNECTED;  /* now the usb device is connected */
	while(usb_device_dev.status != USBD_CONFIGURED);

	while(1)
	{
		if(ADCValue[1]<200)
		{
			NVIC_SystemReset();	
		}		
	}		
}

//		if(debug)
//		{
//			debug=0;
//			bmp_encode();
//		}

//	res = f_mkfs("0:", 0, 0);
//	res = f_open(&fil, "hello.txt", FA_CREATE_NEW | FA_WRITE);
//	f_write(&fil, "Hello, World!\r\n", 15, &bw);
	
	
//	res = f_getfree("0", &fre_clust, &fs1);
//	tot_sect=(fs1->n_fatent-2)*fs1->csize;    //得到总扇区数
//	fre_sect=fre_clust*fs1->csize;            //得到空闲扇区数	
	
//	res = f_open(&fil, "hello.txt", FA_OPEN_EXISTING | FA_READ);
//	f_read(&fil,data,10,&bw);
//	
//	f_close(&fil);
	
//	debug=SPI_Flash_ReadID();
//	SPI_Flash_Write(data,0,10);
//	data[0]=0;data[1]=0;data[2]=0;data[3]=0;data[4]=0;data[5]=0;data[6]=0;data[7]=0;
//	SPI_Flash_Read(data,0,10);








static void system_clock_96m_hxtal(void)
{
    /* select PLL as system clock */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_HXTAL;
	
	RCU_CTL &= ~RCU_CTL_PLLEN;  //PLL失能
	
    /* CK_PLL = (CK_HXTAL/2) * 24 = 96 MHz */
    RCU_CFG0 &= ~(RCU_CFG0_PLLMF | RCU_CFG0_PLLMF_4);
    RCU_CFG0 |= RCU_PLL_MUL24;

    /* enable PLL */
    RCU_CTL |= RCU_CTL_PLLEN;

    /* wait until PLL is stable */
    while(0U == (RCU_CTL & RCU_CTL_PLLSTB)){
    }

    /* select PLL as system clock */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_PLL;

    /* wait until PLL is selected as system clock */
    while(0U == (RCU_CFG0 & RCU_SCSS_PLL)){
    }
}

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
	system_clock_96m_hxtal();
	
    /* enable USB pull-up pin clock */ 
    rcu_periph_clock_enable(RCU_GPIOA);
	
    /* enable the power clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* configure USB model clock from PLL clock */
    rcu_usb_clock_config(RCU_CKUSB_CKPLL_DIV2);

    /* enable USB APB1 clock */
    rcu_periph_clock_enable(RCU_USBD);
}

/*!
    \brief      configure interrupt priority
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
    /* 1 bit for pre-emption priority, 3 bits for subpriority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);

    /* enable the USB low priority interrupt */
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);
}

