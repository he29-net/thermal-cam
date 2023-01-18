#ifndef __TIM_H
#define	__TIM_H

#include "bsp.h"
 
#define AC_SIG_EDGE_RISE  0
#define AC_SIG_EDGE_FALL  1

void TIM1_Encoder_Init(void);		// uses TIM1
void ACSig_In_Capture_Init(void);	// uses TIM2
void PWMOut_LCD_BK_Init(void);		// uses TIM4
void Set_LCD_Bk(uint32_t pulse);

#endif

