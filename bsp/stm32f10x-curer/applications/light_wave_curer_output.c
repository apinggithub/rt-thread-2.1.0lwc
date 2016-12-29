/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>
#include <drivers/lcdht1621b.h>
#include <drv_lcdht1621b.h>
#include <drivers/hwtimer.h>

#include "drv_led.h"
#include "drv_gpio.h"
#include "drv_hwbutton.h"

#include "light_wave_curer.h"

static rt_err_t lwc_cure_output(rt_device_t dev, lwc_cure_t *lc);

const float frq[][6] = {
    {26.0,  17.0,   9.0,    5.0,    0.5,    4.0},
    {1.0,   0.5,    0.5,    2.5,    1.5,    5.0},
    {0.5,   1.5,    2.0,    6.5,    3.5,    2.5},
    {1.5,   5.0,    0.5,    0.5,    4.5,    2.0},
    {26.0,  13.0,   5.0,    4.0,    1.5,    50.0},
    {9.0,   4.5,    2.0,    5.5,    6.5,    1.5},
    {0.5,   0.75,   6.5,    1.5,    9.5,    1.5},
    {3.5,   0.5,    9.0,    2.6,    5.0,    1.3}
};

//static rt_sem_t sem = RT_NULL;

ALIGN(RT_ALIGN_SIZE)
rt_uint8_t lwc_output_stack[ 1024 ];
struct rt_thread lwc_output_thread;

static rt_err_t timer3_timeout_cb(rt_device_t dev, rt_size_t size)
{
    //rt_kprintf("HT %d\n", rt_tick_get());
    //lct.lreg.tval.cure_done = 1;  
    rt_event_send(&event, RT_EVENT_LWC_TIMER_FINISH_CLOSE);
    return 0;
}
static rt_err_t timer4_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("HT %d\n", rt_tick_get());    
    return 0;
}
void lwc_output_thread_entry(void* parameter)
{

    rt_err_t err;
    rt_device_t dev_hwtimer3 = RT_NULL;
    rt_device_t dev_hwtimer4 = RT_NULL;
    rt_device_hwtimer_t *timer3;
    rt_device_hwtimer_t *timer4;
    rt_hwtimer_chval_t hwc;
    //rt_hwtimer_tmr_t tmr;
    rt_uint32_t recv_event;
         
    if ((dev_hwtimer3 = rt_device_find(TIMER3)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER3);
        while(1);
    }
    timer3 = (rt_device_hwtimer_t *)dev_hwtimer3;  
    timer3->freq = 1200;
    timer3->prescaler = 71;
    timer3->reload = 833;
    //d = (fs/(f*p))-1 fs-->system frequency 72MHz,f --->timer output(interrupt),p-->prescaler    
    if (rt_device_open(dev_hwtimer3, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER3);
        while(1);
    }
    rt_device_set_rx_indicate(dev_hwtimer3, timer3_timeout_cb);
     /* set the frequency */
    hwc.value = timer3->freq;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_FREQ, &hwc);
    if (err != RT_EOK)
    {
        rt_kprintf("Set timer freq = %d Hz Fail! And close the %s\n" ,timer3->freq, TIMER3);
        err = rt_device_close(dev_hwtimer3);
        while(1);
    }
          
    /* set the duty cycles 50%*/
    /*hwt.count = timer3->reload/4;
    hwt.ch = HWTIMER_CH3;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt.ch);
        err = rt_device_close(dev_hwtimer3);
        while(1);
    }*/
  
#if 0
    if ((dev_hwtimer4 = rt_device_find(TIMER4)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER4);
        while(1);
    } 
    timer4 = (rt_device_hwtimer_t *)dev_hwtimer4;  
    timer4->freq = 1200;
    timer4->prescaler = 71;
    timer4->reload = 833;
    if (rt_device_open(dev_hwtimer4, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER4);
        while(1);
    }
    rt_device_set_rx_indicate(dev_hwtimer4, timer4_timeout_cb);
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_FREQ, &timer4->freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set timer freq = %d Hz Fail! And close the %s\n" ,timer4->freq, TIMER4);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }
    //hwt.count = timer3->reload/2;
    hwt.ch = HWTIMER_CH4;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt.ch);
        err = rt_device_close(dev_hwtimer3);
        while(1);
    }
    hwt.count = timer4->reload/2;
    hwt.ch = HWTIMER_CH1;
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt.ch);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }
    hwt.ch = HWTIMER_CH3;
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt.ch);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }
#endif
    
    #if 0
    /*set time lenght */                
    tmrval.sec = 30;//lcc->lreg.tval.tmr_value*60;
    if (rt_device_write(dev_hwtimer3, 2, &tmrval, sizeof(tmrval)) != sizeof(tmrval))
    {
        rt_kprintf("SetTime = %d Fail and close the %s\n" ,tmrval,TIMER3);
        err = rt_device_close(dev_hwtimer3);
        while(1);
    }
    rt_thread_delay( RT_TICK_PER_SECOND*5 );
    
    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_DUTY_CYCLE, &hwt);
    rt_kprintf(" default Get ch = %d pwm = %d \n", &hwt.ch, timer3->pwm_duty_cycle[hwt.ch]);
    hwt.count += 100;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt.ch);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }
    rt_thread_delay( RT_TICK_PER_SECOND*5 );    
    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_DUTY_CYCLE, &hwt);    
    rt_kprintf(" default Get ch = %d pwm = %d \n", &hwt.ch, timer3->pwm_duty_cycle[hwt.ch]);
    hwt.count += 100;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt.ch);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }
    /*if (rt_device_write(dev_hwtimer3, 2, &tmrval, sizeof(tmrval)) != sizeof(tmrval))
    {
        rt_kprintf("SetTime = %d Fail and close the %s\n" ,tmrval,TIMER3);
        err = rt_device_close(dev_hwtimer3);
        while(1);
    }*/
    
    rt_pin_mode(PD2_BEEP, PIN_MODE_OUTPUT);
    rt_pin_mode(PB5_IONTHERAPY_SW, PIN_MODE_OUTPUT);
    rt_pin_mode(PB12_IONTHERAPY_PWR, PIN_MODE_OUTPUT);
    rt_pin_mode(PB13_IONTHERAPY_CRL1, PIN_MODE_OUTPUT);
    rt_pin_mode(PB13_IONTHERAPY_CRL1, PIN_MODE_OUTPUT);
    rt_pin_mode(PB15_IONTHERAPY_DECT, PIN_MODE_INPUT);
    #endif
    
    while(1)
    {
        
        //rt_pin_write(19, PIN_HIGH);
        //rt_pin_write(19, PIN_LOW);	
        if (rt_event_recv(&event, (RT_EVENT_LWC_TIMER_FINISH_CLOSE
                                    |RT_EVENT_LWC_DEVICE_FORCE_CLOSE
                                    |RT_EVENT_LWC_BUTTON_UPDATE
                                    |RT_EVENT_LWC_LASER_CURE_CLOSE
                                    |RT_EVENT_LWC_HEAT_CURE_CLOSE
                                    ),
                           RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                           RT_TICK_PER_SECOND/100, &recv_event) == RT_EOK)
        {
            switch(recv_event)
            {
                case RT_EVENT_LWC_TIMER_FINISH_CLOSE:
                case RT_EVENT_LWC_DEVICE_FORCE_CLOSE:    
                {
                    hwc.ch = TMR_CH_LASER_CURE;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                    hwc.ch = TMR_CH_HEAT_CURE;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                    
                    rt_event_send(&event, RT_EVENT_LWC_DEVICE_POWER_CLOSE);
                }
                break;
                case RT_EVENT_LWC_BUTTON_UPDATE:
                {
                    lwc_cure_output(dev_hwtimer3, (lwc_cure_t *)&lct);
                }
                break;
                case RT_EVENT_LWC_LASER_CURE_CLOSE:
                {
                    hwc.ch = TMR_CH_LASER_CURE;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                }
                break;
                case RT_EVENT_LWC_HEAT_CURE_CLOSE:
                {
                    hwc.ch = TMR_CH_HEAT_CURE;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                }
                break;
                default:
                break;
            }           
            
        }
                             
        rt_thread_delay( RT_TICK_PER_SECOND/10 );
    }
}

static rt_err_t lwc_cure_output(rt_device_t dev, lwc_cure_t *lc)
{
    rt_err_t err = RT_EOK;
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(lc != RT_NULL);   
    rt_hwtimer_chval_t hwc;   
    
    if(LWC_ACTIVED == lc->lway[LASER_CURE].status)
    {
        if(1 == lc->lreg.tval.tmr_lock)
        { 
            if(LWC_INACTIVE == lc->lcf[LASER_CURE].cure_out_actived)
            {                
                lc->lcf[LASER_CURE].cure_out_actived = LWC_ACTIVED;
                                             
            }            
            if(1 == lc->lreg.btn.button_jg)/* low */
            {
                /* get the reload of the timer */
                hwc.ch = TMR_CH_LASER_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;               
                hwc.value = tval*1/3;
                hwc.ch = TMR_CH_LASER_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwc);
                if (err != RT_EOK)
                {
                    rt_kprintf("Set ch  = %d pwm Fail\n", hwc.ch);
                    return err;
                }
                else
                {
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                }
                hwc.ch = TMR_CH_LASER_CURE;
                rt_device_control(dev, HWTIMER_CTRL_START, &hwc);
            }
            else if(2 == lc->lreg.btn.button_jg) /* middle */
            {
                hwc.ch = TMR_CH_LASER_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*2/3;
                hwc.ch = TMR_CH_LASER_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwc);
                if (err != RT_EOK)
                {
                    rt_kprintf("Set ch  = %d pwm Fail\n", hwc.ch);
                    return err;
                } 
                else
                {
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                }
            }
            else if(3 == lc->lreg.btn.button_jg) /* hight */
            {
                hwc.ch = TMR_CH_LASER_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*3/3;
                hwc.ch = TMR_CH_LASER_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwc);
                if (err != RT_EOK)
                {
                    rt_kprintf("Set ch  = %d pwm Fail\n", hwc.ch);
                    return err;
                } 
                else
                {
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                }
            }                                               
        }      
        
    }
    
    if(LWC_ACTIVED == lc->lway[HEAT_CURE].status)
    {
        if(1 == lc->lreg.tval.tmr_lock)
        { 
            if(LWC_INACTIVE == lc->lcf[HEAT_CURE].cure_out_actived)
            {                
                lc->lcf[HEAT_CURE].cure_out_actived = LWC_ACTIVED;
                                             
            }            
            if(1 == lc->lreg.btn.button_rl)/* low */
            {
                /* get the reload of the timer */
                hwc.ch = TMR_CH_HEAT_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;               
                hwc.value = tval*1/3;
                hwc.ch = TMR_CH_HEAT_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwc);
                if (err != RT_EOK)
                {
                    rt_kprintf("Set ch  = %d pwm Fail\n", hwc.ch);
                    return err;
                }
                else
                {
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                }
                hwc.ch = TMR_CH_HEAT_CURE;
                rt_device_control(dev, HWTIMER_CTRL_START, &hwc);
            }
            else if(2 == lc->lreg.btn.button_rl) /* middle */
            {
                hwc.ch = TMR_CH_HEAT_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*2/3;
                hwc.ch = TMR_CH_HEAT_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwc);
                if (err != RT_EOK)
                {
                    rt_kprintf("Set ch  = %d pwm Fail\n", hwc.ch);
                    return err;
                } 
                else
                {
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                }
            }
            else if(3 == lc->lreg.btn.button_rl) /* hight */
            {
                hwc.ch = TMR_CH_HEAT_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*3/3;
                hwc.ch = TMR_CH_HEAT_CURE;
                err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwc);
                if (err != RT_EOK)
                {
                    rt_kprintf("Set ch  = %d pwm Fail\n", hwc.ch);
                    return err;
                } 
                else
                {
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                }
            }                                               
        }              
    }
    
    
}


/*@}*/
