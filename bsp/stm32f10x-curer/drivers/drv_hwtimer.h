#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
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

