/*
 * File      : gpio.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-03-24     Bright      the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <drv_hwbutton.h>
#include <drivers/hwbutton.h>


#ifdef RT_USING_HWBUTTON


static rt_err_t hw_button_port_init(rt_device_t dev)
{
    
    rt_device_button_t *button = (rt_device_button_t *)dev;

    /* check parameters */
    RT_ASSERT(button != RT_NULL);
    
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
  
    /*Configure GPIO pins : PC0 PC1 PC2 PC3 PC4 PC5 */
    
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3; /*PC0~PC3*/
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_5;/*PC4 PC5*/
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    return RT_EOK;
}

static rt_size_t hw_button_port_read(rt_device_t dev)
{
    //rt_device_port_status_t *stat;
    rt_device_button_t *button = (rt_device_button_t *)dev;
    
    rt_uint8_t rcv = 0,val = 0;
    
    /* check parameters */
    RT_ASSERT(button != RT_NULL);
    
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
     
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);
    
    //rt_thread_delay(RT_TICK_PER_SECOND/100);
       
    //rt_pin_write(44, PIN_HIGH);
    //rt_pin_write(45, PIN_HIGH);
    
    rcv = (uint8_t)((~GPIOC->IDR)&0x0f);   
    if(rcv != 0)  
    {          
        switch(rcv)
        {
            case 0x01:
            {
                 val = 0x11;//S1 RL
            }
            break;
            case 0x02:
            {
                val = 0x12;//S2 DY/DS
            }
            break;
            case 0x04:
            {
                val = 0x14;//S3 ZL+
            }
            break;
            case 0x08:
            {
                val = 0x18;//S4 ZL2+
            }
            break;
            default:
            break;
        }
    }
    else
    {
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_RESET);
        rcv = (uint8_t)((~GPIOC->IDR)&0x0f);
        if(rcv!= 0)  
        {
            switch(rcv)
            {
                case 0x01:
                {
                    val = 0x21;//
                }
                break;
                case 0x02:
                {
                    val = 0x22;//S9 JG
                }
                break;
                case 0x04:
                {
                    val = 0x24;//S10 SD
                }
                break;
                case 0x08:
                {
                    val = 0x28;//S11 CYB
                }
                break;
                default:
                break;
            }
        }
        else
        {
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);
            rcv = (uint8_t)((~GPIOC->IDR)&0x0f); 
            if(rcv != 0)
            {
                switch(rcv)
                {
                    case 0x01:
                    {
                        val = 0x31;//S5 JY
                    }
                    break;
                    case 0x02:
                    {
                        val = 0x32;//S6 GN
                    }
                    break;
                    case 0x04:
                    {
                        val = 0x34;//S7 ZL-
                    }
                    break;
                    case 0x08:
                    {
                        val = 0x38;//S8 ZL2-
                    }
                    break;
                    default:
                    break;
                }               
            }
        }
    }   
    return val;
}


const static rt_port_ops_t _hwbutton_port_ops =
{
    hw_button_port_init,   
    hw_button_port_read,
   
};

int stm32_hw_button_init(void)
{
    int result;
    
    result = rt_device_button_register("button", &_hwbutton_port_ops, RT_NULL);
    return result;
}
INIT_BOARD_EXPORT(stm32_hw_button_init);

#endif /* RT_USING_HWBUTTON */