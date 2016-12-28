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

lwc_cure_display_t lcd;
lwc_cure_output_t lco;

ALIGN(RT_ALIGN_SIZE)
rt_uint8_t lwc_button_stack[ 1024 ];
struct rt_thread lwc_button_thread;

void lwc_button_thread_entry(void* parameter)
{
 #if 0   
    rt_uint8_t val,vcno;
    rt_device_t dev_button = RT_NULL;
    rt_device_t dev_xtp = RT_NULL;
    //rt_uint32_t mb_msg[4];
    rt_uint8_t flag_timer_set = 0;
    rt_uint8_t flag_voice_close = 0;

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
    
 #endif   
    while (1)
    {
#if 0        
        /* read the button value */
        rt_device_read(dev_button, 0, &val, sizeof(val));        
        //rt_kprintf("Read button = %x \n", val);                 
        switch(val)
        {
           
            case BUTTON_DY_DS:/* 0x12 电源 定时 */
            {
                         
                lcd.lreg.btn.button_dyds++;
                if(( lcd.lreg.btn.button_dyds > 4)||(1 == flag_timer_set) )
                {
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 31;/* 治疗结束，请关断电源，谢谢使用，祝您早日康复 */                   
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));       
                    }
                    lcd.lreg.btn.button_dyds = 0;
                    flag_timer_set = 0;
                    lcd.lway[SET_TIMER].status = LWC_INACTIVE;
                } 
                else if(1 == lcd.lreg.btn.button_dyds) 
                {
                    lcd.lway[SET_TIMER].status = LWC_ACTIVED;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 62;/* 欢迎使用光波康复理疗仪，请定时 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));    
                    }                        
                } 
                else if(2 == lcd.lreg.btn.button_dyds) 
                {
                    lcd.lreg.tval.tmr_value = 10;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 1;/* 10分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));  
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 4;/* 请设置治疗方式 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                }
                else if(3 == lcd.lreg.btn.button_dyds) 
                {
                    lcd.lreg.tval.tmr_value = 20;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 2;/* 20分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 4;/* 请设置治疗方式 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                }
                else if(4 == lcd.lreg.btn.button_dyds) 
                {
                    lcd.lreg.tval.tmr_value = 30;
                    if(0 == flag_voice_close)
                    {
                        vcno = 0x5A + 3;/* 30分钟 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        rt_thread_delay(1*RT_TICK_PER_SECOND );
                        vcno = 0x5A + 4;/* 请设置治疗方式 */
                        rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                    }
                }                               
            }
            break;
            case BUTTON_JG:/* 0x22 激光 */
            {
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    flag_timer_set = 1;              
                    lcd.lreg.btn.button_jg++;
                    if( lcd.lreg.btn.button_jg > 3)
                    {    
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 28;/* 激光治疗关闭 */                   
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));   
                        }                        
                        lcd.lreg.btn.button_jg = 0;
                        lcd.lway[LASER_CURE].status = LWC_INACTIVE;
                    } 
                    else if(1 == lcd.lreg.btn.button_jg) 
                    {
                        lcd.lway[LASER_CURE].status = LWC_ACTIVED;   
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 48;/* 激光治疗输出强度 弱档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }                        
                        
                    } 
                    else if(2 == lcd.lreg.btn.button_jg) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 49;/* 激光治疗输出强度 中档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                    }
                    else if(3 == lcd.lreg.btn.button_jg) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 50;/* 激光治疗输出强度 强档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    } 
                }                
            }
            break;
            case BUTTON_RL:/* 0x11 热疗 */
            {
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    flag_timer_set = 1;             
                    lcd.lreg.btn.button_rl++;
                    if( lcd.lreg.btn.button_rl > 3)
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 8;/* 热疗停止 */                   
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));     
                        }                        
                        lcd.lreg.btn.button_rl = 0;
                        lcd.lway[HEAT_CURE].status = LWC_INACTIVE;
                    } 
                    else if(1 == lcd.lreg.btn.button_rl) 
                    {                       
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 5;/* 低温热疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));                              
                        }
                        lcd.lway[HEAT_CURE].status = LWC_ACTIVED;  
                    } 
                    else if(2 == lcd.lreg.btn.button_rl) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 6;/* 中温热疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                    }
                    else if(3 == lcd.lreg.btn.button_rl) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 7;/* 高温热疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    } 
                }                
            }
            break;            
            case BUTTON_LZLF:/* 0x28 离子治疗 */
            {
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    flag_timer_set = 1;               
                    lcd.lreg.btn.button_lzlf++;
                    if( lcd.lreg.btn.button_lzlf > 3)
                    { 
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 47;/* 超音波治疗关闭 */                   
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }                            
                        lcd.lreg.btn.button_lzlf = 0;
                        lcd.lway[IONICE_CURE].status = LWC_INACTIVE;
                    } 
                    else if(1 == lcd.lreg.btn.button_lzlf) 
                    {   
                        lcd.lway[IONICE_CURE].status = LWC_ACTIVED;     
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 44;/* 超音波治疗输出强度 弱档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }                                      
                    } 
                    else if(2 == lcd.lreg.btn.button_lzlf) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 45;/* 超音波治疗输出强度 中档 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }
                    }
                    else if(3 == lcd.lreg.btn.button_lzlf) 
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
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    flag_timer_set = 1;            
                    lcd.lreg.btn.button_gn++;
                    if( lcd.lreg.btn.button_gn > 10)
                    {
                        lcd.lreg.btn.button_gn = 1;
                    } 
                    if(1 == lcd.lreg.btn.button_gn) 
                    {   
                        lcd.lway[FUNCTION].status = LWC_ACTIVED; 
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 9; /* 全功能 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));   
                        }                            
                    } 
                    else if(2 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 10;/* 中频治疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));  
                        }                            
                    }
                    else if(3 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 14;/* 针灸 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }                            
                    } 
                    else if(4 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 15;/* 拍打 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }                            
                    } 
                    else if(5 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 12;/* 推拿 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno));
                        }                            
                    } 
                    else if(6 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 17;/* 按摩 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    } 
                    else if(7 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 13;/* 拔罐 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                    else if(8 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 41;/* 足疗 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                    else if(9 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 42;/* 减肥 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                    else if(10 == lcd.lreg.btn.button_gn) 
                    {
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 38;/* 音频 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
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
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    lcd.lreg.btn.button_sd++;
                    if(1 < lcd.lreg.btn.button_sd)
                    {
                        lcd.lreg.btn.button_sd = 0;
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
                
                lcd.lreg.btn.button_jy++;
                if(1 < lcd.lreg.btn.button_jy)
                {
                    flag_voice_close = 0;
                    lcd.lreg.btn.button_jy = 0;
                    
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
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    if(LWC_ACTIVED == lcd.lway[FUNCTION].status)
                    {                    
                        if(23 < (++lcd.lreg.btn.button_zl1))
                        {
                            lcd.lreg.btn.button_zl1 = 23;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 19;/* 治疗输出强度1 增加 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                }
            }
            break;
            case BUTTON_ZL2_INC:/* 0x18 治疗2+ */
            {
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    if(LWC_ACTIVED == lcd.lway[FUNCTION].status) 
                    {
                        if(23 < (++lcd.lreg.btn.button_zl2))
                        {
                            lcd.lreg.btn.button_zl1 = 23;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 21;/* 治疗输出强度2 增加 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                }
            }
            break;                    
            case BUTTON_ZL1_DEC:/* 0x34 治疗1- */
            {
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    if(LWC_ACTIVED == lcd.lway[FUNCTION].status)
                    {
                        if(0 < lcd.lreg.btn.button_zl1 )
                        {
                            lcd.lreg.btn.button_zl1--;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 20;/* 治疗输出强度1 减少 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                }
            }
            break;
            case BUTTON_ZL2_DEC:/* 0x38 治疗2- */
            {
                if(1 < lcd.lreg.btn.button_dyds)
                {
                    if(LWC_ACTIVED == lcd.lway[FUNCTION].status) 
                    {
                        if(0 < lcd.lreg.btn.button_zl2 )
                        {
                            lcd.lreg.btn.button_zl2--;
                        }
                        if(0 == flag_voice_close)
                        {
                            vcno = 0x5A + 22;/* 治疗输出强度2 减少 */
                            rt_device_write(dev_xtp, 0, &vcno, sizeof(vcno)); 
                        }
                    }
                }
            }
            break;
            default:
            break;
        }
 #endif                   						
        rt_thread_delay( RT_TICK_PER_SECOND/100 );
    }
}


/*@}*/
