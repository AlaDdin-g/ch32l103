/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/07/08
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *Touchkey WAKEUP routine:
 *This example provide Touchkey WakeUp , when Touch the channel of PA2 Touchkey,
 * sleep wake will be performed.
 *
 */

#include "debug.h"

/* Global define */

/*********************************************************************
 * @fn      Touch_Key_Init
 *
 * @brief   Initializes Touch Key collection.
 *
 * @return  none
 */
void Touch_Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	ADC_InitTypeDef ADC_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE );
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_ADC1, ENABLE );
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_TKey_WakeUpCmd(ADC1, ADC_Channel_2,ADC_TKey_WakeUp_Mode1,ENABLE);

    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TKeyWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

	ADC_Cmd(ADC1, ENABLE);
	TKey1->CTLR1 |= (1<<26)|(1<<24);     // Enable TouchKey and Buffer
}

/*********************************************************************
 * @fn      Touch_Key_Adc
 *
 * @brief   Returns ADCx conversion result data.
 *
 * @param   ch - ADC channel.
 *            ADC_Channel_0 - ADC Channel0 selected.
 *            ADC_Channel_1 - ADC Channel1 selected.
 *            ADC_Channel_2 - ADC Channel2 selected.
 *            ADC_Channel_3 - ADC Channel3 selected.
 *            ADC_Channel_4 - ADC Channel4 selected.
 *            ADC_Channel_5 - ADC Channel5 selected.
 *            ADC_Channel_6 - ADC Channel6 selected.
 *            ADC_Channel_7 - ADC Channel7 selected.
 *            ADC_Channel_8 - ADC Channel8 selected.
 *            ADC_Channel_9 - ADC Channel9 selected.
 *            ADC_Channel_16 - ADC Channel16 selected.
 *            ADC_Channel_17 - ADC Channel17 selected.
 *
 * @return  val - The Data conversion value.
 */
u16 Touch_Key_Adc(u8 ch)
{
  ADC_RegularChannelConfig(ADC1,ch, 1, ADC_SampleTime_CyclesMode2 );
  TKey1->IDATAR1 =0x10;  //Charging Time
  TKey1->RDATAR =0x8;   //Discharging Time
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));

  return (uint16_t) TKey1->RDATAR;
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	u16 ADC_val;

	SystemCoreClockUpdate();
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

	Touch_Key_Init();
    printf("\r\n ********** \r\n");
    __WFI();
    printf("\r\n ########## \r\n");
	while(1)
	{
	    ADC_val = Touch_Key_Adc(ADC_Channel_2);
	    printf("TouckKey Value:%d\r\n",ADC_val);
	    Delay_Ms(500);
	}
}

void TKeyWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      TIM1_CC_IRQHandler
 *
 * @brief   This function handles TIM1  Capture Compare Interrupt exception.
 *
 * @return  none
 */
void TKeyWakeUp_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line18)!=RESET)
    {
        SystemInit();
        printf("EXTI18 Wake_up\r\n");

        ADC_TKey_WakeUpCmd(ADC1, ADC_Channel_2,ADC_TKey_WakeUp_Mode1,DISABLE);

        EXTI_ClearITPendingBit(EXTI_Line18);     /* Clear Flag */
    }
}