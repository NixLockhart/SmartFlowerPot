#include "bump.h"

/**
 ****************************************************************************************************
 * @file        bump.c
 * @author      NixStudio(NixLockhart)
 * @version     V1.0
 * @date        2025-12-13
 * @brief       水泵风扇模块实现
 ****************************************************************************************************
 * @attention
 *
 * 平台:正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */


/**
 * @brief 水泵初始化
 */
void BUMP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	BUMP_OFF;
}

/**
 * @brief 风扇初始化
 */
void FUN_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 						 		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	FUN_OFF;
}
