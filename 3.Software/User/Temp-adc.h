#ifndef __TEMP_ADC_H
#define	__TEMP_ADC_H

#include "bsp.h"
 
extern uint32_t ADCValue[2];

void ADC_DMA_Init(void);
void OverTemp_Detect(void);
	
#endif

