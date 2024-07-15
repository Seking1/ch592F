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
/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_common.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/


#ifndef __CH59x_COMM_H__
#define __CH59x_COMM_H__
#endif
#ifdef __cplusplus
 extern "C" {
#endif


#ifndef  NULL
#define  NULL     0
#endif
#define  ALL            0xFFFF

#ifndef __HIGH_CODE
#define __HIGH_CODE   __attribute__((section(".highcode")))
#endif

#ifndef __INTERRUPT
    #ifdef INT_SOFT
#define __INTERRUPT   __attribute__((interrupt()))
#else
#define __INTERRUPT   __attribute__((interrupt("WCH-Interrupt-fast")))
#endif
#endif

#define Debug_UART0        0
#define Debug_UART1        1
#define Debug_UART2        2
#define Debug_UART3        3

#ifdef DEBUG
#include <stdio.h>
#endif
/*
 * @brief  系统主频时钟（Hz）
*/
#ifndef  FREQ_SYS
#define  FREQ_SYS       60000000
#endif
#ifndef  SAFEOPERATE
#define  SAFEOPERATE   __nop();__nop()
#endif

/**
 * @brief  32K时钟（Hz）
*/
#ifdef CLK_OSC32K
#if ( CLK_OSC32K == 1 )
#define CAB_LSIFQ       32000
#else
#define CAB_LSIFQ       32768
#endif
#else
#define CAB_LSIFQ       32000
#endif

#include <string.h>
#include <stdint.h>
#include "CH592SFR.h"
#include "core_riscv.h"
#include "CH59x_clk.h"
#include "CH59x_uart.h"
#include "CH59x_gpio.h"
#include "CH59x_i2c.h"
#include "CH59x_flash.h"
#include "CH59x_pwr.h"
#include "CH59x_pwm.h"
#include "CH59x_adc.h"
#include "CH59x_sys.h"
#include "CH59x_timer.h"
#include "CH59x_spi.h"
#include "CH59x_usbdev.h"
#include "CH59x_usbhost.h"
#include "ISP592.h"


#define DelayMs(x)      mDelaymS(x)
#define DelayUs(x)      mDelayuS(x)

uint8_t TxBuff[] = "This is a tx exam\r\n";
uint8_t RxBuff[100];
uint8_t Rx[10];
/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main()
{

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* 配置串口1：先配置IO口模式，再配置串口 */
    GPIOA_SetBits(GPIO_Pin_5);
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART3_DefInit();
    UART3_SendString(TxBuff, sizeof(TxBuff));
    UART3_ByteTrigCfg(0);
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART3_IRQn);


    while(1);
}

__INTERRUPT
void UART3_IRQHandler(void)
{
    volatile uint8_t i;
    uint8_t nowprt=0;
    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART3_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            UART3_SendString("1", 1);
            uint8_t j=200;
            while(j-- || (R8_UART3_LSR == RB_LSR_DATA_RDY))
            {
                   i=UART3_RecvString(Rx);
                memcpy(RxBuff+nowprt,Rx,i);
                nowprt+=i;
                memset(Rx,0,10);
            }
            UART3_SendString(&nowprt, 1);
            UART3_SendString(RxBuff, nowprt);
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

