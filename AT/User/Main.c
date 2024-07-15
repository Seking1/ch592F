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

uint8_t RxBuff[200];
uint8_t trigB;
uint8_t rx[20];
typedef struct uartmsg
{
    char data[20];
    uint8_t len;
}uartmsg;
typedef struct MSG
{
    char data[200];
    uint8_t len;
}MSG;
uartmsg uart1msg;uint8_t nowprt=0;uint8_t len0 = 0;
MSG mainmsg;
uint8_t tag=0;
int is_last_at_res_right()
{
    UART3_SendString(&mainmsg.data[nowprt-4], 1);
    UART3_SendString(&mainmsg.data[nowprt-3], 1);
    if(mainmsg.data[nowprt-4] =='O' && mainmsg.data[nowprt-3] =='K')
        {nowprt=0;
        return 1;}
    else {
        return 0;
    }
}

int connect()
{
    UART3_SendString("AT+CP",5);
    DelayMs(1000);
    UART1_SendString("AT+CPIN?\r\n",10);
    DelayMs(1000);
    if(is_last_at_res_right())UART3_SendString("cpinok\r\n", 8);
    UART1_SendString("AT+CGREG?\r\n",11);
    DelayMs(1000);
        if(is_last_at_res_right())UART3_SendString("CGREGok\r\n", 9);
    UART1_SendString("AT+QICSGP=1,1?\r\n",16);
    DelayMs(1000);
        if(is_last_at_res_right())UART3_SendString("csGPok\r\n", 8);
    UART1_SendString("AT+QIACT=1\r\n",12);
    DelayMs(1000);
            if(is_last_at_res_right())UART3_SendString("QIACTok\r\n", 9);
            char opentcp[]="AT+QIOPEN=1,0,\"TCP\",\"112.27.250.90\",61135,0,1\r\n";
    UART1_SendString(opentcp,strlen(opentcp));
    DelayMs(1000);
                    if(is_last_at_res_right()){UART3_SendString("openok\r\n", 8);
                    return 1;
                    }
                    else {return 0;}

}
int main()
{

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    uart1msg.len=0;
    mainmsg.len=0;
    /* 配置串口1：先配置IO口模式，再配置串口 */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART1_DefInit();
    UART1_ByteTrigCfg(0);
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
    GPIOA_SetBits(GPIO_Pin_5);
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART3_DefInit();
    UART3_ByteTrigCfg(0);
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART3_IRQn);
    DelayMs(5000);
    connect();
while(1)
{}//UART3_SendString(mainmsg.data, nowprt);memset(mainmsg.data,0,200);nowprt=0;DelayMs(1000);}
}

__INTERRUPT
void UART1_IRQHandler(void)
{
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART1_GetLinSTA();
        }
            break;

        case UART_II_RECV_RDY:
        case UART_II_RECV_TOUT:// 接收超时，暂时一帧数据接收完成

            len0=UART1_RecvString(uart1msg.data);
            memcpy(&mainmsg.data[nowprt],&uart1msg.data,len0);
            nowprt+=len0;
            mainmsg.len++;
            memset(uart1msg.data,0,20);
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}



__INTERRUPT
void UART3_IRQHandler(void)
{
    if(UART3_GetITFlag()==UART_II_RECV_RDY)
    {
        UART3_RecvString(RxBuff);
        UART1_SendString(RxBuff, strlen(RxBuff));
        memset(RxBuff,0,200);
    }
}
