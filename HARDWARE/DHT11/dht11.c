/**
 ****************************************************************************************************
 * @file        dht11.c
 * @author      NixStudio(NixLockhart)
 * @version     V1.0
 * @date        2025-01-02
 * @brief       DHT11相关操作
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */


#include "dht11.h"
#include "delay.h"

 //发送复位信号给 DHT11，启动数据通信
void DHT11_Rst(void)
{
    DHT11_IO_OUT();     //SET OUTPUT
    DHT11_DQ_OUT(0);     //??DQ
    delay_ms(20);        //????18ms
    DHT11_DQ_OUT(1);     //DQ=1 
    delay_us(30);         //????20~40us
}

//检测 DHT11 的响应信号，判断其是否准备好通信
uint8_t DHT11_Check(void)
{
    uint8_t retry = 0;
    DHT11_IO_IN();//SET INPUT     
    while (DHT11_DQ_IN && retry < 100)//DHT11???40~80us
    {
        retry++;
        delay_us(1);
    };
    if (retry >= 100)return 1;
    else retry = 0;
    while (!DHT11_DQ_IN && retry < 100)//DHT11????????40~80us
    {
        retry++;
        delay_us(1);
    };
    if (retry >= 100)return 1;
    return 0;
}

//读取 DHT11 传输的 1 位数据
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry = 0;
    while (DHT11_DQ_IN && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    retry = 0;
    while (!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    delay_us(40);
    if (DHT11_DQ_IN)return 1;
    else return 0;
}

//读取 DHT11 传输的 1 字节（8 位）数据
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

// DHT11 的温湿度数据，并存储到指定变量中
uint8_t DHT11_Read_Data(uint8_t* temp, uint8_t* humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if (DHT11_Check() == 0)
    {
        for (i = 0; i < 5; i++)
        {
            buf[i] = DHT11_Read_Byte();
        }
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = buf[0];
            *temp = buf[2];
        }
    }
    else return 1;
    return 0;
}

//初始化 DHT11 的数据引脚和通信准备
uint8_t DHT11_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    GPIO_SetBits(GPIOG, GPIO_Pin_11);
    DHT11_Rst();
    return DHT11_Check();
}
