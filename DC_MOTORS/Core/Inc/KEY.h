#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/* ↓↓↓ 换成你实际接按键的 4 个引脚，CubeMX 里要设成 GPIO_Input + Pull-up ↓↓↓ */
#define KEY1   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)
#define KEY2   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)
#define KEY3   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5)
#define KEY4   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6)

#define KEY1_PRES   1
#define KEY2_PRES   2
#define KEY3_PRES   3
#define KEY4_PRES   4

uint8_t KEY_Scan(uint8_t mode);  /* 少了这行 main.c 会报 implicit declaration */

#endif
