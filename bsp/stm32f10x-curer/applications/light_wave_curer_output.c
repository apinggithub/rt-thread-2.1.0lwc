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

static rt_err_t lwc_cure_timer3_output(rt_device_t dev, lwc_cure_t *lc);
static rt_err_t lwc_cure_timer4_output(rt_device_t dev, lwc_cure_t *lc);
static rt_err_t lwc_cure_ion_output(rt_device_t dev, lwc_cure_t *lc);

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

static struct rt_timer timerions;
static struct rt_timer timerfm;

static rt_uint16_t ion_count;

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
static void timeout_ionswtich(void* parameter)
{

	if(20*60-1 > ion_count)
    {
        ion_count++;
    }
    else
    {
        ion_count = 0;
        //rt_timer_stop(&timerions);
        //rt_event_send(&event, RT_EVENT_LWC_TIMER_FINISH_CLOSE);
    }
}
static void timeout_frequency_modulation(void* parameter)
{
    
	
}
void lwc_output_thread_entry(void* parameter)
{
    rt_err_t err;
    rt_hwtimer_chval_t hwc;
    //rt_hwtimer_tmr_t tmr;
    rt_uint32_t recv_event;
#ifdef RT_USING_HWTIM3  
    rt_device_t dev_hwtimer3 = RT_NULL;
    rt_device_hwtimer_t *timer3;
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
#endif /* RT_USING_HWTIM3 */          

  
#ifdef RT_USING_HWTIM4
    rt_device_t dev_hwtimer4 = RT_NULL;    
    rt_device_hwtimer_t *timer4;
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
    hwc.value = timer4->freq = 1200;
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_FREQ, &hwc);
    if (err != RT_EOK)
    {
        rt_kprintf("Set timer freq = %d Hz Fail! And close the %s\n" ,timer4->freq, TIMER4);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }    
#endif /* RT_USING_HWTIM4 */   

        
	rt_timer_init(&timerions, "timerions", 
	timeout_ionswtich, 
	RT_NULL, 
	1000, /*1000 tick */
	RT_TIMER_FLAG_PERIODIC); 	
	//rt_timer_start(&timerions);
    
    rt_timer_init(&timerfm, "timerfm", 
	timeout_frequency_modulation, 
	RT_NULL, 
	1000, /*1000 tick */
	RT_TIMER_FLAG_PERIODIC); 	
	//rt_timer_start(&timerions);
       
    rt_pin_mode(PD2_BEEP, PIN_MODE_OUTPUT);
    rt_pin_mode(PB5_IONTHERAPY_RLY, PIN_MODE_OUTPUT);
    rt_pin_mode(PB12_IONTHERAPY_PWR, PIN_MODE_OUTPUT);
    rt_pin_mode(PB13_IONTHERAPY_CRL1, PIN_MODE_OUTPUT);
    rt_pin_mode(PB13_IONTHERAPY_CRL1, PIN_MODE_OUTPUT);
    rt_pin_mode(PB15_IONTHERAPY_DECT, PIN_MODE_INPUT);
    
    while(1)
    {
        
        //rt_pin_write(PB5_IONTHERAPY_SW, PIN_HIGH);
        //rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW);	
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
                    hwc.ch = TMR_CH_LASER_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                    hwc.ch = TMR_CH_HEAT_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                    hwc.ch = TMR_CH_CUREI_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                    hwc.ch = TMR_CH_CUREII_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                    
                    rt_event_send(&event, RT_EVENT_LWC_DEVICE_POWER_CLOSE);
                }
                break;
                case RT_EVENT_LWC_BUTTON_UPDATE:
                {
                    lwc_cure_timer3_output(dev_hwtimer3, (lwc_cure_t *)&lct);
                    lwc_cure_timer4_output(dev_hwtimer4, (lwc_cure_t *)&lct);
                }
                break;
                case RT_EVENT_LWC_LASER_CURE_CLOSE:
                {
                    hwc.ch = TMR_CH_LASER_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                }
                break;
                case RT_EVENT_LWC_HEAT_CURE_CLOSE:
                {
                    hwc.ch = TMR_CH_HEAT_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwc); 
                }
                break;
                default:
                break;
            }                      
        }
        lwc_cure_ion_output(dev_hwtimer3, (lwc_cure_t *)&lct);                     
        rt_thread_delay( RT_TICK_PER_SECOND/10 );
    }
}

static rt_err_t lwc_cure_timer3_output(rt_device_t dev, lwc_cure_t *lc)
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
                hwc.ch = TMR_CH_LASER_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;               
                hwc.value = tval*1/3;
                hwc.ch = TMR_CH_LASER_PWM;
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
                hwc.ch = TMR_CH_LASER_PWM;
                rt_device_control(dev, HWTIMER_CTRL_START, &hwc);
            }
            else if(2 == lc->lreg.btn.button_jg) /* middle */
            {
                hwc.ch = TMR_CH_LASER_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*2/3;
                hwc.ch = TMR_CH_LASER_PWM;
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
                hwc.ch = TMR_CH_LASER_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*3/3;
                hwc.ch = TMR_CH_LASER_PWM;
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
                hwc.ch = TMR_CH_HEAT_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;               
                hwc.value = tval*1/3;
                hwc.ch = TMR_CH_HEAT_PWM;
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
                hwc.ch = TMR_CH_HEAT_PWM;
                rt_device_control(dev, HWTIMER_CTRL_START, &hwc);
            }
            else if(2 == lc->lreg.btn.button_rl) /* middle */
            {
                hwc.ch = TMR_CH_HEAT_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*2/3;
                hwc.ch = TMR_CH_HEAT_PWM;
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
                hwc.ch = TMR_CH_HEAT_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;
                hwc.value = tval*3/3;
                hwc.ch = TMR_CH_HEAT_PWM;
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
    return err;
}

static rt_err_t lwc_cure_timer4_output(rt_device_t dev, lwc_cure_t *lc)
{
    rt_err_t err = RT_EOK;
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(lc != RT_NULL);   
    rt_hwtimer_chval_t hwc; 
    
    if(LWC_ACTIVED == lc->lway[FUNCTION].status)
    {
        if(1 == lc->lreg.tval.tmr_lock)
        { 
            if(LWC_INACTIVE == lc->lcf[FUNCTION].cure_out_actived)
            {                
                lc->lcf[FUNCTION].cure_out_actived = LWC_ACTIVED;
            }   
            /* get the reload of the timer */
            if(0 < lc->lreg.btn.button_zl1)
            {
                hwc.ch = TMR_CH_CUREI_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;               
                hwc.value = (lc->lreg.btn.button_zl1)*tval/24;
                hwc.ch = TMR_CH_CUREI_PWM;
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
                if(1 == lc->lreg.btn.button_zl1)
                {
                    hwc.ch = TMR_CH_CUREI_PWM;                
                    rt_device_control(dev, HWTIMER_CTRL_START, &hwc); 
                }                    
            }
            else
            {
               hwc.ch = TMR_CH_CUREI_PWM;
               rt_device_control(dev, HWTIMER_CTRL_STOP, &hwc);    
            }
            if(0 < lc->lreg.btn.button_zl2)
            {
                hwc.ch = TMR_CH_CUREII_PWM;
                err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &hwc); 
                if (err != RT_EOK)
                {
                    rt_kprintf("Get the timer reload Fail\n");
                    return err;
                }
                /* set duty cycles */
                uint16_t tval = hwc.value;               
                hwc.value = (lc->lreg.btn.button_zl2)*tval/24;
                hwc.ch = TMR_CH_CUREII_PWM;
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
                if(1 == lc->lreg.btn.button_zl2)
                {
                    hwc.ch = TMR_CH_CUREII_PWM;
                    rt_device_control(dev, HWTIMER_CTRL_START, &hwc);  
                }                    
            }
            else
            {
               hwc.ch = TMR_CH_CUREII_PWM;
               rt_device_control(dev, HWTIMER_CTRL_STOP, &hwc);    
            }
        }
                                                                                                                                          
    }  
    return err;
}
static rt_err_t lwc_cure_ion_output(rt_device_t dev, lwc_cure_t *lc)
{    
    rt_err_t err = RT_EOK;
    if(LWC_ACTIVED == lc->lway[IONICE_CURE].status)
    {
        if(1 == lc->lreg.tval.tmr_lock)
        { 
            if(LWC_INACTIVE == lc->lcf[IONICE_CURE].cure_out_actived)
            {                
                lc->lcf[IONICE_CURE].cure_out_actived = LWC_ACTIVED;
                rt_timer_start(&timerions);                             
            }            
            if(1 == lc->lreg.btn.button_lzlf)/* low */
            {
                /* turn on  control 1 */
                if(PIN_LOW == rt_pin_read(PB15_IONTHERAPY_DECT))
                {                    
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_HIGH);
                    rt_pin_write(PB13_IONTHERAPY_CRL1, PIN_HIGH);
                    rt_pin_write(PB14_IONTHERAPY_CRL2, PIN_LOW);
                    if(600 > ion_count)
                    {
                        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_HIGH);
                    }
                    else
                    {
                        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                    }
                }   
                else
                {
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW); 
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                }
                
            }
            else if(2 == lc->lreg.btn.button_lzlf) /* middle */
            {
                
                /* turn on  control 2 */
                if(PIN_LOW == rt_pin_read(PB15_IONTHERAPY_DECT))
                {                    
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_HIGH);
                    rt_pin_write(PB13_IONTHERAPY_CRL1,PIN_LOW);
                    rt_pin_write(PB14_IONTHERAPY_CRL2,PIN_HIGH);
                    if(600 > ion_count)
                    {
                        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_HIGH);
                    }
                    else
                    {
                        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                    }
                }   
                else
                {
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW); 
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                }
            }
            else if(3 == lc->lreg.btn.button_rl) /* hight */
            {
                             
               //rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
               /* turn on  control 3 */
                if(PIN_LOW == rt_pin_read(PB15_IONTHERAPY_DECT))
                {                    
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_HIGH);
                    rt_pin_write(PB13_IONTHERAPY_CRL1,PIN_HIGH);
                    rt_pin_write(PB14_IONTHERAPY_CRL2,PIN_HIGH);
                    if(600 > ion_count)
                    {
                        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_HIGH);
                    }
                    else
                    {
                        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                    }
                }   
                else
                {
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW); 
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                } 
                
            }             
        }              
    }
    else
    {
        rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW);
        rt_pin_write(PB13_IONTHERAPY_CRL1,PIN_LOW);
        rt_pin_write(PB14_IONTHERAPY_CRL2,PIN_LOW);
        rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
        rt_timer_stop(&timerions);
    }
    return err;        
}


/*@}*/
