#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRV_TIM_CH_TYPE_BASE             0x00
#define DRV_TIM_CH_TYPE_PWM              0x01   
#define DRV_TIM_CH_TYPE_IC               0x02  
#define DRV_TIM_CH_TYPE_OC               0x03  
#define DRV_TIM_CH_TYPE_ENCODER          0x04 
    

//#define HWTIM_CHANNEL1              (uint8_t)TIM_CHANNEL_1  
//#define HWTIM_CHANNEL2              (uint8_t)TIM_CHANNEL_2
//#define HWTIM_CHANNEL3              (uint8_t)TIM_CHANNEL_3  
//#define HWTIM_CHANNEL4              (uint8_t)TIM_CHANNEL_4
    
        
    
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

