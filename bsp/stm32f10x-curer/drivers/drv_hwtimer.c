/*
 * File      : drv_hwtimer.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author           Notes
 * 2015-09-02     heyuanjie87      the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_hwtimer.h"
#include "tim.h"

#ifdef RT_USING_HWTIMER
static void timer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{   
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    TIM_MasterConfigTypeDef sMasterConfig;
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;        
  
    if (state == 1)
    {
              
        hwtim->TimerHandle.Instance = tim;//TIMx;
        hwtim->TimerHandle.Init.Prescaler = timer->prescaler;
        hwtim->TimerHandle.Init.Period = timer->reload;        
        hwtim->TimerHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
        hwtim->TimerHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        if (HAL_TIM_Base_Init(&(hwtim->TimerHandle)) != HAL_OK)
        {
            //Error_Handler();
            while(1);
        }

        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&(hwtim->TimerHandle), &sMasterConfig) != HAL_OK)
        {
            //Error_Handler();
            while(1);
        }
                          
		HAL_TIM_Base_MspInit(&(hwtim->TimerHandle));
    }
    else
    {
        HAL_TIM_Base_MspDeInit(&(hwtim->TimerHandle));
    }
    
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_hwtimer_mode_t opmode)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    //uint16_t m;
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    __HAL_TIM_SET_AUTORELOAD(&(hwtim->TimerHandle), timer->reload);
    
    //m = (opmode == HWTIMER_MODE_ONESHOT)? TIM_OPMODE_SINGLE : TIM_OPMODE_REPETITIVE;
    //TIM_SelectOnePulseMode(tim, m);
    switch(opmode)
    { 
    case HWTIMER_MODE_PERIOD:          
        HAL_TIM_Base_Start(&(hwtim->TimerHandle));
    break;
    case HWTIMER_MODE_ONESHOT:   
        HAL_TIM_Base_Start(&(hwtim->TimerHandle));        
    break;
    default:break;
    
    }    

    return RT_EOK;
}

static void timer_stop(rt_hwtimer_t *timer)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    HAL_TIM_Base_Stop(&(hwtim->TimerHandle));
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    
    rt_err_t err = RT_EOK;
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    assert_param(IS_TIM_INSTANCE(htim->Instance));
    hwtim->TimerHandle.Instance = tim;
    
    switch (cmd)
    {
    case HWTIMER_CTRL_FREQ_SET:
    {     
        //uint16_t timer_period;
        uint32_t freq,sysclk;
        
        /* T=(TIM_Period+1)*(TIM_Prescaler+1)/TIMxCLK 
        //T is timer cycles,The value of TIM_Period and TIM_Prescaler is (0-65535), TIMxCLK is system clock 72MHz
        //if T =    1 us (1MHz), TIM_Prescaler = 71 ,  and the TIM_Period = 0 < 65535 ok. 
        //if T =  100 us,(10KHz), TIM_Prescaler = 71 ,  and the TIM_Period = 99 < 65535 ok. 
        //if T =  500 us,(2KHz), TIM_Prescaler = 71 ,  and the TIM_Period = 499 < 65535 ok. 
        //if T =    1 ms,(1KHz), TIM_Prescaler = 71 ,  and the TIM_Period = 999 < 65535 ok. 
        //if T =   10 ms,(100Hz), TIM_Prescaler = 71 ,  and the TIM_Period = 9999 < 65535 ok. 
        //if T =   50 ms,(20Hz), TIM_Prescaler = 71 ,  and the TIM_Period = 49999 < 65535 ok. 
        //if T = 1 s, TIM_Prescaler = 7199, and the TIM_Period = 9999 < 65535 ok.
        */       
        sysclk = HAL_RCC_GetHCLKFreq();/* get system clock 72MHz */                        
        freq = *((rt_uint32_t*)arg);   /* the frequence to user set in order to calculate the timer reload value */   
        
        RT_ASSERT(timer->freq != 0);       
        timer->reload= sysclk/(timer->prescaler + 1)/freq - 1;
                
        __HAL_TIM_DISABLE_IT(&(hwtim->TimerHandle), TIM_IT_UPDATE);
        
        __HAL_TIM_SET_PRESCALER(&(hwtim->TimerHandle),timer->prescaler);
        
        __HAL_TIM_SET_AUTORELOAD(&(hwtim->TimerHandle), timer->reload);
        
        __HAL_TIM_CLEAR_IT(&(hwtim->TimerHandle),TIM_IT_UPDATE);
        
        __HAL_TIM_ENABLE_IT(&(hwtim->TimerHandle), TIM_IT_UPDATE);
    }
    break;
    default:
    {
        err = -RT_ENOSYS;
    }
    break;
    }

    return err;
}

static rt_uint32_t timer_counter_get(rt_hwtimer_t *timer)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 

    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    return __HAL_TIM_GET_COUNTER(&(hwtim->TimerHandle));
}

static const struct rt_hwtimer_info _info =
{
    1000000,           /* the maximum count frequency can be set */
    20,                /* the minimum count frequency can be set */
    0xFFFF,            /* the maximum counter value */
    HWTIMER_CNTMODE_UP,/* Increment or Decreasing count mode */
};

static const struct rt_hwtimer_ops _ops =
{
    timer_init,
    timer_start,
    timer_stop,
    timer_counter_get,
    timer_ctrl,
};

#ifdef RT_USING_HWTIM6

static drv_hwtimer_t hwtimer6;
static rt_hwtimer_t rttimer6;

rt_uint8_t led3sw = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    rt_pin_write(20, led3sw);
    led3sw = (~led3sw)&0x01;
    rt_device_hwtimer_isr(&rttimer6);
}
void TIM6_IRQHandler(void)
{
    drv_hwtimer_t *hwtim; 
    hwtim = &hwtimer6;
    
    HAL_TIM_IRQHandler(&(hwtim->TimerHandle));
       
}
#endif /*RT_USING_HWTIM6*/

int stm32_hwtimer_init(void)
{
    //drv_hwtimer_t *hwtim;    
#ifdef RT_USING_HWTIM6
       
    rttimer6.info = &_info;
    rttimer6.ops  = &_ops;
    
    //hwtim = &hwtimer6;
    //hwtim->TimerHandle.Instance = TIM6;
    hwtimer6.TimerHandle.Instance = TIM6;
       
    rt_device_hwtimer_register(&rttimer6, "timer6", &hwtimer6);
    
#endif /*RT_USING_HWTIM6*/
    return 0;
}

INIT_BOARD_EXPORT(stm32_hwtimer_init);
#endif
