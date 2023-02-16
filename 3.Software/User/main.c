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
	- Main RAM consumers (not up to date):
		- Stack size: 		2048 B (default 1024 B is not sufficient, observed peak usage was around 1850 B)
		- Sensor output:	1668 B (2*834, raw dump from the sensor, including calibration data etc.)
		- Temperature out:	1536 B (2*768, computed temperature readings from the sensor)
		- Display buffer:	3200 B (used to render a 10 pixel tall strip of thermal image + GUI)
		- BPM data buffer:	1280 B (2*640, used during BPM export)
		- bbb_data			 512 B (used for something when storing data to the integrated flash memory)
		- SPI flash buffer:	4096 B (holds page to be be overwritten when writing to the integrated flash memory?)
		- Others in main():	4764 B (17056 - 12292, everything outside the data and data2 unions)

 Considerations:
	- Code: instructions stored in flash; non-commercial version of linker limits code size to 32k
	- RO-data: are constant data stored in the program memory (and thus contribute to total Code size)
	- RW-data: are data stored in SRAM, but initialized from program memory
	- ZI-data: are data stored in SRAM, but not initialized (Zero Initialized), so they do not take any Code space

 Todo:
	- BMP saving is currently broken (why? how? Only blue color seems to be saved...)
	- no way out of USB mode in case USB isn't connected (needs hard reset instead)
*/

#include "bsp.h"
#include "config.h"

// Load color maps in RGB565 format. 0..253 = temperature → color map, 254 = GUI FG, 255 = GUI BG
// 色条颜色，共256色
#include "scales/blackbody.c"
#include "scales/contrast.c"
#include "scales/rainbow.c"
#include "scales/skyline.c"

// Storage for measurement data. (No idea why are the buffers randomly mixed in two unions, but I'm afraid to touch it.)
// Measurement range is -40 to 300 °C, sensor returns 10 * (x + 40), i.e. 0 to 3400.
// 测温范围：-40~300 ，放大10倍：-400~3000，并向上平移400：0~3400
UnionData data;
UnionData2 data2;

float Ta;

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

// Define available UI modes
#ifdef BMP_SAVE
enum {UI_START, UI_SNAPSHOT = UI_START, UI_FPS, UI_SCALE, UI_RANGE, UI_EMIS, UI_USB, UI_END} ui_mode = UI_SCALE;
#else
enum {UI_START, UI_FPS = UI_START, UI_SCALE, UI_RANGE, UI_EMIS, UI_USB, UI_END, UI_SNAPSHOT} ui_mode = UI_SCALE;
#endif

// Define the cycle of preset values selectable in each mode, and their default values.
enum {CM_START, CM_SKYLINE = UI_START, CM_BLACKBODY, CM_CONTRAST, CM_RAINBOW, CM_END} colormap = CM_SKYLINE;
uint16_t *camColors = (uint16_t*)&(skyline.pixel_data[0]);	// color mapping lookup table

enum {RAN_START, RAN_AUTO = RAN_START, RAN_ROOM, RAN_WATER, RAN_MAX, RAN_COLD, RAN_BOIL, RAN_END} range = RAN_AUTO;
int16_t range_low = -40;									// low and high limit for a fixed range
int16_t range_high = -40;									// (anything under -40 means automatic range)

enum {EM_START, EM_WATER = EM_START, EM_SKIN, EM_MAX, EM_METAL, EM_ALOX, EM_40, EM_60, EM_SNOW, EM_PTFE, EM_PAINT,
	EM_LIME, EM_PLANT, EM_END} emindex = EM_WATER;
float emissivity = 0.96;

// Menu values for use in display drawing funciton
const char *menu_text = "    ";
uint8_t menu_tweak = 0;
const char *menu_value = "    ";

uint8_t refresh_rate = DefaultRefreshRate;
uint16_t statusRegister = 0;

// Get temperature readings from sensor
void update_data() {
	MLX90640_GetFrameData(MLX90640_ADDR, data.mlx90640_Zoom10); // takes around 11 ms
	Ta = MLX90640_GetTa(data.mlx90640_Zoom10, &mlx90640);		// 读取MLX90640 外壳温度 / Get ambient temperature
#ifdef DRAWING_TEST
	for (uint16_t i = 0; i < 768 / 2; i++) data2.mlx90640To[i] = i * 10;
	for (uint16_t i = 768 / 2; i < 768; i++) data2.mlx90640To[i] = 7680 - i * 10;
#else
	// Calculate temperature output based on raw measured data (ultra slow: 1.6M cycles / 137 ms).
	// MLX90640_CalculateTo(data.mlx90640_Zoom10, &mlx90640, emissivity , Ta - TA_SHIFT, data2.mlx90640To);
	// Do the same, but using optimized code (mainly pow() and sqrt() operations; about 26 ms).
	MLX90640_CalculateToOpt(data.mlx90640_Zoom10, &mlx90640, emissivity , Ta - TA_SHIFT, data2.mlx90640To);
#endif
}

// Get rid of the the first few images after initialization or configuration change
// (all they contain are variants of the checker pattern)
void cleanup() {
#ifdef CLEANUP
	for (int i = 0; i < 3; i++) {
		while (!(statusRegister & 0x0008)) {
			MLX90640_I2CRead(MLX90640_ADDR, 0x8000, 1, &statusRegister);
		}
		MLX90640_GetFrameData(MLX90640_ADDR, data.mlx90640_Zoom10);
	}
#endif
}

#ifdef PROFILE
int16_t sensor_diff = 0;
#endif

int main(void) {
	bool menu_pressed = true;
	bool action_pressed = false;

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
	cleanup();
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
		// If there are new data available, read them out, compute T out, and draw result to display
		MLX90640_I2CRead(MLX90640_ADDR, 0x8000, 1, &statusRegister);	// ~1700 cycles / 0.14 ms
		if (statusRegister & 0x0008) {
		#ifdef PROFILE
			SysTick->VAL = 0;
		#endif
			update_data();		// about 37 ms
		#ifdef PROFILE
			sensor_diff = (0xFFFFFF - SysTick->VAL) / 11888;
		#endif
			// Optional filtering of bad pixels or outliers (hacked to work with uint32_t instead of float).
			// (Useless? My sensor reports all pixels are perfect, even though there is one obvious bad outlier area..)
			//MLX90640_BadPixelsCorrection(mlx90640.brokenPixels, data2.mlx90640To, 1, &mlx90640);
			//MLX90640_BadPixelsCorrection(mlx90640.outlierPixels, data2.mlx90640To, 1, &mlx90640);
			display_image(); 	// around 25 ms
		}

		// Update UI text if any button was pressed in the previous frame. (All UI ~10 cycles when nothing is pressed.)
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

				case UI_SCALE:		// cycle through available color maps
					menu_text = " map";
					menu_tweak = 0;
					switch (colormap) {
						case CM_BLACKBODY:	menu_value = "fire"; break;
						case CM_RAINBOW:	menu_value = "rbow"; break;
						case CM_CONTRAST:	menu_value = "trip"; break;
						case CM_SKYLINE:	menu_value = "glow"; break;
						default: menu_value= "fail"; break;
					}
					break;

				case UI_RANGE:		// cycle through available temperature ranges
					menu_text = "span";
					menu_tweak = 0;
					switch (range) {
						case RAN_AUTO:	menu_value = "auto"; break;
						case RAN_COLD:	menu_value = "cold"; break;
						case RAN_ROOM:	menu_value = "room"; break;
						case RAN_WATER:	menu_value = " H2O"; break;
						case RAN_BOIL:	menu_value = "boil"; break;
						case RAN_MAX:	menu_value = " max"; break;
						default: menu_value= "fail"; break;
					}
					break;

				case UI_EMIS:		// cycle through available emissivities
					menu_text = "emis";
					menu_tweak = 2;
					switch (emindex) {
						// common values form https://en.wikipedia.org/wiki/Emissivity#Emissivities_of_common_surfaces
						case EM_METAL:	menu_value = "metl"; break;
						case EM_ALOX:	menu_value = "Alox"; break;
						case EM_40:		menu_value = "em40"; break;
						case EM_60:		menu_value = "em60"; break;
						case EM_SNOW:	menu_value = "snow"; break;
						case EM_PTFE:	menu_value = "PTFE"; break;
						case EM_PAINT:	menu_value = "pnt."; break;
						case EM_LIME:	menu_value = "lime"; break;
						case EM_PLANT:	menu_value = "vege"; break;
						case EM_WATER:	menu_value = " H2O"; break;
						case EM_SKIN:	menu_value = "skin"; break;
						case EM_MAX:	menu_value = " max"; break;
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
					cleanup();
					break;
			#endif
				case UI_FPS:		// cycle through available frame rates
					refresh_rate++;
					if (refresh_rate > MaximumRefreshRate) refresh_rate = MinimumRefreshRate;
					MLX90640_SetRefreshRate(MLX90640_ADDR, refresh_rate);
					cleanup();
					break;

				case UI_SCALE:		// cycle through available color maps
					colormap++;
					if (colormap >= CM_END) colormap = CM_START;
					switch (colormap) {
						case CM_BLACKBODY:	camColors = (uint16_t*)&(blackbody.pixel_data[0]); break;
						case CM_RAINBOW:	camColors = (uint16_t*)&(rainbow.pixel_data[0]); break;
						case CM_CONTRAST:	camColors = (uint16_t*)&(contrast.pixel_data[0]); break;
						case CM_SKYLINE:	camColors = (uint16_t*)&(skyline.pixel_data[0]); break;
						default: 			camColors = (uint16_t*)&(skyline.pixel_data[0]); break;
					}
					break;

				case UI_RANGE:		// cycle through available temperature ranges
					range++;
					if (range >= RAN_END) range = RAN_START;
					switch (range) {
						case RAN_AUTO:	range_low = -100; range_high = -100; break;
						case RAN_COLD:	range_low = -40; range_high = 10; break;
						case RAN_ROOM:	range_low = 10; range_high = 40; break;
						case RAN_WATER:	range_low = 0; range_high = 100; break;
						case RAN_BOIL:	range_low = 70; range_high = 110; break;
						case RAN_MAX:	range_low = -40; range_high = 300; break;
						default: 		range_low = -100; range_high = -100; break;
					}
					break;

				case UI_EMIS:		// cycle through available emissivities
					emindex++;
					if (emindex >= EM_END) emindex = EM_START;
					switch (emindex) {
						// common values form https://en.wikipedia.org/wiki/Emissivity#Emissivities_of_common_surfaces
						case EM_METAL:	emissivity = 0.04; break;
						case EM_ALOX:	emissivity = 0.20; break;
						case EM_40:		emissivity = 0.40; break;
						case EM_60:		emissivity = 0.60; break;
						case EM_SNOW:	emissivity = 0.80; break;
						case EM_PTFE:	emissivity = 0.85; break;
						case EM_PAINT:	emissivity = 0.90; break;
						case EM_LIME:	emissivity = 0.92; break;
						case EM_PLANT:	emissivity = 0.94; break;
						case EM_WATER:	emissivity = 0.96; break;
						case EM_SKIN:	emissivity = 0.98; break;
						case EM_MAX:	emissivity = 1.00; break;
						default: emissivity = 0.95; break;
					}
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
