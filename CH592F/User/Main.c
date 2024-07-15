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
uint8_t last_AT_done=1;
uint8_t initt=0;
uint8_t rx[100];
/*int send_to_ec801()
{}
*/

/*int at_ready()
{
    UART1_SendString("AT+CPIN?\r\n", 10);
    strcmp(RxBuff,"RDY");
}*//*
int AT_send()
{
    UART3_SendString("sendcpin",8 );
    UART1_SendString("AT+CPIN?\r\n", 10);
    UART3_SendString("sendcpin",8 );
    while(!strcmp(RxBuff,""))
    last_AT_done=1;
        while(last_AT_done){UART3_SendString(&last_AT_done, 1);DelayMs(500);}
    UART1_SendString("AT+CGREG?\r\n", 11);
    UART3_SendString("sendcREG",8 );
    last_AT_done=1;
    while(last_AT_done){UART3_SendString(&last_AT_done, 1);DelayMs(500);}
    char set_pdp[]="AT+QICSGP=1,1\r\n";
    UART1_SendString(set_pdp,strlen(set_pdp));
    last_AT_done=1;
    while(last_AT_done){UART3_SendString(&last_AT_done, 1);DelayMs(500);}
    UART1_SendString("AT+QIACT=1\r\n", 12);
    last_AT_done=1;
    while(last_AT_done){UART3_SendString(&last_AT_done, 1);DelayMs(500);}
    char con_tcp[]="AT+QIOPEN=1,0,\"TCP\",\"112.27.250.90\",61135,0,1";
    UART1_SendString(con_tcp, strlen(con_tcp));
    initt=1;
}*/

int main()
{

    SetSysClock(CLK_SOURCE_PLL_60MHz);

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
   // TMR0_TimerInit(60000000);
   // TMR0_ITCfg(ENABLE,TMR0_3_IT_CYC_END);
   // PFIC_EnableIRQ(TMR0_IRQn);
    UART3_SendString("ready",5);
while(1);
}


__INTERRUPT
void UART1_IRQHandler(void)
{
    if(UART1_GetITFlag()==UART_II_RECV_RDY)
    {
        while(R8_UART1_LSR == RB_LSR_DATA_RDY)
        {

            memcpy(RxBuff,rx,UART1_RecvString(rx));
            memset(rx,0,100);
        }
        UART3_SendString(RxBuff,30 );
        memset(RxBuff,0,200);
    }
}


__INTERRUPT
void UART3_IRQHandler(void)
{
    if(UART3_GetITFlag()==UART_II_RECV_RDY)
    {
        UART3_RecvString(RxBuff);
        UART1_SendString(RxBuff, strlen(RxBuff));
        memset(RxBuff,0,100);
    }
}
/*
__INTERRUPT
void TMR0_IRQHandler(void)
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        send_to_ec801();
    }
}*/
