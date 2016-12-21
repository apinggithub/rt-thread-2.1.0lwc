/*
 * File      : gpio.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-01-05     Bernard      the first version
 */
#ifndef DRV_XT8XXP8_H__
#define DRV_XT8XXP8_H__
 
#include <stdint.h> 
#include <drivers/xt8xxp8.h>

#define XTP_DAT_H()  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET)                                             
#define XTP_DAT_L()  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_RESET)  

int stm32_xtp_init(void);

#endif
