#ifndef __TIM_H
#define	__TIM_H

#include "bsp.h"
 
#define AC_SIG_EDGE_RISE  0
#define AC_SIG_EDGE_FALL  1

void TimeBase_Init(void);  //TIM5 
void PWMOut_LCD_BK(void);  //TIM4
void TIM1_Encoder_Init(void);
void PWMOut_LCD_BK_Init(void);
void Set_LCD_Bk(uint32_t pulse);
void ACSig_In_Capture_Init(void);

#endif

