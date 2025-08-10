/*
 * oled_049.c
 *
 *  Created on: Feb 19, 2025
 *      Author: Aluizio d'Affonsêca Netto
 */



#include <ch32v00x.h>
#include "oled_fonte.h"
#include "oled_049.h"
//PC2 -> SCL
//PC1 -> SDA

//uint8_t ssd1306InitCommand[] = { 0x00, 0x20, 0x02, 0x21, 0x00, 0x7F, 0x22, 0x00,
//        0x07, 0x8D, 0x14, 0xAF };

uint8_t ssd1306InitCommand[27] = {
        (0x00), /*Start command */
        (0xAE), /* Entire Display OFF */
        (0xD5), /* Set Display Clock Divide Ratio and Oscillator Frequency */
        (0x80), /* Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended */
        (0xA8), /* Set Multiplex Ratio */
        (0x1F), /* 32 COM lines */
        (0xD3), /* Set display offset */
        (0x00), /* 0 offset */
        (0x40), /* Set first line as the start line of the display */
        (0x8D), /* Charge pump */
        (0x14), /* Enable charge dump during display on */
        (0x20), /* Set memory addressing mode */
        (0x00), /* Horizontal addressing mode */
        (0xA1), /* Set segment remap with column address 0 mapped to segment 0 */
        (0xC8), /* Set com output scan direction, scan from com63 to com 0 */
        (0xDA), /* Set com pins hardware configuration */
        (0x12), /* Alternative com pin configuration, disable com left/right remap */
        (0x81), /* Set contrast control */
        (0x7F), /* Set Contrast to 128 */
        (0xD9), /* Set pre-charge period */
        (0xF1), /* Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK */
        (0xDB), /* Set Vcomh deselect level */
        (0x20), /* Vcomh deselect level ~ 0.77 Vcc */
        (0xA4), /* Entire display ON, resume to RAM content display */
        (0xA6), /* Set Display in Normal Mode, 1 = ON, 0 = OFF */
        (0x2E), /* Deactivate scroll */
        (0xAF) /* Display on in normal mode */

};


void IIC_Init(u32 bound, u16 address) {
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    I2C_InitTypeDef I2C_InitTSturcture = { 0 };

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOC | RCC_PB2Periph_AFIO, ENABLE );
    RCC_PB1PeriphClockCmd( RCC_PB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //SCL
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //SDA
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure);

    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture);

    I2C_Cmd( I2C1, ENABLE);
    I2C_AcknowledgeConfig( I2C1, ENABLE);

}

static void OledI2CWriteData(uint8_t *d, int len) {

    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );
    I2C_GenerateSTART( I2C1, ENABLE);

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );
    I2C_Send7bitAddress( I2C1, ssd1306Address, I2C_Direction_Transmitter);

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    for (int i = 0; i < len; i++) {
        I2C_SendData( I2C1, d[i]);
        while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
    }
    I2C_GenerateSTOP( I2C1, ENABLE);

}

void ssd1306Init(void) {

    IIC_Init(100000,ssd1306Address);

    OledI2CWriteData(ssd1306InitCommand,27);
}


static void ssd1306SetXY(uint8_t col_start, uint8_t col_end,uint8_t page_start, uint8_t page_end) {
    uint8_t data[7];
    data[0] = (0x00); /* For Data Transmission, C = 0 and D/C = 0 */
    data[1] = (0x21); /* Set Column Start and End Address */
    data[2] = (col_start); /* Column Start Address col_start */
    data[3] = (col_end); /* Column End Address col_end */
    data[4] = (0x22); /* Set Page Start and End Address */
    data[5] = (page_start); /* Page Start Address page_start */
    data[6] = (page_end); /* Page End Address page_end */

    OledI2CWriteData(data,7);

}


void ssd1306Fill(uint8_t pattern) {
    uint8_t data[2];
    ssd1306SetXY(0x20,0x5f,0x00,0x03);
    data[0] = 0x40;
    data[1]=pattern;
    for(uint16_t segment = 0; segment < 256; segment++)
    {
        OledI2CWriteData(data,2); //Set all 8 pixels
    }


}

void ssd1306PrintChar(uint8_t page, uint8_t segment, uint8_t chr) {

    uint8_t data[6];
    ssd1306SetXY(segment+0x20,0x5f,page,0x03);
    data[0] = 0x40;
    for(uint8_t i = 0; i < 5 ; i++) //Read the 5 bytes from the font[][] array and send it to the display byte-by-byte
    {
        data[1+i] = font[chr-0x20][i];
        //{ 0x3c, 0x40, 0x30, 0x40, 0x3c }, // w
    }
    OledI2CWriteData(data,6);


}

void ssd1306Print(uint8_t page, uint8_t segment, uint8_t *str) {

    while(*str!=0) {

        ssd1306PrintChar(page, segment, *str);
        str++;
        segment += 6;
    }

}


void ssd1306PrintInt(uint8_t page, uint8_t segment,int32_t num) {
    int32_t i = 0;
    char temp[12];
    char str_out[16];

    char *ptr_out = str_out;

    if(num == 0) {
        *ptr_out++ = '0';
    }

    if(num < 0) {
        *ptr_out++ = '-';
        num = -num;
    }

    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    //envia carcateres na ordem invertida
    while (--i >= 0) {
        *ptr_out++ = temp[i];
    }
    *ptr_out = '\0';
    ssd1306Print(page, segment, str_out);
}

void ssd1306PrintFloat(uint8_t page, uint8_t segment,float v, int decimalDigits)
{
    int32_t i = 0;
    char temp[12];
    char str_out[16];
    char *ptr_out = str_out;
    int32_t kd = 1;
    int32_t intPart, fractPart,dec;
    dec = decimalDigits;
    for (;decimalDigits!=0; kd*=10, decimalDigits--);
    intPart = (int32_t)v;
    fractPart = (int32_t)((v-(float)(int32_t)v)*kd);
    if(fractPart < 0) fractPart *= -1;
    //printf("%d.%d", intPart, fractPart);

    if(intPart == 0) {
        *ptr_out++ = '0';
    }

    if(intPart < 0) {
        *ptr_out++ = '-';
        intPart = -intPart;
    }

    while (intPart > 0) {
        temp[i++] = (intPart % 10) + '0';
        intPart /= 10;
    }
    //envia carcateres na ordem invertida
    while (--i >= 0) {
        *ptr_out++ = temp[i];
    }

    *ptr_out++ = '.';
    if (fractPart > 0) {
        kd/=10;
        while(fractPart < kd){
            *ptr_out++ = '0';
            kd/=10;
        }
        i = 0;
        while (fractPart > 0) {
            temp[i++] = (fractPart % 10) + '0';
            fractPart /= 10;
        }
        //envia carcateres na ordem invertida
        while (--i >= 0) {
            *ptr_out++ = temp[i];
        }

    }
    else {
        for (i = 0; i < dec; i++)
            *ptr_out++ = '0';
    }
    *ptr_out = '\0';

    ssd1306Print(page, segment, str_out);


}





