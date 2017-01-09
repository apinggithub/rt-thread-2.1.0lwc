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

static const float freq[][6] = {
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
rt_uint8_t lwc_output_stack[ 2048 ];
struct rt_thread lwc_output_thread;

struct rt_timer timerions;
static rt_uint16_t ion_count;
lwc_function_fm_t lff;

static rt_err_t timer3_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_event_send(&event, RT_EVENT_LWC_TIMER_FINISH_CLOSE);
    return 0;
}

/*switch timer 4 on ch2 and ch4 */
void rt_device_hwtimer_hook(rt_device_hwtimer_t *timer, rt_uint8_t ch )
{
    rt_hwtimer_chfreq_t     hwq;
    rt_device_t dev = RT_NULL;   
    
    //rt_pin_write(PD2_BEEP, lff.fm_switch);
    lff.fm_switch = (~lff.fm_switch)&0x01;
        
    if ((dev = rt_device_find(TIMER4)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER4);
        while(1);
    } 
    
    if(0x01 == lff.fm_switch)
    {
        if(0 < lct.lreg.btn.button_zl1)
        {                                             
            hwq.ch = TMR_CH_CUREI_FREQ;           
            rt_device_control(dev, HWTIMER_CTRL_START, &hwq);                                
        }
        else
        {
           hwq.ch = TMR_CH_CUREI_FREQ;
           rt_device_control(dev, HWTIMER_CTRL_STOP, &hwq);    
        }  
        if(0 < lct.lreg.btn.button_zl2)
        {                                                            
            hwq.ch = TMR_CH_CUREII_FREQ;           
            rt_device_control(dev, HWTIMER_CTRL_START, &hwq);                                
        }
        else
        {
           hwq.ch = TMR_CH_CUREII_FREQ;
           rt_device_control(dev, HWTIMER_CTRL_STOP, &hwq);    
        }      
    }
    else
    {
        hwq.ch = TMR_CH_CUREI_FREQ;
        rt_device_control(dev, HWTIMER_CTRL_STOP, &hwq); 
        hwq.ch = TMR_CH_CUREII_FREQ;
        rt_device_control(dev, HWTIMER_CTRL_STOP, &hwq); 
    }
}
static rt_err_t timer6_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_err_t err;
    rt_hwtimer_chfreq_t     hwq;
    rt_hwtimer_tmrval_t     hwt;
    //rt_kprintf("HT %d\n", rt_tick_get());  
    if((0 < lct.lreg.btn.button_gn)&&(9 >= lct.lreg.btn.button_gn))
    {
        if(5 < lff.fm_idx)/* 0 ~ 5*/
        {
            lff.fm_idx = 0;
        }
        hwq.ch = HWTIMER_BASE;
        hwq.freq = (2*freq[lff.func_idx][lff.fm_idx]);
        err = rt_device_control(dev, HWTIMER_CTRL_SET_FREQ, &hwq);
        if (err != RT_EOK)
        {
            rt_kprintf("Set Freq[%d][%d] = %dHz Fail\n", lff.func_idx,lff.fm_idx, (uint32_t)hwq.freq/2);
            while(1);
        }
        else   
        {
            #ifdef USER_HWTIMER_APP_BUG_TEST
            rt_kprintf("Set Freq[%d][%d] = %dHz ok\n", lff.func_idx,lff.fm_idx, (uint32_t)hwq.freq/2);
            #endif
        }        
        hwt.sec = 10;
        hwt.usec = 0;
        //rt_kprintf("SetTime: Sec %d, Usec %d\n", hwt.sec, hwt.usec);   
        if (rt_device_write(dev, hwq.ch, &hwt, sizeof(hwt)) != sizeof(hwt))
        {
            rt_kprintf("SetTime Fail\n");
            while(1);
        }
        else
        {
            #ifdef USER_HWTIMER_APP_BUG_TEST
            rt_kprintf("Set timer work on  = %dsec. ok.\n", hwt.sec);
            #endif
        }
        switch(lct.lreg.btn.button_gn)
        {                
            case FULL_FUNCTION:   /* 1 È«¹¦ÄÜ */
            {
              if(5 == lff.fm_idx)
              {
                  if(0 == lct.lreg.btn.button_sd)
                  {
                        lff.func_idx++;
                  }
            
              }  
              if(7 < lff.func_idx ) /* 0 ~ 7 */
              {              
                  lff.func_idx = 0;
              }             
            } 
            break; 
            case MID_FREQUENCY:          /* 2 ÖÐÆµ */
            {
                lff.func_idx = 0;
            }
            break; 
            case ACUPUNCTURE_NEEDLE:     /* 3 Õë¾Ä */
            {    
                lff.func_idx = 1;
            }
            break; 
            case PAT_CURE:               /* 4 ÅÄ´ò */
            {
                lff.func_idx = 2;
            }
            break;     
            case NAPRAPATHY:             /* 5 ÍÆÄÃ */
            {
                lff.func_idx = 3;
            }
            break;     
            case MASSOTHERAPY:           /* 6 °´Ä¦ */
            {
                lff.func_idx = 4;
            }
            break;     
            case CUPPING_THERAPY:        /* 7 °Î¹Þ */
            {
                lff.func_idx = 5;
            }
            break;     
            case PEDICURE:               /* 8 ×ãÁÆ */
            {
                lff.func_idx = 6;
            }
            break; 
            case LOSE_WEIGHT:            /* 9 ¼õ·Ê */
            {
                lff.func_idx = 7;
            }
            break;  
            #if 0
            case VOICE_FREQUENCY:        /* ÒôÆµ */
            {
                lff.func_idx = 8;
            }    
            break;
            #endif
            default:
                //lff.func_idx = 0;
            break;        
        } 
        if(0 == lct.lreg.btn.button_sd)
        {
            lff.fm_idx++;
        }
        
    }   
    return 0;
}
static void timeout_ionswtich(void* parameter)
{

	if(1200-1 > ion_count)
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

void lwc_output_thread_entry(void* parameter)
{
    rt_err_t err;
    rt_hwtimer_chfreq_t hwq;
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
    timer3->freq = 2;
    timer3->prescaler = 7199;
    timer3->reload = 833;
    //d = (fs/(f*p))-1 fs-->system frequency 72MHz,f --->timer output(interrupt),p-->prescaler    
    if (rt_device_open(dev_hwtimer3, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER3);
        while(1);
    }
    rt_device_set_rx_indicate(dev_hwtimer3, timer3_timeout_cb);
     /* set the frequency */
    hwq.freq = timer3->freq;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_FREQ, &hwq);
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
    //rt_device_set_rx_indicate(dev_hwtimer4, timer4_timeout_cb);
    hwq.freq = timer4->freq = 1200;
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_FREQ, &hwq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set timer freq = %d Hz Fail! And close the %s\n" ,timer4->freq, TIMER4);
        err = rt_device_close(dev_hwtimer4);
        while(1);
    }    
#endif /* RT_USING_HWTIM4 */   
    
    
#ifdef RT_USING_HWTIM6
    rt_device_t dev_hwtimer6 = RT_NULL;
    
    hwq.ch = HWTIMER_BASE;
    //rt_pin_mode(54, PIN_MODE_OUTPUT);// the port PD2
   
    if((dev_hwtimer6 = rt_device_find(TIMER6)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER6);
        while(1);
    }
    
    rt_device_hwtimer_t *timer6 = (rt_device_hwtimer_t *)dev_hwtimer6;
    timer6 = (rt_device_hwtimer_t *)dev_hwtimer6;  
    timer6->freq = 1;
    timer6->prescaler = 7199;
    timer6->reload = 0;       
       
    if (rt_device_open(dev_hwtimer6, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER6);
        while(1);
    }
    
    rt_device_set_rx_indicate(dev_hwtimer6, timer6_timeout_cb);
    hwq.freq = timer6->freq;
    err = rt_device_control(dev_hwtimer6, HWTIMER_CTRL_SET_FREQ, &hwq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", hwq.freq);
        while(1);
    }


#endif /* RT_USING_HWTIM6 */   
    
	rt_timer_init(&timerions, "timerions", 
	timeout_ionswtich, 
	RT_NULL, 
	1000, /*1000 tick */
	RT_TIMER_FLAG_PERIODIC); 	
	//rt_timer_start(&timerions);
    
           
    rt_pin_mode(PD2_BEEP, PIN_MODE_OUTPUT);
    rt_pin_mode(PB5_IONTHERAPY_RLY, PIN_MODE_OUTPUT);
    rt_pin_mode(PB12_IONTHERAPY_PWR, PIN_MODE_OUTPUT);
    rt_pin_mode(PB13_IONTHERAPY_CRL1, PIN_MODE_OUTPUT);
    rt_pin_mode(PB14_IONTHERAPY_CRL2, PIN_MODE_OUTPUT);
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
                                    |RT_EVENT_LWC_ION_FUNC_START
                                    |RT_EVENT_LWC_ION_CURE_CLOSE
                                    ),
                           RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                           RT_TICK_PER_SECOND/100, &recv_event) == RT_EOK)
        {
            switch(recv_event)
            {
                case RT_EVENT_LWC_TIMER_FINISH_CLOSE:
                case RT_EVENT_LWC_DEVICE_FORCE_CLOSE:    
                {
                    hwq.ch = TMR_CH_LASER_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwq); 
                    hwq.ch = TMR_CH_HEAT_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwq); 
                    hwq.ch = TMR_CH_CUREI_PWM;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq); 
                    hwq.ch = TMR_CH_CUREII_PWM;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq); 
                    hwq.ch = TMR_CH_CUREI_FREQ;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq);
                    hwq.ch = TMR_CH_CUREII_FREQ;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq);   
                    hwq.ch = TMR_CH_BASE;
                    rt_device_control(dev_hwtimer6, HWTIMER_CTRL_STOP, &hwq);       
                                                    
                    rt_event_send(&event, RT_EVENT_LWC_DEVICE_POWER_CLOSE);
                }
                break;
                case RT_EVENT_LWC_BUTTON_UPDATE:
                {
                    lwc_cure_timer3_output(dev_hwtimer3, (lwc_cure_t *)&lct);
                    lwc_cure_timer4_output(dev_hwtimer4, (lwc_cure_t *)&lct);
                    if((0 == lct.lreg.btn.button_zl1 )&&(0 == lct.lreg.btn.button_zl2 ))
                    {
                        lct.lreg.btn.flag_base_timer_status = LWC_BASE_TIMER_IDLE;
                        hwq.ch = TMR_CH_BASE;  
                        rt_device_control(dev_hwtimer6, HWTIMER_CTRL_STOP, &hwq);
                    }
                }
                break;
                case RT_EVENT_LWC_LASER_CURE_CLOSE:
                {
                    hwq.ch = TMR_CH_LASER_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwq); 
                }
                break;
                case RT_EVENT_LWC_HEAT_CURE_CLOSE:
                {
                    hwq.ch = TMR_CH_HEAT_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwq); 
                }
                break;
                case RT_EVENT_LWC_ION_FUNC_START:    
                {
                    lct.lreg.btn.button_jg = 0;
                    lct.lway[LASER_CURE].status = LWC_INACTIVE;
                    lct.lcf[LASER_CURE].cure_out_actived = LWC_INACTIVE;
                    hwq.ch = TMR_CH_LASER_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwq); 
                    
                    lct.lreg.btn.button_rl = 0;
                    lct.lway[HEAT_CURE].status = LWC_INACTIVE;
                    lct.lcf[HEAT_CURE].cure_out_actived = LWC_INACTIVE;
                    hwq.ch = TMR_CH_HEAT_PWM;
                    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwq); 
                    
                    lct.lreg.btn.button_gn = 0;
                    lct.lreg.btn.button_zl1 = 0;
                    lct.lreg.btn.button_zl2 = 0;
                    lct.lway[FUNCTION].status = LWC_INACTIVE;
                    lct.lcf[FUNCTION].cure_out_actived = LWC_INACTIVE;
                    hwq.ch = TMR_CH_CUREI_PWM;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq); 
                    hwq.ch = TMR_CH_CUREII_PWM;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq); 
                    hwq.ch = TMR_CH_CUREI_FREQ;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq);
                    hwq.ch = TMR_CH_CUREII_FREQ;
                    rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, &hwq);   
                    hwq.ch = TMR_CH_BASE;
                    rt_device_control(dev_hwtimer6, HWTIMER_CTRL_STOP, &hwq); 
                    
                    lct.lcf[IONICE_CURE].cure_out_actived = LWC_ACTIVED; 
                    rt_event_send(&event, RT_EVENT_LWC_ION_TIME_UPDATE);
                }
                break;
                case RT_EVENT_LWC_ION_CURE_CLOSE:
                {
                    rt_timer_stop(&timerions);
                    rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW);
                    rt_pin_write(PB13_IONTHERAPY_CRL1,PIN_LOW);
                    rt_pin_write(PB14_IONTHERAPY_CRL2,PIN_LOW);
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);                                     
                    rt_pin_write(PD2_BEEP, PIN_LOW);  
                    lct.lcf[IONICE_CURE].cure_out_actived = LWC_INACTIVE;
                }
                break;
                default:
                break;
            }                      
        }
        lwc_cure_ion_output(dev_hwtimer3, (lwc_cure_t *)&lct);                     
        rt_thread_delay( RT_TICK_PER_SECOND/100 );
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
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
    //rt_device_hwtimer_t *timer = (rt_device_hwtimer_t *)dev;   
    rt_hwtimer_chval_t hwc; 
    rt_hwtimer_tmrval_t hwt; 
    
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
                }                
                if(1 == lc->lreg.btn.button_zl1)
                {
                    rt_device_t dev_hwtimer6 = RT_NULL;                                       
                    hwc.ch = TMR_CH_CUREI_PWM;                
                    rt_device_control(dev, HWTIMER_CTRL_START, &hwc);
                    hwc.ch = TMR_CH_CUREI_FREQ;                
                    rt_device_control(dev, HWTIMER_CTRL_START, &hwc);
                    if(LWC_BASE_TIMER_READY == lct.lreg.btn.flag_base_timer_status)
                    {
                        lct.lreg.btn.flag_base_timer_status = LWC_BASE_TIMER_RUNNING;
                        if((dev_hwtimer6 = rt_device_find(TIMER6)) == RT_NULL)
                        {
                            rt_kprintf("No Device: %s\n", TIMER6);
                            while(1);
                        }
                        rt_device_hwtimer_t *timer = (rt_device_hwtimer_t *)dev_hwtimer6;                                      
                        if(0 == timer->channel_lock[TMR_CH_BASE])
                        {                       
                            hwt.sec = 1;  
                            hwt.usec = 0;                            
                            hwc.ch = TMR_CH_BASE;                          
                            if (rt_device_write(dev_hwtimer6, hwc.ch, &hwt, sizeof(hwt)) != sizeof(hwt))
                            {
                                rt_kprintf("SetTime Fail\n");
                                while(1);
                            }
                            else
                            {
                                #ifdef USER_HWTIMER_APP_BUG_TEST
                                rt_kprintf("SetTime: Sec %d, Usec %d\n", hwt.sec, hwt.usec); 
                                #endif
                            }
                            
                        }
                    }
                }                    
            }
            else
            {
               hwc.ch = TMR_CH_CUREI_PWM;
               rt_device_control(dev, HWTIMER_CTRL_STOP, &hwc); 
               hwc.ch = TMR_CH_CUREI_FREQ;  
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
                    #ifdef USER_HWTIMER_APP_BUG_TEST
                    rt_kprintf("Set ch = %d pwm = %d ok,\n", hwc.ch, hwc.value);
                    #endif
                }
                if(1 == lc->lreg.btn.button_zl2)
                {
                    rt_device_t dev_hwtimer6 = RT_NULL;                                       
                    hwc.ch = TMR_CH_CUREII_PWM;
                    rt_device_control(dev, HWTIMER_CTRL_START, &hwc);  
                    hwc.ch = TMR_CH_CUREII_FREQ;
                    rt_device_control(dev, HWTIMER_CTRL_START, &hwc);  
                    if(LWC_BASE_TIMER_READY == lct.lreg.btn.flag_base_timer_status)
                    {
                        lct.lreg.btn.flag_base_timer_status = LWC_BASE_TIMER_RUNNING;
                        if((dev_hwtimer6 = rt_device_find(TIMER6)) == RT_NULL)
                        {
                            rt_kprintf("No Device: %s\n", TIMER6);
                            while(1);
                        }
                        rt_device_hwtimer_t *timer = (rt_device_hwtimer_t *)dev_hwtimer6;  
                        if(0 == timer->channel_lock[TMR_CH_BASE])
                        {                   
                            hwt.sec = 1;  
                            hwt.usec = 0;
                            hwc.ch = TMR_CH_BASE;                                              
                            if (rt_device_write(dev_hwtimer6, hwc.ch, &hwt, sizeof(hwt)) != sizeof(hwt))
                            {
                                rt_kprintf("SetTime Fail\n");
                                while(1);
                            }
                            else
                            {
                                #ifdef USER_HWTIMER_APP_BUG_TEST
                                rt_kprintf("SetTime: Sec %d, Usec %d\n", hwt.sec, hwt.usec);   
                                #endif
                            }
                        }
                    }
                }                    
            }
            else
            {
                hwc.ch = TMR_CH_CUREII_PWM;
                rt_device_control(dev, HWTIMER_CTRL_STOP, &hwc);  
                hwc.ch = TMR_CH_CUREII_FREQ;  
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
            if((1 == lc->lreg.btn.button_lzlf)&&(PIN_LOW == rt_pin_read(PB15_IONTHERAPY_DECT)))/* low turn on  control 1 2 */
            {
                                                   
                rt_pin_write(PB12_IONTHERAPY_PWR, PIN_HIGH);
                rt_pin_write(PB13_IONTHERAPY_CRL1, PIN_HIGH);
                rt_pin_write(PB14_IONTHERAPY_CRL2, PIN_HIGH);
                if(600 > ion_count)
                {
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_HIGH);
                }
                else
                {
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                }
                rt_pin_write(PD2_BEEP, PIN_LOW);
            }
            else if((2 == lc->lreg.btn.button_lzlf)&&(PIN_LOW == rt_pin_read(PB15_IONTHERAPY_DECT))) /* middle turn on  control 2 */
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
                rt_pin_write(PD2_BEEP, PIN_LOW);
            }
            else if((3 == lc->lreg.btn.button_lzlf)&&(PIN_LOW == rt_pin_read(PB15_IONTHERAPY_DECT))) /* hight turn on  control 1 */
            {
                                                             
                rt_pin_write(PB12_IONTHERAPY_PWR, PIN_HIGH);
                rt_pin_write(PB13_IONTHERAPY_CRL1,PIN_HIGH);
                rt_pin_write(PB14_IONTHERAPY_CRL2,PIN_LOW);
                if(600 > ion_count)
                {
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_HIGH);
                }
                else
                {
                    rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                } 
                rt_pin_write(PD2_BEEP, PIN_LOW);    
            }
            else
            {
                rt_pin_write(PB12_IONTHERAPY_PWR, PIN_LOW);
                rt_pin_write(PB13_IONTHERAPY_CRL1,PIN_LOW);
                rt_pin_write(PB14_IONTHERAPY_CRL2,PIN_LOW);
                rt_pin_write(PB5_IONTHERAPY_RLY, PIN_LOW);
                //rt_timer_stop(&timerions);
                if((TMR_DELAY_100ms >= tmr_count)
                    ||((TMR_DELAY_200ms <= tmr_count)&&(TMR_DELAY_300ms >= tmr_count))
                    ||((TMR_DELAY_400ms <= tmr_count)&&(TMR_DELAY_500ms >= tmr_count))
                    )
                {
                     rt_pin_write(PD2_BEEP, PIN_HIGH);                
                }
                else
                {
                    rt_pin_write(PD2_BEEP, PIN_LOW);
                }
            }    
        }              
    }    
    return err;        
}


/*@}*/
