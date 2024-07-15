/*
 * lcd.c
 *
 *  Created on: 2023年2月9日
 *      Author: A
 */
#include "CH59x_common.h"
#include "lcd.h"

extern void App_Printf( const char *fmt, ... );

#if (DB_TYPE == 1)
/******************TM1621C模块命令定义*********************/
#define SYSDIS   0x00    //关系统振荡器和LCD偏压发生器
#define SYSEN    0x02    //打开系统振荡器

#define LCDOFF   0x04     //关LCD偏压
#define LCDON    0x06     //开LCD偏压

#define RC       0x30     //内部RC振荡

#define BIAS     0x52     //1/3偏压 4公共口

/********************定义控制端口**********************/
#define CS      (GPIO_Pin_12)
#define WRITE   (GPIO_Pin_13)
#define DATA    (GPIO_Pin_14)

/********************定义数据*************************/
static uchar gs_lcd_ram[16];

//0~F字型码 负号码
const static uchar xdigital_data[] = {
    0xF5, 0x60, 0xD3, 0xF2, 0x66, 0xB6, 0xB7, 0xE0,
    0xF7, 0xF6, 0xE7, 0x37, 0x95, 0x73, 0x97, 0x87,
    0x02,
};

const static uchar xdigital_addr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13,
};

const static uchar flag_data[] = {
    0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04, 0x08, 0x08,
};

const static uchar flag_addr[] = {
    0x13, 0x02, 0x04, 0x0D, 0x00, 0x13, 0x13, 0x11, 0x0F,
};


/********************从高位写入数据*************************/
static void Write_Data_H( uchar Data, uchar Cnt )     //Data的高cnt位写入TM1621C，高位在前
{
    uchar i;

    for( i = 0; i < Cnt; i++ )
    {
        GPIOA_ResetBits( WRITE );
        if( Data & 0x80 )                               //从最高位发送
            GPIOA_SetBits( DATA );
        else
            GPIOA_ResetBits( DATA );
        __asm__("nop");
        __asm__("nop");
        GPIOA_SetBits( WRITE );
        Data <<= 1;
    }
    GPIOA_ResetBits( WRITE );
    GPIOA_ResetBits( DATA );
}

/********************从低位写入数据*************************/
static void Write_Data_L( uchar Data, uchar Cnt )    //Data 的低cnt位写入TM1621C，低位在前
{
    uchar i;

    for( i = 0; i < Cnt; i++ )
    {
        GPIOA_ResetBits( WRITE );
        if( Data & 0x01 )                           //从低位发送
            GPIOA_SetBits( DATA );
        else
            GPIOA_ResetBits( DATA );
        __asm__("nop");
        __asm__("nop");
        GPIOA_SetBits( WRITE );
        Data >>= 1;
    }
    GPIOA_ResetBits( WRITE );
    GPIOA_ResetBits( DATA );
}

/********************写入控制命令*************************/
static void WriteCmd( uchar Cmd )
{
    GPIOA_ResetBits( CS );
    __asm__("nop");
    Write_Data_H( 0x80, 4 );     //写入命令标志100
    Write_Data_H( Cmd, 8 );      //写入命令数据
    GPIOA_SetBits( CS );
    __asm__("nop");
}

/*********指定地址写入数据，实际写入后4位************/
static void WriteOneData_8( uchar Addr, uchar Data )
{
    GPIOA_ResetBits( CS );
    Write_Data_H( 0xa0, 3 );     //写入数据标志101
    Write_Data_H( Addr << 2, 6 );  //写入地址数据
    Write_Data_L( Data, 8 );     //写入数据
    GPIOA_SetBits( CS );
    __asm__("nop");
}

static void WriteOneData_4( uchar Addr, uchar Data )
{
    GPIOA_ResetBits( CS );
    Write_Data_H( 0xa0, 3 );     //写入数据标志101
    Write_Data_H( Addr << 2, 6 );  //写入地址数据
    Write_Data_L( Data, 4 );     //写入数据
    GPIOA_SetBits( CS );
    __asm__("nop");
}

/*********连续写入方式，每次数据为8位，写入数据************/
static void WriteAllData( uchar Addr, const uchar *p, uchar cnt )
{
    uchar i;

    GPIOA_ResetBits( CS );
    Write_Data_H( 0xa0, 3 );
    Write_Data_H( Addr << 2, 6 );
    for( i = 0; i < cnt; i++ )
    {
        Write_Data_L( *p, 8 );
        p++;
    }
    GPIOA_SetBits( CS );
    __asm__("nop");
}

static void lcd_flush( void )
{
    WriteAllData( 0, gs_lcd_ram, sizeof(gs_lcd_ram) );
}

static void lcd_blank(void)
{
    memset( gs_lcd_ram, 0x00, sizeof(gs_lcd_ram) );
}

static void lcd_full(void)
{
    memset( gs_lcd_ram, 0xFF, sizeof(gs_lcd_ram) );
}

static void lcd_xdigital(uchar pos, uchar xd)
{
    uchar addr1, addr2;

    addr1 = xdigital_addr[(pos - 1) * 2];
    addr2 = xdigital_addr[(pos - 1) * 2 + 1];

    if(addr1 % 2)
        gs_lcd_ram[addr1 >> 1] |= ((xdigital_data[xd] & 0x0F) << 4);
    else
        gs_lcd_ram[addr1 >> 1] |= (xdigital_data[xd] & 0x0F);

    if(addr2 % 2)
        gs_lcd_ram[addr2 >> 1] |= (xdigital_data[xd] & 0xF0);
    else
        gs_lcd_ram[addr2 >> 1] |= ((xdigital_data[xd] & 0xF0) >> 4);

    return;
}

static void lcd_flag(uint flags)
{
    uchar i;

    for(i = 0; i < sizeof(flag_data); i++) {
        if(flags & (1 << i)) {
            uchar addr = flag_addr[i];

            if(addr % 2)
                gs_lcd_ram[addr >> 1] |= ((flag_data[i] << 4) & 0xF0);
            else
                gs_lcd_ram[addr >> 1] |= (flag_data[i] & 0x0F);
        }
    }
}

static void lcd_xdigitals(
    uchar *xdigitals, uchar len, uchar pos,
    uint flags, uchar neg)
{
    uchar i;
    uchar cur_pos;

    if(!xdigitals || pos < 1 || pos > 6)
        return;

    if(neg) {
        if((pos + len) > 6)
            return;
    }
    else {
        if((pos + len - 1) > 6)
            return;
    }

    cur_pos = pos;

    if(neg) {
        lcd_xdigital(cur_pos++, 16);
    }

    for (i = 0; i < len; i++)
    {
        lcd_xdigital(cur_pos++, xdigitals[i]);
    }

    if(flags) {
        lcd_flag(flags);
    }

    return;
}

void lcd_disp_blank( void )
{
    lcd_blank();
    lcd_flush();
}

void lcd_disp_full( void )
{
    lcd_full();
    lcd_flush();
}

void lcd_disp_xdigitals(
    uchar *xdigitals, uchar len, uchar pos,
    uint flags, uchar neg)
{
    uchar i;
    uchar cur_pos;

    if(!xdigitals || pos < 1 || pos > 6)
        return;

    if(neg) {
        if((pos + len) > 6)
            return;
    }
    else {
        if((pos + len - 1) > 6)
            return;
    }

    lcd_blank();

    cur_pos = pos;

    if(neg) {
        lcd_xdigital(cur_pos++, 16);
    }

    for (i = 0; i < len; i++)
    {
        lcd_xdigital(cur_pos++, xdigitals[i]);
    }

    if(flags) {
        lcd_flag(flags);
    }

    lcd_flush();

    return;
}

void lcd_disp_num(int num, uchar dot_size, uint flags, uchar base)
{
    uchar neg = 0;
    uchar i;
    uchar count;
    uchar int_ct;
    uchar xdigitals[6];

    if(base != 10 && base != 16)
        return;

    if(dot_size > 2)
        return;

    count = 6;
    int_ct = 6;
    if(num < 0) {
        neg = 1;
        num *= (-1);
    }

    if(dot_size == 0) {
        if(neg) {
            count = 5;
            int_ct = 5;
        }
        else {
            count = 6;
            int_ct = 6;
        }
    }
    else if(dot_size == 1) {
        if(neg) {
            count = 5;
            int_ct = 4;
        }
        else {
            count = 6;
            int_ct = 5;
        }
    }

    for(i = 0; i < count; i++) {
        xdigitals[count - i - 1] = num % base;
        num = num / base;
    }

    for(i = 0; i < int_ct; i++) {
        if(xdigitals[i])
            break;
    }

    if(i == int_ct) {
        i = 1;
        xdigitals[0] = 0;
    }
    else if(i != 0) {
        memcpy(&xdigitals[0], &xdigitals[i], int_ct - i);
        i = int_ct - i;
    }
    else {
        i = int_ct;
    }

    if(dot_size) {
        lcd_blank();

        lcd_xdigitals(&xdigitals[0], i,
            int_ct - i + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), neg);
        lcd_xdigitals(&xdigitals[int_ct], dot_size, 6, LCD_F_DOT, 0);
        lcd_flush();
    }
    else {
        lcd_blank();
        lcd_xdigitals(&xdigitals[0], i,
            (int_ct - i) / 2 + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), neg);
        lcd_flush();
    }

    return;
}

void lcd_disp_num_fix(unsigned int num, uchar size, uint flags, uchar base)
{
    uchar i;
    uchar xdigitals[6];

    if(base != 10 && base != 16)
        return;

    if(size > 6)
        return;

    for(i = 0; i < size; i++) {
        xdigitals[size - i - 1] = num % base;
        num = num / base;
    }

    lcd_blank();
    lcd_xdigitals(&xdigitals[0], size,
        (6 - size) / 2 + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), 0);
    lcd_flush();

    return;
}

void lcd_disp_neg_num_fix(int num, uchar size, uint flags, uchar base)
{
    uchar i;
    uchar xdigitals[6];

    if(base != 10 && base != 16)
        return;

    if(size > 5 || num >= 0)
        return;

    num *= (-1);

    for(i = 0; i < size; i++) {
        xdigitals[size - i - 1] = num % base;
        num = num / base;
    }

    lcd_blank();
    lcd_xdigitals(&xdigitals[0], size,
        (5 - size) / 2 + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), 1);
    lcd_flush();

    return;
}

void lcd_disp_init(void)
{
    GPIOA_SetBits( CS );
    GPIOA_ModeCfg( CS, GPIO_ModeOut_PP_5mA );

    GPIOA_SetBits( WRITE );
    GPIOA_ModeCfg( WRITE, GPIO_ModeOut_PP_5mA );

    GPIOA_SetBits( DATA );
    GPIOA_ModeCfg( DATA, GPIO_ModeOut_PP_5mA );

    GPIOA_SetBits( CS );
    GPIOA_SetBits( WRITE );
    GPIOA_SetBits( DATA );

    WriteCmd( BIAS );         //1/3偏压 4公共口
    WriteCmd( RC );           //内部RC振荡
    WriteCmd( SYSDIS );       //关系统振荡器和LCD偏压发生器
    WriteCmd( SYSEN );        //打开系统振荡器
    WriteCmd( LCDON );        //开LCD偏压

    //TEST
    /*for(uchar i = 1; i <= 8; i++) {
        for(uchar j = 0; j <= 15; j++) {
            lcd_blank();
            lcd_xdigital(i, j);
            if((j % 2) == 0)
                lcd_flag(0xFFFF);
            lcd_flush();
        }
    }*/
}
#else
/******************TM1621C模块命令定义*********************/
#define SYSDIS   0x00    //关系统振荡器和LCD偏压发生器
#define SYSEN    0x02    //打开系统振荡器

#define LCDOFF   0x04     //关LCD偏压
#define LCDON    0x06     //开LCD偏压

#define RC       0x30     //内部RC振荡

#define BIAS     0x52     //1/3偏压 4公共口

/********************定义控制端口**********************/
#define CS      (GPIO_Pin_12)
#define WRITE   (GPIO_Pin_13)
#define DATA    (GPIO_Pin_14)

/********************定义数据*************************/
static uchar gs_lcd_ram[16];

//0~F字型码 负号码
const static uchar xdigital_data[] = {
    0x7D, 0x60, 0x3E, 0x7A, 0x63, 0x5B, 0x5F, 0x70,
    0x7F, 0x7B, 0x77, 0x4F, 0x1D, 0x6E, 0x1F, 0x17,
    0x02, 0x37, 0x05, 0x6D,
};

const static uchar xdigital_addr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E,
};

const static uchar flag_data[] = {
    0x08, 0x01, 0x04, 0x08, 0x01, 0x08, 0x08, 0x02, 0x04, 0x08, 0x08,
};

const static uchar flag_addr[] = {
    0x12, 0x1D, 0x1D, 0x1D, 0x1E, 0x1A, 0x1C, 0x1E, 0x1E, 0x01, 0x03,
};


/********************从高位写入数据*************************/
static void Write_Data_H( uchar Data, uchar Cnt )     //Data的高cnt位写入TM1621C，高位在前
{
    uchar i;

    for( i = 0; i < Cnt; i++ )
    {
        GPIOA_ResetBits( WRITE );
        if( Data & 0x80 )                               //从最高位发送
            GPIOA_SetBits( DATA );
        else
            GPIOA_ResetBits( DATA );
        __asm__("nop");
        __asm__("nop");
        GPIOA_SetBits( WRITE );
        Data <<= 1;
    }
    GPIOA_ResetBits( WRITE );
    GPIOA_ResetBits( DATA );
}

/********************从低位写入数据*************************/
static void Write_Data_L( uchar Data, uchar Cnt )    //Data 的低cnt位写入TM1621C，低位在前
{
    uchar i;

    for( i = 0; i < Cnt; i++ )
    {
        GPIOA_ResetBits( WRITE );
        if( Data & 0x01 )                           //从低位发送
            GPIOA_SetBits( DATA );
        else
            GPIOA_ResetBits( DATA );
        __asm__("nop");
        __asm__("nop");
        GPIOA_SetBits( WRITE );
        Data >>= 1;
    }
    GPIOA_ResetBits( WRITE );
    GPIOA_ResetBits( DATA );
}

/********************写入控制命令*************************/
static void WriteCmd( uchar Cmd )
{
    GPIOA_ResetBits( CS );
    __asm__("nop");
    Write_Data_H( 0x80, 4 );     //写入命令标志100
    Write_Data_H( Cmd, 8 );      //写入命令数据
    GPIOA_SetBits( CS );
    __asm__("nop");
}

/*********指定地址写入数据，实际写入后4位************/
static void WriteOneData_8( uchar Addr, uchar Data )
{
    GPIOA_ResetBits( CS );
    Write_Data_H( 0xa0, 3 );     //写入数据标志101
    Write_Data_H( Addr << 2, 6 );  //写入地址数据
    Write_Data_L( Data, 8 );     //写入数据
    GPIOA_SetBits( CS );
    __asm__("nop");
}

static void WriteOneData_4( uchar Addr, uchar Data )
{
    GPIOA_ResetBits( CS );
    Write_Data_H( 0xa0, 3 );     //写入数据标志101
    Write_Data_H( Addr << 2, 6 );  //写入地址数据
    Write_Data_L( Data, 4 );     //写入数据
    GPIOA_SetBits( CS );
    __asm__("nop");
}

/*********连续写入方式，每次数据为8位，写入数据************/
static void WriteAllData( uchar Addr, const uchar *p, uchar cnt )
{
    uchar i;

    GPIOA_ResetBits( CS );
    Write_Data_H( 0xa0, 3 );
    Write_Data_H( Addr << 2, 6 );
    for( i = 0; i < cnt; i++ )
    {
        Write_Data_L( *p, 8 );
        p++;
    }
    GPIOA_SetBits( CS );
    __asm__("nop");
}

static void lcd_flush( void )
{
    WriteAllData( 0, gs_lcd_ram, sizeof(gs_lcd_ram) );
}

static void lcd_blank(void)
{
    memset( gs_lcd_ram, 0x00, sizeof(gs_lcd_ram) );
}

static void lcd_full(void)
{
    memset( gs_lcd_ram, 0xFF, sizeof(gs_lcd_ram) );
}

static void lcd_xdigital(uchar pos, uchar xd)
{
    uchar addr1, addr2;

    addr1 = xdigital_addr[(pos - 1) * 2];
    addr2 = xdigital_addr[(pos - 1) * 2 + 1];

    if(addr1 % 2)
        gs_lcd_ram[addr1 >> 1] |= ((xdigital_data[xd] & 0x0F) << 4);
    else
        gs_lcd_ram[addr1 >> 1] |= (xdigital_data[xd] & 0x0F);

    if(addr2 % 2)
        gs_lcd_ram[addr2 >> 1] |= (xdigital_data[xd] & 0xF0);
    else
        gs_lcd_ram[addr2 >> 1] |= ((xdigital_data[xd] & 0xF0) >> 4);

    return;
}

static void lcd_flag(uint flags)
{
    uchar i;

    if(flags & LCD_F_P) {
        lcd_xdigital(1, 17);
    }
    else if(flags & LCD_F_I) {
        lcd_xdigital(1, 18);
    }
    else if(flags & LCD_F_U) {
        lcd_xdigital(1, 19);
    }

    for(i = 0; i < sizeof(flag_data); i++) {
        if(flags & (1 << i)) {
            uchar addr = flag_addr[i];

            if(addr % 2)
                gs_lcd_ram[addr >> 1] |= ((flag_data[i] << 4) & 0xF0);
            else
                gs_lcd_ram[addr >> 1] |= (flag_data[i] & 0x0F);
        }
    }
}

static void lcd_xdigitals(
    uchar *xdigitals, uchar len, uchar pos,
    uint flags, uchar neg)
{
    uchar i;
    uchar cur_pos;

    if(!xdigitals || pos < 1 || pos > 8)
        return;

    if(neg) {
        if((pos + len) > 8)
            return;
    }
    else {
        if((pos + len - 1) > 8)
            return;
    }

    cur_pos = pos;

    if(neg) {
        lcd_xdigital(cur_pos++, 16);
    }

    for (i = 0; i < len; i++)
    {
        lcd_xdigital(cur_pos++, xdigitals[i]);
    }

    if(flags) {
        lcd_flag(flags);
    }

    return;
}

void lcd_disp_blank( void )
{
    lcd_blank();
    lcd_flush();
}

void lcd_disp_full( void )
{
    lcd_full();
    lcd_flush();
}

void lcd_disp_xdigitals(
    uchar *xdigitals, uchar len, uchar pos,
    uint flags, uchar neg)
{
    uchar i;
    uchar cur_pos;

    if(!xdigitals || pos < 1 || pos > 8)
        return;

    if(neg) {
        if((pos + len) > 8)
            return;
    }
    else {
        if((pos + len - 1) > 8)
            return;
    }

    lcd_blank();

    cur_pos = pos;

    if(neg) {
        lcd_xdigital(cur_pos++, 16);
    }

    for (i = 0; i < len; i++)
    {
        lcd_xdigital(cur_pos++, xdigitals[i]);
    }

    if(flags) {
        lcd_flag(flags);
    }

    lcd_flush();

    return;
}

void lcd_disp_num(int num, uchar dot_size, uint flags, uchar base)
{
    uchar neg = 0;
    uchar i;
    uchar count;
    uchar int_ct;
    uchar xdigitals[8];

    if(base != 10 && base != 16)
        return;

    if(dot_size > 2)
        return;

    count = 8;
    int_ct = 8;
    if(num < 0) {
        neg = 1;
        num *= (-1);
    }

    if(dot_size == 0) {
        if(neg) {
            count = 7;
            int_ct = 7;
        }
        else {
            count = 8;
            int_ct = 8;
        }
    }
    else if(dot_size == 1) {
        if(neg) {
            count = 6;
            int_ct = 5;
        }
        else {
            count = 7;
            int_ct = 6;
        }
    }
    else if(dot_size == 2) {
        if(neg) {
            count = 7;
            int_ct = 5;
        }
        else {
            count = 8;
            int_ct = 6;
        }
    }

    for(i = 0; i < count; i++) {
        xdigitals[count - i - 1] = num % base;
        num = num / base;
    }

    for(i = 0; i < int_ct; i++) {
        if(xdigitals[i])
            break;
    }

    if(i == int_ct) {
        i = 1;
        xdigitals[0] = 0;
    }
    else if(i != 0) {
        memcpy(&xdigitals[0], &xdigitals[i], int_ct - i);
        i = int_ct - i;
    }
    else {
        i = int_ct;
    }

    if(dot_size) {
        lcd_blank();

        lcd_xdigitals(&xdigitals[0], i,
            int_ct - i + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), neg);
        lcd_xdigitals(&xdigitals[int_ct], dot_size, 7, LCD_F_DOT, 0);
        lcd_flush();
    }
    else {
        lcd_blank();
        lcd_xdigitals(&xdigitals[0], i,
            (int_ct - i) / 2 + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), neg);
        lcd_flush();
    }

    return;
}

void lcd_disp_num_fix(unsigned int num, uchar size, uint flags, uchar base)
{
    uchar i;
    uchar xdigitals[8];

    if(base != 10 && base != 16)
        return;

    if(size >= 8)
        return;

    for(i = 0; i < size; i++) {
        xdigitals[size - i - 1] = num % base;
        num = num / base;
    }

    lcd_blank();
    lcd_xdigitals(&xdigitals[0], size,
        (8 - size) / 2 + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), 0);
    lcd_flush();

    return;
}

void lcd_disp_neg_num_fix(int num, uchar size, uint flags, uchar base)
{
    uchar i;
    uchar xdigitals[8];

    if(base != 10 && base != 16)
        return;

    if(size >= 7 || num >= 0)
        return;

    num *= (-1);

    for(i = 0; i < size; i++) {
        xdigitals[size - i - 1] = num % base;
        num = num / base;
    }

    lcd_blank();
    lcd_xdigitals(&xdigitals[0], size,
        (7 - size) / 2 + 1, flags & ((~LCD_F_DOT) & 0xFFFFFFFF), 1);
    lcd_flush();

    return;
}

void lcd_disp_init(void)
{
    GPIOA_SetBits( CS );
    GPIOA_ModeCfg( CS, GPIO_ModeOut_PP_5mA );

    GPIOA_SetBits( WRITE );
    GPIOA_ModeCfg( WRITE, GPIO_ModeOut_PP_5mA );

    GPIOA_SetBits( DATA );
    GPIOA_ModeCfg( DATA, GPIO_ModeOut_PP_5mA );

    GPIOA_SetBits( CS );
    GPIOA_SetBits( WRITE );
    GPIOA_SetBits( DATA );

    WriteCmd( BIAS );         //1/3偏压 4公共口
    WriteCmd( RC );           //内部RC振荡
    WriteCmd( SYSDIS );       //关系统振荡器和LCD偏压发生器
    WriteCmd( SYSEN );        //打开系统振荡器
    WriteCmd( LCDON );        //开LCD偏压

    //TEST
    /*for(uchar i = 1; i <= 8; i++) {
        for(uchar j = 0; j <= 15; j++) {
            lcd_blank();
            lcd_xdigital(i, j);
            if((j % 2) == 0)
                lcd_flag(0xFFFF);
            lcd_flush();
        }
    }*/
}
#endif
