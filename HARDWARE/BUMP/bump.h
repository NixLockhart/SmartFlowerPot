/**
 ****************************************************************************************************
 * @file        bump.h
 * @author      NixStudio(NixLockhart)
 * @version     V1.3
 * @date        2025-12-13
 * @brief       水泵风扇模块
 ****************************************************************************************************
 * @attention
 *
 * 平台:正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */

#ifndef __BUMP_H
#define	__BUMP_H
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

#define BUMP_ON 		GPIO_SetBits(GPIOA, GPIO_Pin_7)
#define BUMP_OFF 		GPIO_ResetBits(GPIOA, GPIO_Pin_7)

#define FUN_ON			GPIO_SetBits(GPIOA , GPIO_Pin_6)
#define FUN_OFF			GPIO_ResetBits(GPIOA , GPIO_Pin_6)

// 函数声明
void BUMP_Init(void);
void FUN_Init(void);

#endif



