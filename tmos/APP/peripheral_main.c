/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外设从机应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"

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
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        UART3_SendString("turn\r\n", 6);
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
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
    PRINT("%s\n", VER_LIB);
    CH59x_BLEInit();
    HAL_Init();
    Peripheral_Init();
    Main_Circulation();
}

/******************************** endfile @ main ******************************/
