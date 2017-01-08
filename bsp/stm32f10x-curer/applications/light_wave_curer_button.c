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
#include <drivers/hwtimer.h>
#include <drivers/lcdht1621b.h>
#include <drv_lcdht1621b.h>


#include "drv_led.h"
#include "drv_gpio.h"
#include "drv_hwbutton.h"

#include "light_wave_curer.h"

lwc_cure_t lct;

ALIGN(RT_ALIGN_SIZE)
rt_uint8_t lwc_button_stack[ 1024 ];
struct rt_thread lwc_button_thread;
static struct rt_timer timerdec;

struct rt_event event;/* 事件控制块*/

static uint32_t tmr_init_val = 0;
static uint32_t tmr_count_dec = 0;

/*定时器超时函数*/
static void timeout_dec(void* parameter)
{

	if((tmr_init_val - tmr_count_dec) > 0)
    {
        tmr_count_dec++;
        lct.lreg.tval.tmr_value = (tmr_init_val - tmr_count_dec)/60;
    }
    else
    {
        tmr_count_dec = 0;       
        rt_event_send(&event, RT_EVENT_LWC_TIMER_FINISH_CLOSE);        
    }
}

void lwc_button_thread_entry(void* parameter)
{
  
    rt_uint8_t val,vcno;
    rt_device_t dev_button = RT_NULL;
    rt_device_t dev_xtp = RT_NULL;
    //rt_uint32_t mb_msg[4];    
    rt_uint8_t flag_voice_close = 0;
    rt_uint8_t flag_tmrval_start = 0;
    //rt_uint8_t flag_ion_timer_update = 0;
        
    rt_uint32_t recv_event;
    
    rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
    
    if ((dev_button = rt_device_find(BUTTON)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", BUTTON);
        while(1);
    }
   
    if (rt_device_open(dev_button, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", BUTTON);
        while(1);
    }
    
     /* 打开 XTP 设备 播放提示音 */        
    if ((dev_xtp = rt_device_find(XTP)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", XTP);
        while(1);
    }  
    if (rt_device_open(dev_xtp, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", XTP);
        while(1);
    }
    
    /* 初始化定时器 */
	rt_timer_init(&timerdec, "timerdec", /* 定时器名为timer2 */
	timeout_dec, /* 超时函数回调处理 */
	RT_NULL, /* 超时函数入口参数*/
	1000, /* 定时长度,OS 以Tick为单位,即1000个OS Tick 产生一次超时处理 */
	RT_TIMER_FLAG_PERIODIC); /* 周期性定时 */
	  
    while (1)
    {
        
        if((1 == lct.lreg.tval.tmr_lock)&&(0 == flag_tmrval_start))           
        {
            if(1 <= lct.lreg.btn.button_lzlf)
            {               
                lct.lreg.tval.tmr_value = 30;
                rt_timer_start(&timerions);
                if(0 == flag_voice_close)
                {
                    vcno = 0x5A + 3;/* 30分钟 */
                    rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));                   
                    rt_thread_delay(1*RT_TICK_PER_SECOND );
                    vcno = 0x5A + 44;/* 超音波治疗输出强度 弱档 */
                    rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                }   
                
            }
            
            tmr_init_val = lct.lreg.tval.tmr_value*60;
            rt_timer_start(&timerdec);
            flag_tmrval_start = 1;                                              
            
        }
                    
        if (rt_event_recv(&event, (RT_EVENT_LWC_DEVICE_POWER_CLOSE
                                    |RT_EVENT_LWC_ION_TIME_UPDATE
                                    ),
                           RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                           RT_TICK_PER_SECOND/100, &recv_event) == RT_EOK)
        {
            switch(recv_event)
            {
                case RT_EVENT_LWC_DEVICE_POWER_CLOSE:
                {
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 31;/* 治疗结束，请关断电源，谢谢使用，祝您早日康复 */                   
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));       
                    }
                    rt_timer_stop(&timerdec);
                    rt_timer_stop(&timerions);
                    flag_tmrval_start = 0;
                    lct.lreg.btn.button_dyds = 0;
                    lct.lreg.tval.tmr_lock = 0;
                    lct.lreg.tval.tmr_value = 0;
                    lct.lcf[IONICE_CURE].cure_out_actived = LWC_INACTIVE;   
                    lct.lway[SET_TIMER].status = LWC_INACTIVE;
                }
                break;
                case RT_EVENT_LWC_ION_TIME_UPDATE:
                {
                    //flag_ion_timer_update = 1;
                    lct.lreg.tval.tmr_value = 30;
                    rt_timer_start(&timerions);
                    tmr_init_val = lct.lreg.tval.tmr_value*60;
                    rt_timer_start(&timerdec);
                                             
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 3;/* 30分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));                   
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 44;/* 超音波治疗输出强度 弱档 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                    }                                                                          
                }
                break;
                default:
                break;
            }
        }
       
        /* read the button value */
        rt_device_read(dev_button, 0, &val, sizeof(val));        
        //rt_kprintf("Read button = %x \n", val);                 
        switch(val)
        {
           
            case BUTTON_DY_DS:/* 0x12 电源 定时 */
            {
                         
                lct.lreg.btn.button_dyds++;
                if(( lct.lreg.btn.button_dyds > 4)||(1 == lct.lreg.tval.tmr_lock ))
                {
                    //if(0 == flag_voice_close)
                    //{
                    //    vcno = 0x5A + 31;/* 治疗结束，请关断电源，谢谢使用，祝您早日康复 */                   
                    //    rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));       
                    //}
                    //lct.lreg.btn.button_dyds = 0;
                    //lct.lreg.tval.tmr_lock = 0;
                    //lct.lreg.tval.tmr_value = 0;
                    //lct.lway[SET_TIMER].status = LWC_INACTIVE;
                    rt_event_send(&event, RT_EVENT_LWC_DEVICE_FORCE_CLOSE);
                } 
                else if(1 == lct.lreg.btn.button_dyds) 
                {
                    lct.lway[SET_TIMER].status = LWC_ACTIVED;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 62;/* 欢迎使用光波康复理疗仪，请定时 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));    
                    }
                    rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                } 
                else if(2 == lct.lreg.btn.button_dyds) 
                {
                    lct.lreg.tval.tmr_value = 10;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 1;/* 10分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));  
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 4;/* 请设置治疗方式 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                    rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                }
                else if(3 == lct.lreg.btn.button_dyds) 
                {
                    lct.lreg.tval.tmr_value = 20;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 2;/* 20分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 4;/* 请设置治疗方式 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                    rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                }
                else if(4 == lct.lreg.btn.button_dyds) 
                {
                    lct.lreg.tval.tmr_value = 30;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 3;/* 30分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 4;/* 请设置治疗方式 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                    rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                }                               
            }
            break;
            case BUTTON_JG:/* 0x22 激光 */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    lct.lreg.tval.tmr_lock = 1;              
                    lct.lreg.btn.button_jg++;
                    if( lct.lreg.btn.button_jg > 3)
                    {    
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 28;/* 激光治疗关闭 */                   
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));   
                        }                        
                        lct.lreg.btn.button_jg = 0;
                        lct.lway[LASER_CURE].status = LWC_INACTIVE;
                        rt_event_send(&event, RT_EVENT_LWC_LASER_CURE_CLOSE);
                    } 
                    else if(1 == lct.lreg.btn.button_jg) 
                    {
                        lct.lway[LASER_CURE].status = LWC_ACTIVED;   
                        lct.lreg.tval.tmr_value = 30;
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 48;/* 激光治疗输出强度 弱档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }                        
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                    else if(2 == lct.lreg.btn.button_jg) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 49;/* 激光治疗输出强度 中档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    else if(3 == lct.lreg.btn.button_jg) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 50;/* 激光治疗输出强度 强档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                }                
            }
            break;
            case BUTTON_RL:/* 0x11 热疗 */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    lct.lreg.tval.tmr_lock = 1;         
                    lct.lreg.btn.button_rl++;
                    if( lct.lreg.btn.button_rl > 3)
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 8;/* 热疗停止 */                   
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));     
                        }                        
                        lct.lreg.btn.button_rl = 0;
                        lct.lway[HEAT_CURE].status = LWC_INACTIVE;
                        rt_event_send(&event, RT_EVENT_LWC_HEAT_CURE_CLOSE);
                    } 
                    else if(1 == lct.lreg.btn.button_rl) 
                    {                       
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 5;/* 低温热疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));                              
                        }
                        lct.lway[HEAT_CURE].status = LWC_ACTIVED;  
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                    else if(2 == lct.lreg.btn.button_rl) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 6;/* 中温热疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    else if(3 == lct.lreg.btn.button_rl) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 7;/* 高温热疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                }                
            }
            break;            
            case BUTTON_LZLF:/* 0x28 离子治疗 */
            {
                if(1 < lct.lreg.btn.button_dyds)
                {
                                        
                    lct.lreg.btn.button_lzlf++;
                    if( lct.lreg.btn.button_lzlf > 3)
                    { 
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 47;/* 超音波治疗关闭 */                   
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }                            
                        lct.lreg.btn.button_lzlf = 0;
                        rt_event_send(&event, RT_EVENT_LWC_ION_CURE_CLOSE);
                        lct.lcf[IONICE_CURE].cure_out_actived = LWC_INACTIVE;
                        lct.lway[IONICE_CURE].status = LWC_INACTIVE;
                        
                    } 
                    else if(1 == lct.lreg.btn.button_lzlf) 
                    {
                        if((1 == lct.lreg.tval.tmr_lock)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived)) 
                        {
                            rt_event_send(&event, RT_EVENT_LWC_ION_FUNC_START);    
                        }
                        else
                        {
                            lct.lreg.tval.tmr_lock = 1;
                            lct.lcf[IONICE_CURE].cure_out_actived = LWC_ACTIVED; 
                        }
                        lct.lway[IONICE_CURE].status = LWC_ACTIVED;     
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 44;/* 超音波治疗输出强度 弱档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        } 
                        
                    } 
                    else if(2 == lct.lreg.btn.button_lzlf) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 45;/* 超音波治疗输出强度 中档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                    }
                    else if(3 == lct.lreg.btn.button_lzlf) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 46;/* 超音波治疗输出强度 强档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    } 
                }
            }
            break;
            case BUTTON_GN:/* 0x32 功能 */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    lct.lreg.tval.tmr_lock = 1;           
                    lct.lreg.btn.button_gn++;
                    if( lct.lreg.btn.button_gn > 9)
                    {
                        lct.lreg.btn.button_gn = 1;
                    } 
                    if(1 == lct.lreg.btn.button_gn) 
                    {   
                        lct.lway[FUNCTION].status = LWC_ACTIVED; 
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 9; /* 全功能 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));   
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                    else if(2 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 10;/* 中频治疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));  
                        } 
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    else if(3 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 14;/* 针灸 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);    
                    } 
                    else if(4 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 15;/* 拍打 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                    else if(5 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 12;/* 推拿 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                    else if(6 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 17;/* 按摩 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    } 
                    else if(7 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 13;/* 拔罐 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    else if(8 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 41;/* 足疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    else if(9 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 42;/* 减肥 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    #if 0
                    else if(10 == lct.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 38;/* 音频 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                    #endif
                    if(0 == flag_voice_close)
                    {
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 18;/* 请调节治疗输出强度 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                }               
            }
            break;
            case BUTTON_SD:/* 0x24 锁定 */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(1 <= lct.lreg.btn.button_gn)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    lct.lreg.btn.button_sd++;
                    if(1 < lct.lreg.btn.button_sd)
                    {
                        lct.lreg.btn.button_sd = 0;
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 30;/* 循环 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                    else
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 29;/* 锁定 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                }
            }
            break;            
            case BUTTON_JY:/* 0x31 静音 */
            {
                
                lct.lreg.btn.button_jy++;
                if(1 < lct.lreg.btn.button_jy)
                {
                    flag_voice_close = 0;
                    lct.lreg.btn.button_jy = 0;
                    
                    vcno = 0x5A + 39;/* 语音功能开启 */
                    rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));                         
                }
                else
                {
                    flag_voice_close = 1;                       
                    vcno = 0x5A + 40;/* 语音功能关闭 */
                    rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));                        
                }
                               
            }
            break;
            case BUTTON_ZL1_INC:/* 0x14 治疗1+ */
            { 
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    if(LWC_ACTIVED == lct.lway[FUNCTION].status)
                    {  
                        lct.lreg.btn.button_zl1_dir = 1;
                        if(23 < (++lct.lreg.btn.button_zl1))
                        {
                            lct.lreg.btn.button_zl1 = 23;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 19;/* 治疗输出强度1 增加 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                }
            }
            break;
            case BUTTON_ZL2_INC:/* 0x18 治疗2+ */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    if(LWC_ACTIVED == lct.lway[FUNCTION].status) 
                    {
                        lct.lreg.btn.button_zl2_dir = 1;
                        if(23 < (++lct.lreg.btn.button_zl2))
                        {
                            lct.lreg.btn.button_zl2 = 23;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 21;/* 治疗输出强度2 增加 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                }
            }
            break;                    
            case BUTTON_ZL1_DEC:/* 0x34 治疗1- */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    if(LWC_ACTIVED == lct.lway[FUNCTION].status)
                    {
                        lct.lreg.btn.button_zl1_dir = 0;
                        if(0 < lct.lreg.btn.button_zl1 )
                        {
                            lct.lreg.btn.button_zl1--;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 20;/* 治疗输出强度1 减少 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                }
            }
            break;
            case BUTTON_ZL2_DEC:/* 0x38 治疗2- */
            {
                if((1 < lct.lreg.btn.button_dyds)&&(LWC_INACTIVE == lct.lcf[IONICE_CURE].cure_out_actived))
                {
                    if(LWC_ACTIVED == lct.lway[FUNCTION].status) 
                    {
                        lct.lreg.btn.button_zl2_dir = 0;
                        if(0 < lct.lreg.btn.button_zl2 )
                        {
                            lct.lreg.btn.button_zl2--;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 22;/* 治疗输出强度2 减少 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                        rt_event_send(&event, RT_EVENT_LWC_BUTTON_UPDATE);
                    }
                }
            }
            break;
            default:
            break;
        }
                  						
        rt_thread_delay( RT_TICK_PER_SECOND/100 );
    }
}


/*@}*/
