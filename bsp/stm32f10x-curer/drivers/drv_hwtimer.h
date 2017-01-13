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
    
#ifdef RT_USING_HWTIM3
//#define RT_USING_HWTIM3_CH1 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM3_CH1_PWM   0   /* select timer pwm mode */
#define RT_USING_HWTIM3_CH1_OCM   0   /* select timer compare out mode */
#define RT_USING_HWTIM3_CH1_IC    0   /* select timer intput capture mode */
#define RT_USING_HWTIM3_CH1_ENCODER 0 /* select timer encoder mode */

//#define RT_USING_HWTIM3_CH2 
#define RT_USING_HWTIM3_CH2_PWM   0   /*select timer pwm mode*/
#define RT_USING_HWTIM3_CH2_OCM   0   /*select timer compare out mode*/
#define RT_USING_HWTIM3_CH2_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM3_CH2_ENCODER 0 /*select timer encoder mode*/

#define RT_USING_HWTIM3_CH3 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM3_CH3_PWM   1   /*select timer pwm mode*/
#define RT_USING_HWTIM3_CH3_OCM   0   /*select timer compare out mode*/
#define RT_USING_HWTIM3_CH3_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM3_CH3_ENCODER   0 /*select timer encoder mode*/

#define RT_USING_HWTIM3_CH4 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM3_CH4_PWM   1   /*select timer pwm mode*/
#define RT_USING_HWTIM3_CH4_OCM   0   /*select timer compare out mode*/
#define RT_USING_HWTIM3_CH4_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM3_CH4_ENCODER 0 /*select timer encoder mode*/

#endif /* RT_USING_HWTIM3 */

#ifdef RT_USING_HWTIM4
#define RT_USING_HWTIM4_CH1 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM4_CH1_PWM   1   /*select timer pwm mode*/
#define RT_USING_HWTIM4_CH1_OCM   0   /*select timer compare out mode*/
#define RT_USING_HWTIM4_CH1_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM4_CH1_ENCODER   0 /*select timer encoder mode*/

#define RT_USING_HWTIM4_CH2 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM4_CH2_PWM   0   /*select timer pwm mode*/
#define RT_USING_HWTIM4_CH2_OCM   1   /*select timer compare out mode*/
#define RT_USING_HWTIM4_CH2_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM4_CH2_ENCODER   0 /*select timer encoder mode*/

#define RT_USING_HWTIM4_CH3 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM4_CH3_PWM   1   /*select timer pwm mode*/
#define RT_USING_HWTIM4_CH3_OCM   0   /*select timer compare out mode*/
#define RT_USING_HWTIM4_CH3_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM4_CH3_ENCODER 0 /*select timer encoder mode*/

#define RT_USING_HWTIM4_CH4 /* 0 ---> disable or 1 ---> enable blow for single choice */
#define RT_USING_HWTIM4_CH4_PWM   0   /*select timer pwm mode*/
#define RT_USING_HWTIM4_CH4_OCM   1   /*select timer compare out mode*/
#define RT_USING_HWTIM4_CH4_IC    0   /*select timer intput capture mode*/
#define RT_USING_HWTIM4_CH4_ENCODER 0 /*select timer encoder mode*/

#endif /* RT_USING_HWTIM4 */
    
    
    
    
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

