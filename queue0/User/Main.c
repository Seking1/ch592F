#include "CH59x_common.h"
typedef struct{
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint8_t bottom_point;
    char data[1001];
}Queue_t;
typedef struct {
    unsigned char len;
    unsigned char type;
    unsigned char data[50];
}MAIN_QUEUE_ITEM;
typedef enum{
    Uart3_dialog=0x01,
    modem_dialog=0x02,
}msg_type;
uint8_t RxBuff[200];
uint8_t trigB;
uint8_t rx[20];
uint8_t nowprt=0;uint8_t len0 = 0;
uint8_t tag=0;
Queue_t mainqueue;
void queue_init()
{
    mainqueue.head=0;
    mainqueue.tail=0;
    mainqueue.bottom_point=(uint8_t)1000;
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
int main()
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    GPIOA_SetBits(GPIO_Pin_5);
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART3_DefInit();
    UART3_ByteTrigCfg(0);
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART3_IRQn);
    queue_init();
    while(1)
    {
        if(!queue_pop(&mainqueue, &recvmsg))UART3_SendString("nodata\r\n",8);
        else UART3_SendString(&recvmsg.data,recvmsg.len);
        DelayMs(2000);
    }
}

uint8_t Rx[10];
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
            {int j=300;
                while(j-- || (R8_UART3_LSR == RB_LSR_DATA_RDY))
                    {
                        i=UART3_RecvString(Rx);
                        memcpy(RxBuff+nowprt,Rx,i);
                        nowprt+=i;
                        memset(Rx,0,10);
                    }
                uartmsg.len=nowprt;
                uartmsg.type=Uart3_dialog;
                memcpy(uartmsg.data,RxBuff,nowprt);
                queue_push(&mainqueue, &uartmsg);
                mainqueue.count++;
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

