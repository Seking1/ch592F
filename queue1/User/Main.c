#include "CH59x_common.h"
typedef struct{
    uint16_t head;
    uint16_t tail;
    //uint8_t count;
    uint16_t bottom_point;
    char data[1001];
}Queue_t;

typedef struct {
    unsigned char len;
    unsigned char type;
    unsigned char data[50];
}MAIN_QUEUE_ITEM;

typedef enum{
    Uart3_msg=0x01,
    modem_msg=0x02,
    uart0_msg=0x03,
}msg_type;

typedef enum{
    connect=0x01,
    close=0x02,
    recv=0x03,
}tcp_msg_type;
int time_tick=0;

uint8_t RxBuff[200];
uint8_t trigB;
uint8_t rx[20];
uint8_t nowprt=0;uint8_t len0 = 0;
uint8_t tag=0;
Queue_t mainqueue;
void queue_init()
{
    mainqueue.head=(uint16_t)0;
    mainqueue.tail=(uint16_t)0;
    mainqueue.bottom_point=(uint16_t)1000;
}
int queue_push(Queue_t *queue,MAIN_QUEUE_ITEM *item) {
    if((queue->tail+item->len+2)> 1000)//超出缓存区
    {
        queue->bottom_point = queue->tail;//将缓存区底置为尾部
        queue->tail = 0;//尾指向0
    }
        memcpy(&queue->data[queue->tail], item, item->len+2);
        //UART3_SendString(&queue->data[queue->head],item->len+2 );
        queue->tail=queue->tail+item->len+2;
    return 1;
}
int queue_pop(Queue_t *queue,MAIN_QUEUE_ITEM *item) {
    if(queue->head == queue->tail)return 0;//头指向尾，无数据
    if(queue->head ==  queue->bottom_point)//头指向底，置0
    {
        queue->head = 0;
    }
    memcpy(item,&queue->data[queue->head],queue->data[queue->head]+2);

    queue->head=queue->data[queue->head]+queue->head+2;

    return 1;
}
MAIN_QUEUE_ITEM uartmsg;
MAIN_QUEUE_ITEM recvmsg;
MAIN_QUEUE_ITEM MODEMmsg;
/*
static void modem_uart_reset(void)
{
    UART3_SendString("modem_uart_reset\n",17 );

    //复位控制器
    PFIC_DisableIRQ(UART1_IRQn);
    UART1_INTCfg(DISABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    UART1_Reset();

    //复位MODEM
    GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA);
    GPIOB_ResetBits(GPIO_Pin_23);
    task_delay(10);
    GPIOB_SetBits(GPIO_Pin_23);
    task_delay(500);
    GPIOB_ResetBits(GPIO_Pin_23);

    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART1_DefInit();
    UART1_ByteTrigCfg(UART_7BYTE_TRIG);
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART1_IRQn);
}*/

void UART_init()
{
        GPIOA_SetBits(GPIO_Pin_5);
        GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RXD-配置上拉输入
        GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
        UART3_DefInit();
        GPIOA_SetBits(GPIO_Pin_9);
        GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      // RXD-配置上拉输入
        GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
        UART1_DefInit();
        UART1_ByteTrigCfg(0);
        UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
        PFIC_EnableIRQ(UART1_IRQn);
        UART3_ByteTrigCfg(0);
        UART3_INTCfg(ENABLE, RB_IER_RECV_RDY);
        PFIC_EnableIRQ(UART3_IRQn);
}
int get_AT_respone()
{
    if(recvmsg.len > 17 && recvmsg.data[0]== '+' && recvmsg.data[1]=='Q' && recvmsg.data[2]== 'I')
    {
      unsigned char buf[50];
      for(int i=0;i<recvmsg.len;i++)
      {
          if(recvmsg.data[i] == '\"')
          {
              i++;
              for(int j=0;j<recvmsg.len-1;j++)
              {
                  strtok_r(&recvmsg.data,)
              }
          }

      }
      UART3_SendString("AT_RESPON:",10);
      UART3_SendString(&recvmsg.data[0], recvmsg.len);
      return 1;
    }
    return 0;
}

int is_645(MAIN_QUEUE_ITEM recv)
{
    if(recv.len > 10 && recv.data[0]==104 && recv.data[7]==104 && recv.data[recv.len-1]==22)return 1;
    else return 0;
}

int is_AT(MAIN_QUEUE_ITEM recv)
{
    if(recv.data[0]=='A' && recv.data[1]=='T' && recv.data[2]=='+')return 1;
    else return 0;
}

int main()
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    UART_init();
    UART1_SendString("ATE0\r\n",6);
    //TMR0_TimerInit(600000);//10ms
    //TMR0_ITCfg(ENABLE,TMR0_3_IT_CYC_END);
    //PFIC_EnableIRQ(TMR0_IRQn);
    //PFIC_DisableIRQ(TMR0_IRQn);
    queue_init();
    while(1)
    {
        if(queue_pop(&mainqueue, &recvmsg)) //UART3_SendString("nodata\r\n",8);
        {
            if(recvmsg.type == modem_msg)//来自modem的消息
            {
                get_AT_respone();
                if (is_645(recvmsg))//是645，则来自服务器，直接送至电表
                {
                    UART3_SendString("S->C:",5);
                    UART3_SendString("is645server\r\n",13);
                    UART3_SendString(&recvmsg.data,recvmsg.len);
                }
                else if (is_AT(recvmsg))//是AT，来自服务器，发送至modem
                {
                    UART3_SendString("S->C:",5);
                    UART3_SendString("isATserver\r\n",12);
                    UART3_SendString("C->M:",4);
                    UART1_SendString(&recvmsg.data, recvmsg.len);//来自串口的AT发到modem
                }
                else//来自modem的回复或者其他透传信息
                {
                    //if(is_)
                    UART3_SendString("modem_respone_msg:\r\n",19);
                    UART3_SendString(&recvmsg.data,recvmsg.len);
                }
            }
            else if(recvmsg.type == Uart3_msg)//来自串口调试消息
            {
                if (is_645(recvmsg))//是645，来自串口，送至电表
                {
                    UART3_SendString("C->V:",5);
                    UART3_SendString("is645Serial\r\n",13);
                }
                else if (is_AT(recvmsg))//是AT，来自串口，发送至modem
                {
                    UART3_SendString("S->C:",5);
                    UART3_SendString("isATSerial\r\n",12);
                    UART3_SendString("C->M:",5);
                    UART1_SendString(&recvmsg.data, recvmsg.len);//来自串口的AT发到modem
                }
                else//其他信息
                {
                    UART3_SendString("other_msg\r\n",11);
                    UART3_SendString(&recvmsg.data,recvmsg.len);
                }
            }
            else if(recvmsg.type == uart0_msg)//来自电表的回复
            {
                if (is_645(recvmsg))//是645，来自电表，送至服务器
                {
                    unsigned char from_meter[40];
                    snprintf(from_meter,sizeof(from_meter),"AT+QISEND=0,%d\r\n",recvmsg.len);//待确认socket场景号，新增函数
                    UART1_SendString(from_meter, strlen(from_meter));
                    UART1_SendString(&recvmsg.data, recvmsg.len);
                    UART3_SendString("C->M->S",7);
                }
            }
            else UART3_SendString("NO645AT\r\n",9);
        }
        //DelayMs(2000);
    }
}

void TMR0_IRQHandler(void)
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        if(time_tick>0)time_tick--;
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);    // 清除中断标志
    }
}


uint8_t Rx[20];
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
            {int j=1000;
                while(j-- || (R8_UART3_LSR == RB_LSR_DATA_RDY))
                    {
                        i=UART3_RecvString(Rx);
                        memcpy(RxBuff+nowprt,Rx,i);
                        nowprt+=i;
                        memset(Rx,0,10);
                    }
                uartmsg.len=nowprt;
                uartmsg.type=Uart3_msg;
                memcpy(uartmsg.data,RxBuff,nowprt);
                queue_push(&mainqueue, &uartmsg);
                //mainqueue.count++;
                //UART3_SendString(&mainqueue.tail,1);
                memset(RxBuff,0,200);
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}


uint8_t Rx1[20];
__INTERRUPT
void UART1_IRQHandler(void)
{
    volatile uint8_t i;
    uint8_t nowprt=0;
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART1_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            {int j=1000;
                while(j-- || (R8_UART1_LSR == RB_LSR_DATA_RDY))
                    {
                        i=UART1_RecvString(Rx);
                        memcpy(RxBuff+nowprt,Rx,i);
                        nowprt+=i;
                        memset(Rx,0,10);
                    }
                uartmsg.len=nowprt;
                uartmsg.type=modem_msg;
                memcpy(uartmsg.data,RxBuff,nowprt);
                queue_push(&mainqueue, &uartmsg);
                //mainqueue.count++;
                //UART3_SendString(&mainqueue.tail,1);
                memset(RxBuff,0,200);
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }

}
