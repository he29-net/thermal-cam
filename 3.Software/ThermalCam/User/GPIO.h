#ifndef __GPIO_H
#define	__GPIO_H

#include "bsp.h"

#define RELAY_STA_9V   0
#define RELAY_STA_18V  1
#define RELAY_STA_27V  2
#define RELAY_STA_36V  3

#define RELAY_DAC_DELAY_TIME  40   //40ms

///*********************************** Run Led ******************************/
//#define RUN_LED_PIN               GPIO_PIN_1
//#define RUN_LED_PORT              GPIOA
//#define RUN_LED_ON()              gpio_bit_reset(RUN_LED_PORT, RUN_LED_PIN)
//#define RUN_LED_OFF()             gpio_bit_set(RUN_LED_PORT, RUN_LED_PIN)

/*********************************** USB_PULLUP ******************************/
#define USB_PULLUP_PIN               GPIO_PIN_10
#define USB_PULLUP_PORT              GPIOA
#define USB_PULLUP_L()               gpio_bit_reset(USB_PULLUP_PORT, USB_PULLUP_PIN)
#define USB_PULLUP_H()               gpio_bit_set(USB_PULLUP_PORT, USB_PULLUP_PIN)

#define CRG_STA_PIN                  GPIO_PIN_13
#define CRG_STA_PORT                 GPIOC
#define CRG_STA_READ()               gpio_input_bit_get(CRG_STA_PORT, CRG_STA_PIN)

void Delay_us(uint16_t us);
void delay_1ms(uint16_t ms);
void mGPIO_Init(void);
void gd_eval_led_toggle(void);


#endif

