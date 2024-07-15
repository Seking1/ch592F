/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : 串口1收发演示
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"
#include "lcd.h"
uint8_t TxBuff[] = "This is a tx exam\r\n";
uint8_t RxBuff[100];
uint8_t trigB;
uint8_t i;
/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
void set_TMR0IRQ()
{
 TMR0_TimerInit(FREQ_SYS);
    TMR0_ITCfg(ENABLE,TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR0_IRQn);
}

int main()
{
    i=10;
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    GPIOA_SetBits(GPIO_Pin_5);
    GPIOA_ModeCfg(GPIO_Pin_4,GPIO_ModeIN_PD);
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);
    UART3_DefInit();
    lcd_disp_init();
    lcd_disp_blank();
    set_TMR0IRQ();
    UART3_SendString("TMROK", 5);

    while(1);

}

__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void)
{
       if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
       {
           TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // 清除中断标志
           //lcd_disp_num(10,0,LCD_F_DOT,10);
           lcd_disp_num(i,0,LCD_F_REM |LCD_F_KW|LCD_F_DOT|LCD_F_H,10);
           i++;
           UART3_SendString(i+48, 8);
       }
}





