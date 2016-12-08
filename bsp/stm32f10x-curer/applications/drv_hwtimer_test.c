#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>


#ifdef RT_USING_HWTIMER

#define TIMER   "timer6"

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
    rt_hwtimer_mode_t mode;
    int freq = 2000;//Hz, the timer out frequence (20Hz~1MHz)
    int t = 10;//sec. the timer work on time
    
    rt_hwtimer_t *timer;
    
    rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    if ((dev = rt_device_find(TIMER)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", TIMER);
        return -1;
    }
    
    timer = (rt_hwtimer_t *)dev;  
    timer->freq = freq;
    timer->prescaler = 1;
    timer->reload = 0;

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", TIMER);
        return -1;
    }
    
    rt_device_set_rx_indicate(dev, timer_timeout_cb);
    /* 计数时钟设置(默认1Mhz或支持的最小计数频率) */
    err = rt_device_control(dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (err != RT_EOK)
    {
        rt_kprintf("Set Freq = %dHz Fail\n", freq);
        goto EXIT;
    }
    else
    {
        rt_kprintf("Set Freq = %dHz ok\n", freq);
    }

    /* 周期模式 */
    mode = HWTIMER_MODE_PERIOD;
    err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    tick = rt_tick_get();
    rt_kprintf("Start Timer> Tick: %d\n", tick);
    /* 设置定时器超时值并启动定时器 */
    val.sec = t;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    if (rt_device_write(dev, 0, &val, sizeof(val)) != sizeof(val))
    {
        rt_kprintf("SetTime Fail\n");
        goto EXIT;
    }
    rt_kprintf("Sleep %d sec\n", t);
    rt_thread_delay(t*RT_TICK_PER_SECOND);

    /* stop the timer */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");
    /* read the counter */
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read: Sec = %d, Usec = %d\n", val.sec, val.usec);

EXIT:
    err = rt_device_close(dev);
    rt_kprintf("Close %s\n", TIMER);

    return err;
}

FINSH_FUNCTION_EXPORT(hwtimer, Test hardware timer);
#endif
