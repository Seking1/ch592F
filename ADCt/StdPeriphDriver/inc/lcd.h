/*
 * lcd.h
 *
 *  Created on: 2023年2月9日
 *      Author: A
 */

#ifndef INC_LCD_H_

#define INC_LCD_H_

#define uchar unsigned char              //数据类型宏定义
#define uint unsigned int                //数据类型宏定义

#if (DB_TYPE == 1)
typedef enum{
    LCD_F_DOT               = 0x0001,
    LCD_F_GLOBAL            = 0x0002,
    LCD_F_REM               = 0x0004,
    LCD_F_POWER             = 0x0008,
    LCD_F_RELAY             = 0x0010,
    LCD_F_KW                = 0x0020,
    LCD_F_H                 = 0x0040,
    LCD_F_I                 = 0x0080,
    LCD_F_U                 = 0x0100,
}LCD_F_T;
#else
typedef enum{
    LCD_F_DOT               = 0x0001,
    LCD_F_GLOBAL            = 0x0002,
    LCD_F_REM               = 0x0004,
    LCD_F_ALARM             = 0x0008,
    LCD_F_POWER             = 0x0010,
    LCD_F_RELAY             = 0x0020,
    LCD_F_PULSE             = 0x0040,
    LCD_F_KW                = 0x0080,
    LCD_F_H                 = 0x0100,
    LCD_F_METERNUM          = 0x0200,
    LCD_F_METERC            = 0x0400,
    LCD_F_P                 = 0x0800,
    LCD_F_I                 = 0x1000,
    LCD_F_U                 = 0x2000,
}LCD_F_T;
#endif

void lcd_disp_blank( void );
void lcd_disp_full( void );
void lcd_disp_xdigitals(uchar *xdigitals, uchar len, uchar pos, uint flags, uchar neg);
void lcd_disp_init(void);
void lcd_disp_num(int num, uchar dot_size, uint flags, uchar base);
void lcd_disp_num_fix(unsigned int num, uchar size, uint flags, uchar base);
void lcd_disp_neg_num_fix(int num, uchar size, uint flags, uchar base);
#endif /* INC_LCD_H_ */
