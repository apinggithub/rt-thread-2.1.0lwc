/*
 * File      : hwtimer.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author         Notes
 * 2015-08-31     heyuanjie87    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <drivers/hwtimer.h>

rt_inline rt_uint32_t timeout_calc(rt_device_hwtimer_t *timer, rt_hwtimerval_t *tv)
{

    timer->cycles = tv->sec * timer->freq;
       
    return timer->cycles;
}

extern rt_uint32_t SystemCoreClock;
static rt_err_t rt_hwtimer_init(struct rt_device *dev)
{
    rt_err_t result = RT_EOK;
    rt_device_hwtimer_t *timer;

    timer = (rt_device_hwtimer_t *)dev;   
    
    RT_ASSERT(timer->freq != 0);
    
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
    if ((1000000 <= timer->info->maxfreq) && (20 <= timer->info->minfreq))
    {
        /*if the frequence that user set is in 20Hz ~ 1MHz */
        /* the timer prescaler is the system clock hclk/freq */
        /* if hclk=72MHz ,and the prescaler is 72000000/1000000 = 72 */
        
        timer->prescaler = SystemCoreClock/1000000 - 1; /*set the defualt prescaler is 71 */
    }
    else //if(20 > timer->info->minfreq)
    {
        //timer->freq = timer->info->minfreq;
        timer->prescaler = SystemCoreClock/10000 - 1; /*set the defualt prescaler is 7199 */
    }
    
    timer->reload = SystemCoreClock/(timer->prescaler + 1)/timer->freq - 1; 
    //timer_period = sysclk/(timer->prescaler + 1)/freq - 1;    
    
    //timer->mode = HWTIMER_MODE_PERIOD;
    timer->cycles = 0;
    timer->overflow = 0;

    if (timer->ops->init)
    {
        timer->ops->init(timer, 1);
    }
    else
    {
        result = -RT_ENOSYS;
    }

    return result;
}

static rt_err_t rt_hwtimer_open(struct rt_device *dev, rt_uint16_t oflag)
{
    rt_err_t result = RT_EOK;
    rt_device_hwtimer_t *timer;

    timer = (rt_device_hwtimer_t *)dev;
    if (timer->ops->control != RT_NULL)
    {
        timer->ops->control(timer, HWTIMER_CTRL_SET_FREQ, &timer->freq);
    }
    else
    {
        result = -RT_ENOSYS;
    }

    return result;
}

static rt_err_t rt_hwtimer_close(struct rt_device *dev)
{
    rt_err_t result = RT_EOK;
    rt_device_hwtimer_t *timer;

    timer = (rt_device_hwtimer_t*)dev;
    if (timer->ops->init != RT_NULL)
    {
        timer->ops->init(timer, 0);
    }
    else
    {
        result = -RT_ENOSYS;
    }

    dev->flag &= ~RT_DEVICE_FLAG_ACTIVATED;
    dev->rx_indicate = RT_NULL;

    return result;
}

static rt_size_t rt_hwtimer_read(struct rt_device *dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_device_hwtimer_t *timer;
    rt_hwtimerval_t tv;
    rt_uint32_t cnt;

    timer = (rt_device_hwtimer_t *)dev;
    if (timer->ops->get_counter == RT_NULL)
        return 0;

    cnt = timer->ops->get_counter(timer);
    if (timer->info->cntmode == HWTIMER_CNTMODE_DW)
    {
        cnt = timer->info->maxcnt - cnt;
    }

    RT_ASSERT(timer->freq != 0);
    tv.sec =  timer->overflow/timer->freq;   
    tv.usec = (timer->overflow%timer->freq)*1000000 + cnt;
    
    size = size > sizeof(tv)? sizeof(tv) : size;
    rt_memcpy(buffer, &tv, size);

    return size;
}

static rt_size_t rt_hwtimer_write(struct rt_device *dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    //rt_uint32_t cycles;
    //rt_hwtimer_mode_t opm = HWTIMER_MODE_PERIOD;
    rt_device_hwtimer_t *timer;

    timer = (rt_device_hwtimer_t *)dev;
    if ((timer->ops->start == RT_NULL) || (timer->ops->stop == RT_NULL))
        return 0;

    if (size != sizeof(rt_hwtimerval_t))
        return 0;
    
    //calculater the cycles to auto reload value
    timer->cycles = timeout_calc(timer, (rt_hwtimerval_t*)buffer);

    //if ((timer->cycles <= 1) && (timer->mode == HWTIMER_MODE_ONESHOT))
    //{
    //    opm = HWTIMER_MODE_ONESHOT;
    //}
    timer->ops->stop(timer);
    timer->overflow = 0;
   
    if (timer->ops->start(timer, pos) != RT_EOK)
        size = 0;

    return size;
}

static rt_err_t rt_hwtimer_control(struct rt_device *dev, rt_uint8_t cmd, void *args)
{
    rt_err_t result = RT_EOK;
    rt_device_hwtimer_t *timer;

    timer = (rt_device_hwtimer_t *)dev;

    switch (cmd)
    {
        case HWTIMER_CTRL_STOP:
        {
            if (timer->ops->stop != RT_NULL)
            {
                timer->ops->stop(timer);
            }
            else
            {
                result = -RT_ENOSYS;
            }
        }
        break;
        case HWTIMER_CTRL_SET_FREQ:
        {
            rt_uint32_t *f;
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            f = (rt_uint32_t*)args;
            if ((*f > timer->info->maxfreq) || (*f < timer->info->minfreq))
            {
                result = -RT_ERROR;
                break;
            }
            if (timer->ops->control != RT_NULL)
            {
                result = timer->ops->control(timer, cmd, args);
                if (result == RT_EOK)
                {
                    timer->freq = *f;
                }
            }
            else
            {
                result = -RT_ENOSYS;
            }
        }
        break;
        case HWTIMER_CTRL_GET_INFO:
        {
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            *((struct rt_hwtimer_info*)args) = *timer->info;
        }
        break;
        /*case HWTIMER_CTRL_MODE_SET:
        {
            rt_hwtimer_mode_t *m;
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            m = (rt_hwtimer_mode_t*)args;            
            timer->mode = *m;
        }
        break; */     
        case HWTIMER_CTRL_GET_AUTORELOAD:
        {            
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            
            if (timer->ops->get_autoreload != RT_NULL)
            {
                rt_uint32_t *val;
                val = (rt_uint32_t*)args;
                *val = timer->ops->get_autoreload(timer);      
            }
            else
            {
                result = -RT_ENOSYS;
            }
        }   
        break;
        case HWTIMER_CTRL_SET_AUTORELOAD:
        {            
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            
            if (timer->ops->set_autoreload != RT_NULL)
            {
                rt_uint32_t val;
                val = *(rt_uint32_t*)args;
                result = timer->ops->set_autoreload(timer, val);      
            }
            else
            {
                result = -RT_ENOSYS;
            }
        }   
        break;
        case HWTIMER_CTRL_GET_DUTY_CYCLE:
        {            
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            
            if (timer->ops->get_compare != RT_NULL)
            {
                rt_hwtimercnt_t *hwt;
                hwt = (rt_hwtimercnt_t*)args;
                hwt->count = timer->ops->get_compare(timer, hwt->ch);      
            }
            else
            {
                result = -RT_ENOSYS;
            }
        }   
        break;
        case HWTIMER_CTRL_SET_DUTY_CYCLE:
        {
            rt_uint8_t *ch;
            if (args == RT_NULL)
            {
                result = -RT_EEMPTY;
                break;
            }
            
            ch = (rt_uint8_t*)args;
            if (*ch > 4) /* ch max is in 0~3*/
            {
                result = -RT_ERROR;
                break;
            }
            if (timer->ops->get_compare != RT_NULL)
            {
                rt_hwtimercnt_t *hwt;
                hwt = (rt_hwtimercnt_t*)args;
                result = timer->ops->set_compare(timer,hwt->ch, hwt->count);
                if (result != RT_EOK)
                {
                    break;
                }
            }
            else
            {
                result = -RT_ENOSYS;
            }
        }   
        break;
        default:  
        { 
            result = -RT_ENOSYS;   
            break;
        }
    }

    return result;
}

void rt_device_hwtimer_isr(rt_device_hwtimer_t *timer)
{
    RT_ASSERT(timer != RT_NULL);

    timer->overflow ++;/* add in end of the timer period */

    if (timer->cycles != 0)
    {
        timer->cycles --;
    }
    else //if (timer->cycles == 0)
    {       
        if (timer->ops->stop != RT_NULL)
        {
            timer->ops->stop(timer);
        }        

        if (timer->parent.rx_indicate != RT_NULL)
        {
            timer->parent.rx_indicate(&timer->parent, sizeof(struct rt_hwtimerval));
        }
    }
}

rt_err_t rt_device_hwtimer_register(rt_device_hwtimer_t *timer, const char *name, void *user_data)
{
    struct rt_device *device;

    RT_ASSERT(timer != RT_NULL);
    RT_ASSERT(timer->ops != RT_NULL);
    RT_ASSERT(timer->info != RT_NULL);

    device = &(timer->parent);

    device->type        = RT_Device_Class_Timer;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->init        = rt_hwtimer_init;
    device->open        = rt_hwtimer_open;
    device->close       = rt_hwtimer_close;
    device->read        = rt_hwtimer_read;
    device->write       = rt_hwtimer_write;
    device->control     = rt_hwtimer_control;
    device->user_data   = user_data;

    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
