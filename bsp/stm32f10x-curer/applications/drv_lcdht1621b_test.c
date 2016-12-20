#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>
#include <stdio.h>
#include <drivers/lcdht1621b.h>
#include <drv_lcdht1621b.h>

#ifdef RT_USING_LCDHT1621B

int lcdht(void)
{
    rt_err_t err;   
    rt_device_t devlcdht = RT_NULL;
    
    
    rt_lcdth_ramdat_t val;
    //uint8_t tm = 0;  

    enum lcdcmd;
     
    #define LCD   "lcdht"
       
    //rt_pin_mode(20, PIN_MODE_OUTPUT);// the port PF8
   
    if ((devlcdht = rt_device_find(LCD)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", LCD);
        return -1;
    }
   
    if (rt_device_open(devlcdht, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", LCD);
        return -1;
    }
          
    while(1)
    {
          
        rt_device_control(devlcdht, LCDHT_CMD_FULL_SREEN_OFF, RT_NULL);
        rt_kprintf("full screen the seg lcd OFF.\n");
        rt_thread_delay(RT_TICK_PER_SECOND);
        
        /* write the lcd */
        for(uint8_t i = 0;i < 32;i++)
        {
            val.segno = i;
            val.dat = 0x0f;
            rt_device_write(devlcdht, 0, &val, sizeof(val));
                        
            rt_kprintf("Send to lcd segno = %d , dat = %x \n", val.segno, val.dat);
            
            rt_thread_delay(RT_TICK_PER_SECOND);  

            val.dat = 0x00;
            rt_device_write(devlcdht, 0, &val, sizeof(val));
            rt_kprintf("Send to lcd segno = %d , dat = %x \n", val.segno, val.dat);
            rt_thread_delay(RT_TICK_PER_SECOND);  
            
            if(i == 17)
            {
                i = 30-1;
            }
                                                   
        }
        break;  
        /*
        tm++;
        if(tm > 2)
        {    
            //tm = 0;  
            rt_kprintf("the tm value = %d  \n", tm);              
            break;        
        }
        rt_kprintf("the tm value = %d  \n", tm);              
         */                
        //rt_thread_delay(RT_TICK_PER_SECOND/10);         
                
    } 
    
    rt_device_control(devlcdht, LCDHT_CMD_FULL_SREEN_ON, RT_NULL);
    rt_kprintf("full screen the seg lcd ON.\n");
    rt_thread_delay(RT_TICK_PER_SECOND);  
    
    rt_device_control(devlcdht, LCDHT_CMD_FULL_SREEN_OFF, RT_NULL);
    rt_kprintf("full screen the seg lcd OFF.\n");
    rt_thread_delay(RT_TICK_PER_SECOND); 
    
    rt_device_control(devlcdht, LCDHT_CMD_FULL_SREEN_ON, RT_NULL);
    rt_kprintf("full screen the seg lcd ON.\n");
    rt_thread_delay(RT_TICK_PER_SECOND); 
    
    rt_device_control(devlcdht, LCDHT_CMD_LCDOFF, RT_NULL);
    rt_kprintf("Close the seg lcd .\n");
    rt_device_control(devlcdht, LCDHT_CMD_BKLOFF, RT_NULL);
    rt_kprintf("Close the seg lcd backlight.\n");
    
    err = rt_device_close(devlcdht);
    rt_kprintf("Close %s test.\n", LCD);
   
    return err;

}
FINSH_FUNCTION_EXPORT(lcdht, Test Seg lcd ht1621b);
#endif
