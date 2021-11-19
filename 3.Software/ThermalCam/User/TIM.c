#include "TIM.h"

/**********************************************************************************************************
*	�� �� ����TIM1_Encoder_Init
*	����˵����������ť��ʼ��
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/ 
void TIM1_Encoder_Init(void)  //TIM1   ENC_A-PA0   ENC_B-PA1
{
	timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_TIMER1);
	
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_1);

    timer_deinit(TIMER1);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 65535;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);
	
	timer_slave_mode_select(TIMER1,TIMER_ENCODER_MODE0);

    /* auto-reload preload enable */
//    timer_auto_reload_shadow_enable(TIMER1);
	
    /* TIMER1 counter enable */
    timer_enable(TIMER1);	
}

/**********************************************************************************************************
*	�� �� ����ACSig_In_Capture_Init
*	����˵�������㲨��Ϊ100Hz��10ms���ߵ�ƽΪ500us�ķ�����
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/ 
void ACSig_In_Capture_Init(void)  //TIM2_CH0    AC_SIG-PB4
{
    timer_ic_parameter_struct timer_icinitpara;
    timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_TIMER2);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);

	//��ӳ��
	gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);	   
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);	
	
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(TIMER2_IRQn, 1, 1);	
	
    /* TIMER2 configuration: input capture mode -------------------
    the external signal is connected to TIMER2 CH0 pin (PB4)
    the rising edge is used as active edge
    the TIMER2 CH0CV is used to compute the frequency value
    ------------------------------------------------------------ */   
    timer_deinit(TIMER2);

    /* TIMER2 configuration */
    timer_initpara.prescaler         = 215;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;  
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 65535;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2,&timer_initpara);

    /* TIMER2  configuration */
    /* TIMER2 CH0 input capture configuration */
    timer_icinitpara.icpolarity  = TIMER_IC_POLARITY_RISING;     //������
    timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x4;
    timer_input_capture_config(TIMER2,TIMER_CH_0,&timer_icinitpara);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER2);
    /* clear channel 0 interrupt bit */
    timer_interrupt_flag_clear(TIMER2,TIMER_INT_CH0);
    /* channel 0 interrupt enable */
    timer_interrupt_enable(TIMER2,TIMER_INT_CH0);

    /* TIMER2 counter enable */
    timer_enable(TIMER2);	
}

/**********************************************************************************************************
*	�� �� ����PWMOut_LCD_BK_Init
*	����˵����TFT����PWM��ʼ��
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/ 
void PWMOut_LCD_BK_Init(void)  //TIM4_CH2    LCD_BK-PA2
{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
	
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);

    /*Configure PA2(TIMER4 CH2) as alternate function*/
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* -----------------------------------------------------------------------
    TIMER1 configuration: generate 3 PWM signals with 3 different duty cycles:
    TIMER1CLK = SystemCoreClock / 54 = 2MHz

    TIMER1 channel1 duty cycle = (4000/ 16000)* 100  = 25%
    TIMER1 channel2 duty cycle = (8000/ 16000)* 100  = 50%
    TIMER1 channel3 duty cycle = (12000/ 16000)* 100 = 75%
    ----------------------------------------------------------------------- */

    rcu_periph_clock_enable(RCU_TIMER4);
    timer_deinit(TIMER4);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 53;  //TIMER1CLK = SystemCoreClock / 54 = 2MHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 99;  // 2MHz/100=20KHz  LCD����Ƶ��20K
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER4,&timer_initpara);

    /* CH2 configuration in PWM mode1 */
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_LOW;       //ͨ���������
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;            //ͨ�����״̬
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;     //ͨ�����ڿ���ʱ�����	
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;     //����ͨ��������� 
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;          //����ͨ�����״̬
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;    //����ͨ�����ڿ���ʱ�����

    timer_channel_output_config(TIMER4,TIMER_CH_2,&timer_ocintpara);

    /* CH2 configuration in PWM mode1,duty cycle 50% */
    timer_channel_output_pulse_value_config(TIMER4,TIMER_CH_2,89);
    timer_channel_output_mode_config(TIMER4,TIMER_CH_2,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER4,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER4);
    /* auto-reload preload enable */
    timer_enable(TIMER4);	
}

/**********************************************************************************************************
*	�� �� ����Set_LCD_Bk
*	����˵��������LCD�ı���
*	��    �Σ�pulse:����ֵ��ȡֵ��Χ��0-99��ȡֵ0�رձ��⣬ȡֵ99��������
*	�� �� ֵ����
**********************************************************************************************************/ 
void Set_LCD_Bk(uint32_t pulse)
{
	TIMER_CH2CV(TIMER4) = (uint32_t)pulse;
}

/**********************************************************************************************************
*	�� �� ����TimeBase_Init
*	����˵����
*	��    �Σ�
*	�� �� ֵ��
**********************************************************************************************************/ 
//void TimeBase_Init(void)  //TIM5
//{
//    /* ----------------------------------------------------------------------------
//    TIMER5 Configuration: 
//    TIMER5CLK = SystemCoreClock/5400 = 20KHz.
//    TIMER5 configuration is timing mode, and the timing is 0.2s(4000/20000 = 0.2s).
//    CH0 update rate = TIMER5 counter clock/CH0CV = 20000/4000 = 5Hz.
//    ---------------------------------------------------------------------------- */
//    timer_oc_parameter_struct timer_ocinitpara;
//    timer_parameter_struct timer_initpara;

//    rcu_periph_clock_enable(RCU_TIMER5);

//    timer_deinit(TIMER5);
//    /* initialize TIMER init parameter struct */
//    timer_struct_para_init(&timer_initpara);
//    /* TIMER5 configuration */
//    timer_initpara.prescaler         = 5399;
//    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
//    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
//    timer_initpara.period            = 4000;
//    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
//    timer_init(TIMER5, &timer_initpara);

//    /* initialize TIMER channel output parameter struct */
//    timer_channel_output_struct_para_init(&timer_ocinitpara);
//    /* CH0,CH1 and CH2 configuration in OC timing mode */
//    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
//    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
//    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
//    timer_channel_output_config(TIMER5, TIMER_CH_0, &timer_ocinitpara);

//    /* CH0 configuration in OC timing mode */
//    timer_channel_output_pulse_value_config(TIMER5, TIMER_CH_0, 2000);
//    timer_channel_output_mode_config(TIMER5, TIMER_CH_0, TIMER_OC_MODE_TIMING);
//    timer_channel_output_shadow_config(TIMER5, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

//    timer_interrupt_enable(TIMER5, TIMER_INT_CH0);
//    timer_enable(TIMER5);
//	
//    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
//    nvic_irq_enable(TIMER5_IRQn, 1, 1);	
//}

//void TIMER5_IRQHandler(void)
//{
//    if(SET == timer_interrupt_flag_get(TIMER5, TIMER_INT_CH0)){       
//        timer_interrupt_flag_clear(TIMER5, TIMER_INT_CH0);
//		
//        gd_eval_led_toggle();
//    }
//}


