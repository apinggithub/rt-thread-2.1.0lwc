#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HWTIM_CH_TYPE_BASE             0x00
#define HWTIM_CH_TYPE_PWM              0x01   
#define HWTIM_CH_TYPE_IC               0x02  
#define HWTIM_CH_TYPE_OC               0x03  
#define HWTIM_CH_TYPE_ENCODER          0x04 
    

#define HWTIM_CHANNEL1              (uint8_t)TIM_CHANNEL_1  
#define HWTIM_CHANNEL2              (uint8_t)TIM_CHANNEL_2
#define HWTIM_CHANNEL3              (uint8_t)TIM_CHANNEL_3  
#define HWTIM_CHANNEL4              (uint8_t)TIM_CHANNEL_4
    
    
#define TIM_CH1_PULSE            100   // 定时器通道4占空比为：TIM_CH1_PULSE/TIM_PERIOD*100%=10/500*100%=20%  
#define TIM_CH2_PULSE            300   // 定时器通道3占空比为：TIM_CH2_PULSE/TIM_PERIOD*100%=300/1000*100%=30% 
#define TIM_CH3_PULSE            600   // 定时器通道2占空比为：TIM_CH3_PULSE/TIM_PERIOD*100%=600/1000*100%=60%    
#define TIM_CH4_PULSE            900   // 定时器通道1占空比为：TIM_CH4_PULSE/TIM_PERIOD*100%=900/1000*100%=90%


#ifdef RT_USING_HWTIM2  
#define GENERAL_TIM_RCC_CLK_ENABLE()        __HAL_RCC_TIM2_CLK_ENABLE()
#define GENERAL_TIM_RCC_CLK_DISABLE()       __HAL_RCC_TIM2_CLK_DISABLE()
#define GENERAL_TIM_GPIO_RCC_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define GENERAL_TIM_CH1_PORT                GPIOA
#define GENERAL_TIM_CH1_PIN                 GPIO_PIN_0
#define GENERAL_TIM_CH2_PORT                GPIOA
#define GENERAL_TIM_CH2_PIN                 GPIO_PIN_1
#define GENERAL_TIM_CH3_PORT                GPIOA
#define GENERAL_TIM_CH3_PIN                 GPIO_PIN_2
#define GENERAL_TIM_CH4_PORT                GPIOA
#define GENERAL_TIM_CH4_PIN                 GPIO_PIN_3    
#endif /*RT_USING_HWTIM2*/
    
    
    
/* STM32 timer driver */
typedef struct drv_hwtimer
{
    TIM_HandleTypeDef TimerHandle;
    IRQn_Type irq;
}drv_hwtimer_t;


#ifdef __cplusplus
}
#endif

#endif

