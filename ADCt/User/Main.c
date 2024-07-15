/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/03/09
 * Description        : adc采样示例，包括温度检测、单通道检测、差分通道检测、TouchKey检测、中断方式采样。
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"
#include "lcd.h"
uint16_t adcBuff[40];

volatile uint8_t adclen;
volatile uint8_t DMA_end = 0;

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main()
{

    uint8_t      i;
    signed short RoughCalib_Value = 0; // ADC粗调偏差值

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* 配置串口调试 */
    GPIOA_SetBits(GPIO_Pin_5);
        GPIOA_ModeCfg(GPIO_Pin_4,GPIO_ModeIN_PD);
        GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);
        UART3_DefInit();
    lcd_disp_init();
    lcd_disp_blank();
    ADC_InterTSSampInit();


    while(1){
        int adv_value=ADC_ExcutSingleConver();
        uint8_t temp = adc_to_temperature_celsius(adv_value);
        char tempc[2] ={(temp/10)+48,(temp%10)+48};
        UART3_SendString(tempc,2);
        lcd_disp_num(temp,0,LCD_F_REM |LCD_F_KW|LCD_F_DOT|LCD_F_H,10);
        DelayMs(1000);
    }
}


