#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>


#ifdef RT_USING_HWTIMER

static rt_err_t timer_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("HT %d\n", rt_tick_get());    
    return 0;
}

int hwtimer(void)
{
    rt_err_t err;
    rt_hwtimerval_t val;
    rt_device_t dev = RT_NULL;
    rt_tick_t tick;
    //rt_hwtimer_mode_t mode;
    int freq = 2000;//Hz, the timer out frequence (20Hz~1MHz)
    int t = 10;//sec. the timer work on time
    //int temp = 0;
    //uint8_t ch;
    
    rt_hwtimer_t *timer;
    
#ifdef RT_USING_HWTIM6
    
    #define TIMER6   "timer6"
    
    rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    if ((dev = rt_device_find(TIMER6)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER6);
        return -1;
    }
    
    timer = (rt_hwtimer_t *)dev;  
    timer->freq = freq;
    timer->prescaler = 71;
    timer->reload = 0;

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER6);
        return -1;
    }
    
    rt_device_set_rx_indicate(dev, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", freq);
        goto EXIT_TIM6;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", freq);
    }

    /* 周期模式 */
    //mode = HWTIMER_MODE_PERIOD;
    //err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    tick = rt_tick_get();
    rt_kprintf("Start Timer> Tick: %d\n", tick);
    /* 设置定时器超时值并启动定时器 */
    val.sec = t;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    
    for( uint8_t i = 0; i < 4; i++)
    {
        
        if (rt_device_write(dev, i, &val, sizeof(val)) != sizeof(val))
        {
            rt_kprintf("SetTime Fail\n");
            goto EXIT_TIM6;
        }
        else
        {
            rt_kprintf("Set timer work on  = %dsec. on channel No.%d ok.\n", val.sec,i);
        }
    }  
    rt_kprintf("NO. 1 timer on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    
    tempfreq = freq - 200;
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &tempfreq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", tempfreq);
        goto EXIT_TIM6;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", tempfreq);
    }
    rt_kprintf("NO. 2 timer on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    
    /* stop the timer */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT_TIM6:
    err = rt_device_close(dev);
    rt_kprintf("Close %s\n", TIMER6);
    
#endif /*RT_USING_HWTIM6*/    

#ifdef RT_USING_HWTIM2
    #define TIMER2   "timer2"
    //rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    if ((dev = rt_device_find(TIMER2)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER2);
        return -1;
    }
    
    timer = (rt_hwtimer_t *)dev;  
    timer->freq = freq;
    timer->prescaler = 71;
    timer->reload = 0;

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER2);
        return -1;
    }
    
    rt_device_set_rx_indicate(dev, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", freq);
        goto EXIT_TIM2;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", freq);
    }

    /* 周期模式 */
    //mode = HWTIMER_MODE_PERIOD;
    //err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    tick = rt_tick_get();
    rt_kprintf("Start Timer> Tick: %d\n", tick);
    /* 设置定时器超时值并启动定时器 */
    val.sec = t;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    
    //for( uint8_t i = 0; i < 4; i++)
    //{
        
        if (rt_device_write(dev, 2, &val, sizeof(val)) != sizeof(val))
        {
            rt_kprintf("SetTime Fail\n");
            goto EXIT_TIM2;
        }
        else
        {
            rt_kprintf("Set timer work on  = %dsec. on channel No.%d ok.\n", val.sec,2);
        }
        if (rt_device_write(dev, 3, &val, sizeof(val)) != sizeof(val))
        {
            rt_kprintf("SetTime Fail\n");
            goto EXIT_TIM2;
        }
        else
        {
            rt_kprintf("Set timer work on  = %dsec. on channel No.%d ok.\n", val.sec,3);
        }
    //}
    rt_kprintf("NO. 1 timer on %d sec.\n", t);
    rt_thread_delay(t*RT_TICK_PER_SECOND);
    /*
    rt_kprintf("NO. 1 timer on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    
    tempfreq = freq - 200;
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &tempfreq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", tempfreq);
        goto EXIT_TIM2;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", tempfreq);
    }
    rt_kprintf("NO. 2 timer on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    */
    /* stop the timer */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT_TIM2:
    err = rt_device_close(dev);
    rt_kprintf("Close %s\n", TIMER2);
    
#endif /*RT_USING_HWTIM2*/     
#if 0
#ifdef RT_USING_HWTIM3
    #define TIMER3   "timer3"

    if ((dev = rt_device_find(TIMER3)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER3);
        return -1;
    }
    
    timer = (rt_hwtimer_t *)dev;  
    timer->freq = freq;
    timer->prescaler = 71;
    timer->reload = 0;

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER3);
        return -1;
    }
    
    rt_device_set_rx_indicate(dev, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", freq);
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", freq);
    }

    /* 周期模式 */
    //mode = HWTIMER_MODE_PERIOD;
    //err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    tick = rt_tick_get();
    rt_kprintf("Start Timer> Tick: %d\n", tick);
    /* 设置定时器超时值并启动定时器 */
    val.sec = t;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    
         
    if (rt_device_write(dev, 2, &val, sizeof(val)) != sizeof(val))
    {
        rt_kprintf("SetTime Fail\n");
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Set timer work on  = %dsec. on channel No.%d ok.\n", val.sec,2);
    }
    
    rt_kprintf("NO. 1 timer on %d sec.\n", t);
    rt_thread_delay(t*RT_TICK_PER_SECOND);
    /* stop the timer */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev, 2, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT_TIM3:
    err = rt_device_close(dev);
    rt_kprintf("Close %s\n", TIMER3);
    
#endif /*RT_USING_HWTIM3*/ 
#endif
    
#ifdef RT_USING_HWTIM4
    #define TIMER4   "timer4"
    //rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    if ((dev = rt_device_find(TIMER4)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER4);
        return -1;
    }
    
    timer = (rt_hwtimer_t *)dev;  
    timer->freq = freq;
    timer->prescaler = 71;
    timer->reload = 0;

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER4);
        return -1;
    }
    
    rt_device_set_rx_indicate(dev, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev, HWTIMER_CTRL_SET_FREQ, &freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", freq);
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", freq);
    }

    /* 周期模式 */
    //mode = HWTIMER_MODE_PERIOD;
    //err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    tick = rt_tick_get();
    rt_kprintf("Start Timer> Tick: %d\n", tick);
    /* 设置定时器超时值并启动定时器 */
    val.sec = t;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    
    for( uint8_t i = 0; i < 4; i++)
    {
        
        if (rt_device_write(dev, i, &val, sizeof(val)) != sizeof(val))
        {
            rt_kprintf("SetTime Fail\n");
            goto EXIT_TIM4;
        }
        else
        {
            rt_kprintf("Set timer work on  = %dsec. on channel No.%d ok.\n", val.sec,i);
        }       
    }
    //rt_kprintf("NO. 1 timer on %d sec.\n", t);
    //rt_thread_delay(t*RT_TICK_PER_SECOND);
    
    rt_kprintf("NO. 1 timer on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    {
        uint32_t ch = 0,val = 0;
        rt_kprintf(" default reload = %d \n", timer->reload);       
        err = rt_device_control(dev, HWTIMER_CTRL_GET_AUTORELOAD, &val);    
        if (err != RT_EOK)
        {
            rt_kprintf("Get the timer reload Fail\n");
            goto EXIT_TIM4;
        }
        else
        {
            rt_kprintf("Get the timer reload  = %d \n", val);
        }
        rt_kprintf(" default Get ch = %d pwm = %d \n", ch, timer->pwm_duty_cycle[ch]);
        rt_hwtimercnt_t hwt;
        hwt.ch = ch;
        
        err = rt_device_control(dev, HWTIMER_CTRL_GET_DUTY_CYCLE, &hwt);
        if (err != RT_EOK)
        {
            rt_kprintf("Set ch  = %d pwm Fail\n", ch);
            goto EXIT_TIM4;
        }
        else
        {
            rt_kprintf("Get ch = %d pwm = %d ok,\n", hwt.ch, hwt.count);
        }
        
        hwt.count += 100;
        err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt);
        if (err != RT_EOK)
        {
            rt_kprintf("Set ch  = %d pwm Fail\n", ch);
            goto EXIT_TIM4;
        }
        else
        {
            rt_kprintf("Set ch = %d pwm = %d ok,\n", hwt.ch, hwt.count);
        }
    }
    /*
    ch = 2;
    err = rt_device_control(dev, HWTIMER_CTRL_GET_DUTY_CYCLE, &ch);
    if (err != RT_EOK)
    {
        rt_kprintf("Get ch = %d pwm Fail\n", ch);
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Get ch = %d pwm = %d ok\n", ch, timer->pwm_duty_cycle[ch]);
    }
    timer->pwm_duty_cycle[ch] += 100;
    err = rt_device_control(dev, HWTIMER_CTRL_SET_DUTY_CYCLE, &ch);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", ch);
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Set ch = %d pwm = %d ok,\n", ch, timer->pwm_duty_cycle[ch]);
    }
    
    rt_kprintf("NO. 2 timer on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    */
    /* stop the timer */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT_TIM4:
    err = rt_device_close(dev);
    rt_kprintf("Close %s\n", TIMER4);
    
#endif /*RT_USING_HWTIM2*/     

    return err;

}

FINSH_FUNCTION_EXPORT(hwtimer, Test hardware timer);
#endif
