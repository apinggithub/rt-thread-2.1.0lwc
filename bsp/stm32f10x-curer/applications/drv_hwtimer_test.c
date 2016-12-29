#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#ifdef RT_USING_HWTIMER
#ifdef RT_USING_HWTIMER_TEST     

static rt_err_t timer3a_timeout_cb(rt_device_t dev, rt_size_t size)
{
    //rt_kprintf("HT %d\n", rt_tick_get());    
    //if(dev == dev_hwtimer3)
    //{
        rt_kprintf("timer3 stop in timeout_cb \n");  
    //}
    
    return 0;
}    
static rt_err_t timer4a_timeout_cb(rt_device_t dev, rt_size_t size)
{
    //rt_kprintf("HT %d\n", rt_tick_get());    
    //if(dev == dev_hwtimer4)
    //{
        rt_kprintf("timer4 stop in timeout_cb \n");  
    //}
    return 0;
}

int hwtimer(void)
{
    rt_err_t err;
    rt_hwtimer_tmr_t tmr;

    //rt_tick_t tick;
    //rt_hwtimer_mode_t mode;
    //int freq = 2000;//Hz, the timer out frequence (20Hz~1MHz)
    int t = 30;//sec. the timer work on time
    //int temp = 0;
    //uint8_t ch;
    
    //rt_device_hwtimer_t *timer;
    
#ifdef RT_USING_HWTIM6
    
    #define TIMER6   "timer6"
    
    rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    if ((dev = rt_device_find(TIMER6)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER6);
        return -1;
    }
    
    timer = (rt_device_hwtimer_t *)dev;  
    timer->freq = freq;
    timer->prescaler = 71;
    timer->reload = 0;
    
    rt_kprintf("Now test the %s \n", TIMER6);
       
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
    rt_device_t dev_hwtimer2 = RT_NULL;
    #define TIMER2   "timer2"
    //rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
    uint16_t tempfreq;
    
    rt_kprintf("Now test the %s ... \n", TIMER2);
   
    if ((dev_hwtimer2 = rt_device_find(TIMER2)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER2);
        return -1;
    }
    rt_device_hwtimer_t *timer2 = (rt_device_hwtimer_t *)dev_hwtimer2;
    timer2 = (rt_device_hwtimer_t *)dev_hwtimer2;  
    timer2->freq = 1200;
    timer2->prescaler = 71;
    timer2->reload = 0;

    if (rt_device_open(dev_hwtimer2, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER2);
        return -1;
    }
    
    //rt_device_set_rx_indicate(dev_hwtimer2, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev_hwtimer2, HWTIMER_CTRL_SET_FREQ, &timer2->freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", timer2->freq);
        goto EXIT_TIM2;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", timer2->freq);
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
        if (rt_device_write(dev_hwtimer2, i, &val, sizeof(val)) != sizeof(val))
        {
            rt_kprintf("SetTime Fail\n");
            goto EXIT_TIM2;
        }
        else
        {
            rt_kprintf("Set %s work on  = %d sec. on channel PA%d ok.\n", TIMER2, val.sec, i);
        }
        
    }
    rt_kprintf("The timer will work on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);

    rt_kprintf("The timer worked %d sec.\n", t/2);    
    tempfreq = timer2->freq - 200;
    rt_kprintf("Now timer changed  frequenc is %d Hz.\n", tempfreq);
    err = rt_device_control(dev_hwtimer2, HWTIMER_CTRL_SET_FREQ, &tempfreq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", tempfreq);
        goto EXIT_TIM2;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", tempfreq);
    }
     rt_kprintf("The timer will work on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    
    /* stop the timer */
    err = rt_device_control(dev_hwtimer2, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev_hwtimer2, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT_TIM2:
    err = rt_device_close(dev_hwtimer2);
    rt_kprintf("Close %s\n", TIMER2);
    
#endif /*RT_USING_HWTIM2*/     

#ifdef RT_USING_HWTIM3
    #define TIMER3   "timer3"
    rt_device_t dev_hwtimer3 = RT_NULL;
    
    //uint8_t ch = 3;
    static uint16_t aval;
    rt_hwtimer_val_t hwt3;
    hwt3.ch = 3;
    
    rt_kprintf("Now test the %s ... \n", TIMER3);
    
    if ((dev_hwtimer3 = rt_device_find(TIMER3)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER3);
        return -1;
    }
    rt_device_hwtimer_t *timer3 = (rt_device_hwtimer_t *)dev_hwtimer3;
    timer3 = (rt_device_hwtimer_t *)dev_hwtimer3;  
    timer3->freq = 1200;
    timer3->prescaler = 71;
    timer3->reload = 0;

    if (rt_device_open(dev_hwtimer3, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER3);
        return -1;
    }
    
    rt_device_set_rx_indicate(dev_hwtimer3, timer3a_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    hwt3.value = timer3->freq;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_FREQ, &hwt3);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", timer3->freq);
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", timer3->freq);
    }
    
    /* 周期模式 */
    //mode = HWTIMER_MODE_PERIOD;
    //err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);
    
    rt_kprintf(" default reload = %d \n", timer3->reload);  
       
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_AUTORELOAD, &hwt3); 
    if (err != RT_EOK)
    {
        rt_kprintf("Get the timer reload Fail\n");
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Get the timer reload  = %d \n", hwt3.value);
    }
    //rt_kprintf(" default Get ch = %d pwm = %d \n", ch, timer->pwm_duty_cycle[ch]);
    
    
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_DUTY_CYCLE, &hwt3);
    if (err != RT_EOK)
    {
        rt_kprintf("Get ch = %d pwm Fail\n", hwt3.ch);
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Get ch = %d pwm = %d ok\n", hwt3.ch, hwt3.value);
    }
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_AUTORELOAD, &hwt3); 
    if (err != RT_EOK)
    {
        rt_kprintf("Get the timer reload Fail\n");
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Get the timer reload  = %d \n", hwt3.value);
    }
    //timer->pwm_duty_cycle[ch] += 100;
    aval = hwt3.value;
    hwt3.value = aval*1/3;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_DUTY_CYCLE, (rt_hwtimer_val_t*)&hwt3);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt3.ch);
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Set ch = %d pwm = %d ok,\n", hwt3.ch, hwt3.value);
    }
    //tick = rt_tick_get();
    //rt_kprintf("Start Timer> Tick: %d\n", tick);
    /* 设置定时器超时值并启动定时器 */
    tmr.sec = t;
    tmr.usec = 0;
    
    rt_kprintf("SetTime: Sec %d, Usec %d\n", tmr.sec, tmr.usec);
    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_START, &hwt3);
    /*if (rt_device_write(dev_hwtimer3, hwt3.ch, &tmr, sizeof(tmr)) != sizeof(tmr))
    {
        rt_kprintf("SetTime Fail\n");
        goto EXIT_TIM3;
    }*/
    rt_kprintf("The timer will work on %d sec.\n", t/2);
    
    rt_thread_delay(t*RT_TICK_PER_SECOND/2 );
    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwt3);
    //ch = 2;   
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_DUTY_CYCLE, &hwt3);
    if (err != RT_EOK)
    {
        rt_kprintf("Get ch = %d pwm Fail\n", hwt3.ch);
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Get ch = %d pwm = %d ok\n", hwt3.ch, hwt3.value);
    }
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_GET_AUTORELOAD, &hwt3); 
    if (err != RT_EOK)
    {
        rt_kprintf("Get the timer reload Fail\n");
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Get the timer reload  = %d \n", hwt3.value);
    }
    aval = hwt3.value;
    hwt3.value = aval*2/3;
    err = rt_device_control(dev_hwtimer3, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt3);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt3.ch);
        goto EXIT_TIM3;
    }
    else
    {
        rt_kprintf("Set ch = %d pwm = %d ok,\n", hwt3.ch, hwt3.value);
    }
    //rt_device_control(dev_hwtimer3, HWTIMER_CTRL_START, &hwt3);
    tmr.sec = t/2;
    if (rt_device_write(dev_hwtimer3, hwt3.ch, &tmr, sizeof(tmr)) != sizeof(tmr))
    {
        rt_kprintf("SetTime Fail\n");
        goto EXIT_TIM3;
    }
    rt_kprintf("The timer will work on %d sec.\n", t/2);
    
    rt_thread_delay(t*RT_TICK_PER_SECOND/2);
    rt_device_control(dev_hwtimer3, HWTIMER_CTRL_STOP, &hwt3);
    rt_device_read(dev_hwtimer3, hwt3.ch, &tmr, sizeof(tmr));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", tmr.sec, tmr.usec);
    rt_kprintf("Read: timer->cycle[%d] = %d, \n", hwt3.ch, timer3->cycles[hwt3.ch]);
    
EXIT_TIM3:
    err = rt_device_close(dev_hwtimer3);
    rt_kprintf("Close %s\n", TIMER3);
    
#endif /*RT_USING_HWTIM3*/ 

   
#ifdef RT_USING_HWTIM4
    #define TIMER4   "timer4"
    rt_device_t dev_hwtimer4 = RT_NULL;  
    rt_hwtimercnt_t hwt4;
    static uint8_t val4 = 0;
    hwt4.ch = 0;
    //rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    rt_kprintf("Now test the %s ... \n", TIMER4);
    
    if ((dev_hwtimer4 = rt_device_find(TIMER4)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER4);
        return -1;
    }
    rt_device_hwtimer_t *timer4 = (rt_device_hwtimer_t *)dev_hwtimer4;
    timer4 = (rt_device_hwtimer_t *)dev_hwtimer4;  
    timer4->freq = 1200;
    timer4->prescaler = 71;
    timer4->reload = 0;

    if (rt_device_open(dev_hwtimer4, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER4);
        return -1;
    }
    
    //rt_device_set_rx_indicate(dev_hwtimer4, timer4a_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_FREQ, &timer4->freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", timer4->freq);
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", timer4->freq);
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
    
    for( uint8_t i = HWTIMER_CH1; i < HWTIMER_CH1 + 4; i++)
    {
        
        if (rt_device_write(dev_hwtimer4, i, &val, sizeof(val)) != sizeof(val))
        {
            rt_kprintf("SetTime Fail\n");
            goto EXIT_TIM4;
        }
        else
        {
            rt_kprintf("Set %s work on  = %d sec. on channel PB%d ok.\n", TIMER4, val.sec, i+5);
        }       
    }
    rt_kprintf("The timer will work on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2 + 1);
           
    rt_kprintf(" default reload = %d \n", timer4->reload);       
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_GET_AUTORELOAD, &val4);    
    if (err != RT_EOK)
    {
        rt_kprintf("Get the timer reload Fail\n");
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Get the timer reload  = %d \n", val4);
    }
    //rt_kprintf(" default Get ch = %d pwm = %d \n", ch, timer->pwm_duty_cycle[ch]);
    
    rt_kprintf("Get the ch No.%d duty cycle ...\n", hwt4.ch);
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_GET_DUTY_CYCLE, &hwt4);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt4.ch);
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Get ch = %d pwm = %d ok,\n", hwt4.ch, hwt4.count);
    }
    
    hwt4.count += 100;
    rt_kprintf("Set the ch No.%d duty cycle ...\n", hwt4.ch);
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_SET_DUTY_CYCLE, &hwt4);
    if (err != RT_EOK)
    {
        rt_kprintf("Set ch  = %d pwm Fail\n", hwt4.ch);
        goto EXIT_TIM4;
    }
    else
    {
        rt_kprintf("Set ch = %d pwm = %d ok,\n", hwt4.ch, hwt4.count);
    }
    
    rt_kprintf("The timer will work on %d sec.\n", t/2);
    rt_thread_delay(t*RT_TICK_PER_SECOND/2 + 1);
    
    /* stop the timer */
    err = rt_device_control(dev_hwtimer4, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev_hwtimer4, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT_TIM4:
    err = rt_device_close(dev_hwtimer4);
    rt_kprintf("Close %s\n", TIMER4);
    
#endif /*RT_USING_HWTIM4*/     

    return err;

}

FINSH_FUNCTION_EXPORT(hwtimer, Test hardware timer);
#endif /* RT_USING_HWTIMER_TEST */
#endif /* RT_USING_HWTIMER */
