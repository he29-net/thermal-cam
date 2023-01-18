/*
 Designed for device GD32F103C8
	- 64 kB FLASH (some features may not fit with non-commercial Keil uVision license)
	- 20 kB SRAM (makes it damn hard to fit any extra functionality, since most is used for the few required buffer)
	- 3 general timers, 1 advanced timer, 1 SysTick, 2 watchdogs, 1 RTC, 3 USARTs, 2 I2C, 2 SPI, 1 USBD, 37 GPIO
	- The official "Board Support Package" is pretty old and only supports ARM compiler v5, so upgrade may be a pain.

 Expected memory usage with default configuration:
	- Program Size: Code=24304 RO-data=2736 RW-data=432 ZI-data=20048  
		- SRAM 20480 B = RW-data=432 + ZI-data=20048
		- IRAM1 range in compiler settings seems to be set correctly to size 0x5000 (20k)
	- Main RAM consumers:
		- Stack size: 		2048 B (default 1024 B is not sufficient, observed peak usage was around 1850 B)
		- Sensor output:	1668 B (2*834, raw dump from the sensor, including calibration data etc.)
		- Temperature out:	1536 B (2*768, computed temperature readings from the sensor)
		- Display buffer:	3200 B (used to render a 10 pixel tall strip of thermal image + GUI)
		- BPM data buffer:	1280 B (2*640, used during BPM export)
		- bbb_data			 512 B (used for something when storing data to the integrated flash memory)
		- SPI flash buffer:	4096 B (holds page to be be overwritten when writing to the integrated flash memory?)
		- Others in main():	4764 B (17056 - 12292, everything outside the data and data2 unions)

 Considerations:
	- Code: non-commercial version of linker limits code size to 32k
	- RO-data: are.. what and where exactly?
	- RW-data: are.. what and where exactly?
	- ZI-data: apparently stands for Zero Initialized data; how it differs from RW?

 Todo:
	- BPM saving is currently broken (why? how? Only blue color seems to be saved...)
	- if maximum temperature goes over 99.9 °C, the "C" overlaps with menu_value
	- no way out of USB mode in case USB isn't connected (needs hard reset instead)
	- glitches at high temperatures, glitchy min/max and scaling in general (see Disp_TempNew comment)
*/

#include "bsp.h"
#include "config.h"

// Storage for measurement data. (No idea why are the buffers randomly mixed in two unions, but I'm afraid to touch it.)
// Measurement range is -40 to 300 °C, sensor returns 10 * (x + 40), i.e. 0 to 3400.
// 测温范围：-40~300 ，放大10倍：-400~3000，并向上平移400：0~3400
UnionData data;
UnionData2 data2;

const char *menu_text = "    ";
uint8_t menu_tweak = 0;
const char *menu_value = "    ";

float Ta;
float emissivity = 0.95;

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

#ifdef BMP_SAVE
enum {UI_START, UI_SNAPSHOT = UI_START, UI_FPS, UI_USB, UI_END} ui_mode = UI_SNAPSHOT;
#else
enum {UI_START, UI_FPS = UI_START, UI_USB, UI_END} ui_mode = UI_FPS;
#endif

void update_data() {
	MLX90640_GetFrameData(MLX90640_ADDR, data.mlx90640_Zoom10);
	Ta = MLX90640_GetTa(data.mlx90640_Zoom10, &mlx90640);	// 读取MLX90640 外壳温度 / Get ambient temperature
	MLX90640_CalculateTo(data.mlx90640_Zoom10, &mlx90640, emissivity , Ta - TA_SHIFT, data2.mlx90640To);
}

int main(void) {
	uint16_t statusRegister;
	bool menu_pressed = true;
	bool action_pressed = false;

	uint8_t refresh_rate = DefaultRefreshRate;

	delay_1ms(200);
	mGPIO_Init();
	ADC_DMA_Init();
	SPI_Flash_Init();
	f_mount(&fs, "0:", 1);

	exmc_lcd_init();			// configure the EXMC access mode
	lcd_init();					// initialize the LCD
	Lcd_GramScan();

	MLX90640_I2CInit();
	MLX90640_SetRefreshRate(MLX90640_ADDR, DefaultRefreshRate);
	//MLX90640_SetChessMode(MLX90640_ADDR);							// not needed (default)
	MLX90640_DumpEE(MLX90640_ADDR, data.mlx90640_Zoom10);
	MLX90640_ExtractParameters(data.mlx90640_Zoom10, &mlx90640);

#ifdef CLEANUP
	// get rid of the the first few images (all they contain are variants of the checker pattern)
	// (takes up precious space and does not matter that much → disable for now..)
	for (statusRegister = 0; statusRegister < 4; statusRegister++) update_data();
#endif

#ifdef PROFILE
	systick_config();
#endif

	ILI9341_BK_ON;

#ifdef DISPLAY_TEST
	Disp_test();
	while (1);
#endif

	while (1) {
		// if there are new data available, read them out, compute T out, and draw result to display
		MLX90640_I2CRead(MLX90640_ADDR, 0x8000, 1, &statusRegister);
		if (statusRegister & 0x0008) {
			update_data();
			// Optional filtering of bad pixels or outliers (hacked to work with uint32_t instead of float).
			// (Useless? My sensor reports all pixels are perfect, even though there is one obvious bad outlier area..)
			//MLX90640_BadPixelsCorrection(mlx90640.brokenPixels, data2.mlx90640To, 1, &mlx90640);
			//MLX90640_BadPixelsCorrection(mlx90640.outlierPixels, data2.mlx90640To, 1, &mlx90640);
			//Disp_TempPic();	// slightly faster (85 ms?), but bigger by ~2k
			Disp_TempNew();		// around 90 ms, uses function calls for better readability
		}

		// Update UI text if any button was pressed in the previous frame.
		if (action_pressed || menu_pressed) {
			switch (ui_mode) {
			#ifdef BMP_SAVE
				case UI_SNAPSHOT:	// save a BPM snapshot of the current display buffer
					menu_text = "snap";
					menu_tweak = 0;
					menu_value = "    ";
					break;
			#endif
				case UI_FPS:		// cycle through available frame rates
					menu_text = "rate";
					menu_tweak = 2;
					switch (refresh_rate) {
						case 0:	menu_value = ".5Hz"; break;
						case 1:	menu_value = "1 Hz"; break;
						case 2:	menu_value = "2 Hz"; break;
						case 3:	menu_value = "4 Hz"; break;
						case 4:	menu_value = "8 Hz"; break;
						case 5:	menu_value = "16Hz"; break;
						case 6:	menu_value = "32Hz"; break;
						case 7:	menu_value = "64Hz"; break;
						default: menu_value= "fail"; break;
					}
					break;

				case UI_USB:		// break out of image handling loop and initialize USB
					menu_text = " usb";
					menu_tweak = 2;
					menu_value = "    ";
					break;
			}
		}

		// Buttons seem to be working as a voltage divider. Needs debouncing: wait a while and confirm that the readout
		// is stable (i.e. first sample wasn't just a transition from the other button).
		// First button = cycle menu items:
		if (ADCValue[1] < 300) {
			delay_1ms(100);
			if (ADCValue[1] >= 300 || menu_pressed) continue;
			menu_pressed = true;	// prevent repeated triggering

			ui_mode++;
			if (ui_mode >= UI_END) ui_mode = UI_START;


		}
		// Second button = trigger the selected function
		else if (ADCValue[1] < 2350 && ADCValue[1] > 1750)
		{
			delay_1ms(100);
			if (ADCValue[1] >= 2350 || ADCValue[1] <= 1750 || action_pressed) continue;
			action_pressed = true;

			switch (ui_mode) {
			#ifdef BMP_SAVE
				case UI_SNAPSHOT:	// save a BPM snapshot of the current display buffer
					bmp_encode();
				#ifdef CLEANUP
					update_data();
				#endif
					break;
			#endif
				case UI_FPS:		// cycle through available frame rates
					refresh_rate++;
					if (refresh_rate > MaximumRefreshRate) refresh_rate = MinimumRefreshRate;
					MLX90640_SetRefreshRate(MLX90640_ADDR, refresh_rate);
					break;

				case UI_USB:		// break out of image handling loop and initialize USB
					ILI9341_BK_OFF;
					goto usb_init;
			}
		} else {
			// nothing is pressed, clear debounce / multi-action check
			action_pressed = false;
			menu_pressed = false;
		}
	}

	usb_init:
	rcu_config();							 /* system clocks configuration */
	usbd_core_init(&usb_device_dev);		 /* USB device configuration */
	nvic_config();							 /* NVIC configuration */
	USB_PULLUP_H();							 /* enabled USB pull-up */
	usb_device_dev.status = USBD_CONNECTED;  /* now the usb device is connected */
	while (usb_device_dev.status != USBD_CONFIGURED);

	while (1)
	{
		if (ADCValue[1] < 200)	// wait for first button again (menu button in this case?)
		{
			NVIC_SystemReset();	// resets the whole CPU so that it ends up in the image loop again.. uh, kinda nasty..
		}
	}
}

//	if(debug)
//	{
//		debug=0;
//		bmp_encode();
//	}

//	res = f_mkfs("0:", 0, 0);
//	res = f_open(&fil, "hello.txt", FA_CREATE_NEW | FA_WRITE);
//	f_write(&fil, "Hello, World!\r\n", 15, &bw);


//	res = f_getfree("0", &fre_clust, &fs1);
//	tot_sect=(fs1->n_fatent-2)*fs1->csize;	  //得到总扇区数
//	fre_sect=fre_clust*fs1->csize;			  //得到空闲扇区数

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

	RCU_CTL &= ~RCU_CTL_PLLEN;	//PLL失能

	/* CK_PLL = (CK_HXTAL/2) * 24 = 96 MHz */
	RCU_CFG0 &= ~(RCU_CFG0_PLLMF | RCU_CFG0_PLLMF_4);
	RCU_CFG0 |= RCU_PLL_MUL24;

	/* enable PLL */
	RCU_CTL |= RCU_CTL_PLLEN;

	/* wait until PLL is stable */
	while(0U == (RCU_CTL & RCU_CTL_PLLSTB)){}

	/* select PLL as system clock */
	RCU_CFG0 &= ~RCU_CFG0_SCS;
	RCU_CFG0 |= RCU_CKSYSSRC_PLL;

	/* wait until PLL is selected as system clock */
	while(0U == (RCU_CFG0 & RCU_SCSS_PLL)){}
}

// configure the different system clocks
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

// configure interrupt priority
void nvic_config(void)
{
	/* 1 bit for pre-emption priority, 3 bits for subpriority */
	nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);

	/* enable the USB low priority interrupt */
	nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);
}
