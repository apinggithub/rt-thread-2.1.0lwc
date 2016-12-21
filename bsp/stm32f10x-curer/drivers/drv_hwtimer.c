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
static void timer_init(rt_device_hwtimer_t *timer, rt_uint32_t state)
{   
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    TIM_MasterConfigTypeDef sMasterConfig;   
    
    RT_ASSERT(timer != RT_NULL);
    
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
		        
        /*TIM6 and TIM7 are base timer. TIM2,TIM3,TIM4 and TIM5 are general timer , TIM1 and TIM8 are advance timer*/
        if((TIM6 != hwtim->TimerHandle.Instance)&&(TIM7 != hwtim->TimerHandle.Instance))
        {
                        
            //TIM_OC_InitTypeDef sConfigIC;                      
            if((TIM2 == hwtim->TimerHandle.Instance)
                ||(TIM3 == hwtim->TimerHandle.Instance)
                ||(TIM4 == hwtim->TimerHandle.Instance)
                ||(TIM5 == hwtim->TimerHandle.Instance))
            {
                
                
                TIM_OC_InitTypeDef sConfigOC;
                TIM_ClockConfigTypeDef sClockSourceConfig;
                
                sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
                HAL_TIM_ConfigClockSource(&(hwtim->TimerHandle), &sClockSourceConfig);
                
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE; 
                                               
                for(uint8_t i = 0;i < 4; i++)
                {                   
                    switch(timer->channel_no[i])                        
                    {
                        case HWTIM_CHANNEL1:
                        {
                            switch(timer->channel_type[i])
                            {
                                case HWTIM_CH_TYPE_PWM:  
                                {
                                    if (HAL_TIM_PWM_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    }                                     
                                    sConfigOC.OCMode = TIM_OCMODE_PWM1;
                                    sConfigOC.Pulse = TIM_CH1_PULSE;                                    
                                    HAL_TIM_PWM_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_1);     
                                }            
                                break;
                                case HWTIM_CH_TYPE_OC:
                                {
                                    if (HAL_TIM_OC_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    }    
                                    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
                                    HAL_TIM_OC_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_1);   
                                }                                    
                                break;
                                case HWTIM_CH_TYPE_IC:  
                                {
                                    
                                }                                    
                                break; 
                                case HWTIM_CH_TYPE_ENCODER:  
                                {
                                    
                                }                                    
                                break; 
                                default: /*default HWTIMER_TYPE_BASE no output*/
                                    break;                                       
                            }  
                        }                            
                        break;
                        case HWTIM_CHANNEL2:
                        {
                            switch(timer->channel_type[i])
                            {
                                case HWTIM_CH_TYPE_PWM: 
                                {                                    
                                    if (HAL_TIM_PWM_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    } 
                                    sConfigOC.OCMode = TIM_OCMODE_PWM1;
                                    sConfigOC.Pulse = TIM_CH1_PULSE;                                    
                                    HAL_TIM_PWM_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_2);   
                                }                                    
                                break;
                                case HWTIM_CH_TYPE_OC:
                                {
                                    if (HAL_TIM_OC_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    }   
                                    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
                                    HAL_TIM_OC_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_2);    
                                }
                                break;
                                case HWTIM_CH_TYPE_IC:  
                                {
                                    
                                }                                    
                                break; 
                                case HWTIM_CH_TYPE_ENCODER: 
                                {
                                    
                                }
                                break; 
                                default: /*default HWTIMER_TYPE_BASE no output*/
                                    break; 
                            } 
                        }                            
                        break;
                        case HWTIM_CHANNEL3:
                        {
                            switch(timer->channel_type[i])
                            {
                                case HWTIM_CH_TYPE_PWM: 
                                {                                    
                                    if (HAL_TIM_PWM_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    } 
                                    sConfigOC.OCMode = TIM_OCMODE_PWM1;
                                    sConfigOC.Pulse = TIM_CH1_PULSE;                                    
                                    HAL_TIM_PWM_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_3);   
                                }                                    
                                break;
                                case HWTIM_CH_TYPE_OC:
                                {
                                    if (HAL_TIM_OC_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    }    
                                    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
                                    HAL_TIM_OC_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_3);    
                                }
                                break;
                                case HWTIM_CH_TYPE_IC:        
                                {
                                    
                                }                                    
                                break; 
                                case HWTIM_CH_TYPE_ENCODER:  
                                {
                                    
                                }                                    
                                break; 
                                default: /*default HWTIMER_TYPE_BASE no output*/
                                    break; 
                            } 
                        }                            
                        break;
                        case HWTIM_CHANNEL4:
                        {
                            switch(timer->channel_type[i])
                            {
                                case HWTIM_CH_TYPE_PWM: 
                                {                                    
                                    if (HAL_TIM_PWM_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    } 
                                    sConfigOC.OCMode = TIM_OCMODE_PWM1;
                                    sConfigOC.Pulse = TIM_CH1_PULSE;                                    
                                    HAL_TIM_PWM_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_4);      
                                }                                    
                                break;
                                case HWTIM_CH_TYPE_OC:
                                {
                                    if (HAL_TIM_OC_Init(&(hwtim->TimerHandle)) != HAL_OK)
                                    {
                                        while(1);
                                    }     
                                    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
                                    HAL_TIM_OC_ConfigChannel(&(hwtim->TimerHandle), &sConfigOC, TIM_CHANNEL_4);    
                                }
                                break;
                                case HWTIM_CH_TYPE_IC:  
                                {
                                    
                                }                                    
                                break; 
                                case HWTIM_CH_TYPE_ENCODER:    
                                {
                                    
                                }                                    
                                break; 
                                default: /*default HWTIMER_TYPE_BASE no output*/
                                    break; 
                            } 
                        }                            
                        break;                        
                        default:break;
                    }/* end switch timer->channel */                                                           
                }/* end for() */
                HAL_TIM_MspPostInit(&(hwtim->TimerHandle));
            }
        }    
    }
    else
    {
        HAL_TIM_Base_MspDeInit(&(hwtim->TimerHandle));
    }
    
}

static rt_err_t timer_start(rt_device_hwtimer_t *timer, rt_off_t pos)//, rt_hwtimer_mode_t opmode
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    //uint16_t m;
    RT_ASSERT(timer != RT_NULL);
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    __HAL_TIM_SET_AUTORELOAD(&(hwtim->TimerHandle), timer->reload);
    
    //m = (opmode == HWTIMER_MODE_ONESHOT)? TIM_OPMODE_SINGLE : TIM_OPMODE_REPETITIVE;
    //TIM_SelectOnePulseMode(tim, m);
    
    switch(timer->channel_type[pos])
    {
        case HWTIM_CH_TYPE_PWM: 
        {
            HAL_TIM_PWM_Start(&(hwtim->TimerHandle),timer->channel_no[pos]);
        }
        break;
        case HWTIM_CH_TYPE_OC: 
        {
            HAL_TIM_OC_Start(&(hwtim->TimerHandle),timer->channel_no[pos]);
        }
        break;
        case HWTIM_CH_TYPE_IC: 
        {
            
        }
        break;
        case HWTIM_CH_TYPE_ENCODER: 
        {
            
        }
        break;
        default: /* default HWTIMER_TYPE_BASE on output */
        {
            HAL_TIM_Base_Start(&(hwtim->TimerHandle));
        }
        break;
    }           
    return RT_EOK;
}

static void timer_stop(rt_device_hwtimer_t *timer)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    HAL_TIM_Base_Stop(&(hwtim->TimerHandle));
}

static rt_err_t timer_ctrl(rt_device_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    
    rt_err_t err = RT_EOK;
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    assert_param(IS_TIM_INSTANCE(htim->Instance));
    hwtim->TimerHandle.Instance = tim;
    
    switch (cmd)
    {
        case HWTIMER_CTRL_SET_FREQ:
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

rt_err_t timer_set_prescaler(rt_device_hwtimer_t *timer,rt_uint32_t val)
{
    rt_err_t err = RT_EOK;
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    __HAL_TIM_SET_PRESCALER(&(hwtim->TimerHandle),val);    

   return err; 
}
static rt_uint32_t timer_get_counter(rt_device_hwtimer_t *timer)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);   
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    return __HAL_TIM_GET_COUNTER(&(hwtim->TimerHandle));
    
    
}
static rt_err_t timer_set_counter(rt_device_hwtimer_t *timer,rt_uint32_t val)
{
    rt_err_t err = RT_EOK;
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);
    
    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    __HAL_TIM_SET_COUNTER(&(hwtim->TimerHandle),val);
    
    return err;
}
static rt_uint32_t timer_get_autoreload(rt_device_hwtimer_t *timer)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);

    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    return __HAL_TIM_GET_AUTORELOAD(&(hwtim->TimerHandle));
}
static rt_err_t timer_set_autoreload(rt_device_hwtimer_t *timer,rt_uint32_t val)
{
    rt_err_t err = RT_EOK;
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);

    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    __HAL_TIM_SET_AUTORELOAD(&(hwtim->TimerHandle),val);
    
    return err;
}
static rt_uint32_t timer_get_compare(rt_device_hwtimer_t *timer,rt_uint8_t ch)
{
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);

    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    return __HAL_TIM_GET_COMPARE(&(hwtim->TimerHandle),ch);
}
static rt_err_t timer_set_compare(rt_device_hwtimer_t *timer,rt_uint8_t ch,rt_uint32_t val)
{
    rt_err_t err = RT_EOK;
    TIM_TypeDef *tim;
    drv_hwtimer_t *hwtim; 
    
    RT_ASSERT(timer != RT_NULL);

    hwtim = (drv_hwtimer_t *)timer->parent.user_data;
    tim = hwtim->TimerHandle.Instance;
    
    hwtim->TimerHandle.Instance = tim;
    
    __HAL_TIM_SET_COMPARE(&(hwtim->TimerHandle),ch,val);
    
    return err;
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
    timer_set_prescaler,
    timer_get_counter,
    timer_set_counter,
    timer_get_autoreload,
    timer_set_autoreload,
    timer_get_compare,
    timer_set_compare,
    timer_ctrl,
};

#ifdef RT_USING_HWTIM6
static drv_hwtimer_t hwtimer6;
static rt_device_hwtimer_t rttimer6;
rt_uint8_t led3sw = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    rt_pin_write(20, led3sw);
    led3sw = (~led3sw)&0x01;
    rt_device_hwtimer_isr(&rttimer6);
}
void TIM6_IRQHandler(void)
{ 
    HAL_TIM_IRQHandler(&(hwtimer6.TimerHandle));       
}
#endif /*RT_USING_HWTIM6*/

#ifdef RT_USING_HWTIM2
static drv_hwtimer_t hwtimer2;
static rt_device_hwtimer_t rttimer2;
void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&(hwtimer2.TimerHandle));
}
#endif /*RT_USING_HWTIM2*/

#ifdef RT_USING_HWTIM3
static drv_hwtimer_t hwtimer3;
static rt_device_hwtimer_t rttimer3;
void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&(hwtimer3.TimerHandle));
}
#endif /*RT_USING_HWTIM3*/

#ifdef RT_USING_HWTIM4
static drv_hwtimer_t hwtimer4;
static rt_device_hwtimer_t rttimer4;
void TIM4_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&(hwtimer4.TimerHandle));
}
#endif /*RT_USING_HWTIM4*/

int stm32_hwtimer_init(void)
{   
#ifdef RT_USING_HWTIM6      
    rttimer6.info = &_info;
    rttimer6.ops  = &_ops;

    hwtimer6.TimerHandle.Instance = TIM6;       
    rt_device_hwtimer_register(&rttimer6, "timer6", &hwtimer6);    
#endif /*RT_USING_HWTIM6*/
    
#ifdef RT_USING_HWTIM2       
    rttimer2.info = &_info;
    rttimer2.ops  = &_ops;
    
    rttimer2.channel_no[HWTIMER_CH1] = HWTIM_CHANNEL1;
    rttimer2.channel_no[HWTIMER_CH2] = HWTIM_CHANNEL2;
    rttimer2.channel_no[HWTIMER_CH3] = HWTIM_CHANNEL3;
    rttimer2.channel_no[HWTIMER_CH4] = HWTIM_CHANNEL4;
    rttimer2.channel_type[HWTIMER_CH1] = HWTIM_CH_TYPE_PWM;
    rttimer2.channel_type[HWTIMER_CH2] = HWTIM_CH_TYPE_OC;
    rttimer2.channel_type[HWTIMER_CH3] = HWTIM_CH_TYPE_PWM;
    rttimer2.channel_type[HWTIMER_CH4] = HWTIM_CH_TYPE_OC;
    rttimer2.pwm_duty_cycle[HWTIMER_CH1] = 100; //20% 100/500
    rttimer2.pwm_duty_cycle[HWTIMER_CH3] = 100;    
    
    hwtimer2.TimerHandle.Instance = TIM2;       
    rt_device_hwtimer_register(&rttimer2, "timer2", &hwtimer2);    
#endif /*RT_USING_HWTIM2*/    
    
#ifdef RT_USING_HWTIM3       
    rttimer3.info = &_info;
    rttimer3.ops  = &_ops;
    
    rttimer3.channel_no[HWTIMER_CH3] = HWTIM_CHANNEL3;   
    rttimer3.channel_type[HWTIMER_CH3] = HWTIM_CH_TYPE_PWM;   
    rttimer3.pwm_duty_cycle[HWTIMER_CH3] = 100; //20% = 100/500
     
    hwtimer3.TimerHandle.Instance = TIM3;       
    rt_device_hwtimer_register(&rttimer3, "timer3", &hwtimer3);    
#endif /*RT_USING_HWTIM3*/   

#ifdef RT_USING_HWTIM4       
    rttimer4.info = &_info;
    rttimer4.ops  = &_ops;
    
    rttimer4.channel_no[HWTIMER_CH1] = HWTIM_CHANNEL1;
    rttimer4.channel_no[HWTIMER_CH2] = HWTIM_CHANNEL2;
    rttimer4.channel_no[HWTIMER_CH3] = HWTIM_CHANNEL3;
    rttimer4.channel_no[HWTIMER_CH4] = HWTIM_CHANNEL4;
    rttimer4.channel_type[HWTIMER_CH1] = HWTIM_CH_TYPE_PWM;
    rttimer4.channel_type[HWTIMER_CH2] = HWTIM_CH_TYPE_OC;
    rttimer4.channel_type[HWTIMER_CH3] = HWTIM_CH_TYPE_PWM;
    rttimer4.channel_type[HWTIMER_CH4] = HWTIM_CH_TYPE_OC;
    rttimer4.pwm_duty_cycle[HWTIMER_CH1] = 100; //20% 100/500
    rttimer4.pwm_duty_cycle[HWTIMER_CH3] = 100;    
    
    hwtimer4.TimerHandle.Instance = TIM4;       
    rt_device_hwtimer_register(&rttimer4, "timer4", &hwtimer4);    
#endif /*RT_USING_HWTIM4*/  
    
    return 0;
}

INIT_BOARD_EXPORT(stm32_hwtimer_init);
#endif /* RT_USING_HWTIMER */
