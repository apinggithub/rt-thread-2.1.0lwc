#ifndef __HWTIMER_H__
#define __HWTIMER_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Timer Control Command */
typedef enum
{
    HWTIMER_CTRL_SET_FREQ = 0x01,    /* set the count frequency */
    HWTIMER_CTRL_STOP,               /* stop timer */
    HWTIMER_CTRL_GET_INFO,           /* get a timer feature information */
    HWTIMER_CTRL_SET_MODE,           /* Setting the timing mode(pwm,oc,ic or encoder) */
    HWTIMER_CTRL_GET_AUTORELOAD,     /* get the timer autoreload */
    HWTIMER_CTRL_SET_AUTORELOAD,     /* set the timer autoreload */
    HWTIMER_CTRL_GET_DUTY_CYCLE,     /* get the timer pwm duty cycle */
    HWTIMER_CTRL_SET_DUTY_CYCLE      /* set the timer pwm duty cycle */
} rt_hwtimer_ctrl_t;

/* Timing Mode */
/*typedef enum
{
    HWTIMER_MODE_ONESHOT = 0x01,
    HWTIMER_MODE_PERIOD
} rt_hwtimer_mode_t;*/

/* Timer Channel */
typedef enum
{
    HWTIMER_CH1 = 0x00,    
    HWTIMER_CH2,
    HWTIMER_CH3,
    HWTIMER_CH4    
} rt_hwtimer_ch_t;

/* Time Value */
typedef struct rt_hwtimerval
{
    rt_int32_t sec;      /* second */
    rt_int32_t usec;     /* microsecond */
} rt_hwtimerval_t;

typedef struct rt_hwtimercnt
{
    rt_int8_t ch;      /* channel no. */
    rt_int16_t count;  /* counter */
} rt_hwtimercnt_t;

#define HWTIMER_CNTMODE_UP      0x01 /* increment count mode */
#define HWTIMER_CNTMODE_DW      0x02 /* decreasing count mode */


typedef struct rt_device_hwtimer
{
    struct rt_device parent;
    const struct rt_hwtimer_ops *ops;
    const struct rt_hwtimer_info *info;

    rt_int32_t freq;                /* counting frequency(Hz) set by the user */
    rt_uint16_t prescaler;          /* timer prescaler */
    rt_int16_t reload;              /* reload cycles(using in period mode) */
    
    rt_uint8_t channel_no[4];       /* the timer channel no.*/   
    rt_uint8_t channel_type[4];     /* the timer work type ic,oc,pwm,and encode */    
    rt_int16_t pwm_duty_cycle[4];   /* the pwm duty cycle */
         
    rt_int32_t overflow;            /* timer overflows */  
    rt_int32_t cycles;              /* how many times will generate a timeout event after overflow */
    
    //rt_hwtimer_mode_t mode;         /* timing mode(oneshot/period) */
} rt_device_hwtimer_t;

struct rt_hwtimer_ops
{
    void (*init)( rt_device_hwtimer_t *timer, rt_uint32_t state);
    rt_err_t (*start)(rt_device_hwtimer_t *timer, rt_off_t pos);
    void (*stop)(rt_device_hwtimer_t *timer);
    rt_err_t (*set_prescaler)(rt_device_hwtimer_t *timer,rt_uint32_t val);
    rt_uint32_t (*get_counter)(rt_device_hwtimer_t *timer);
    rt_err_t (*set_counter)(rt_device_hwtimer_t *timer,rt_uint32_t val);
    rt_uint32_t (*get_autoreload)(rt_device_hwtimer_t *timer);
    rt_err_t (*set_autoreload)(rt_device_hwtimer_t *timer,rt_uint32_t val);
    rt_uint32_t (*get_compare)(rt_device_hwtimer_t *timer,rt_uint8_t ch);
    rt_err_t (*set_compare)(rt_device_hwtimer_t *timer,rt_uint8_t ch,rt_uint32_t val);
    rt_err_t (*control)(rt_device_hwtimer_t *timer, rt_uint32_t cmd, void *args);
};

/* Timer Feature Information */
struct rt_hwtimer_info
{
    rt_uint32_t maxfreq;    /* the maximum count frequency timer support */
    rt_uint32_t minfreq;    /* the minimum count frequency timer support */
    rt_uint32_t maxcnt;    /* counter maximum value */
    rt_uint8_t  cntmode;   /* count mode (inc/dec) */
};



rt_err_t rt_device_hwtimer_register(rt_device_hwtimer_t *timer, const char *name, void *user_data);
void rt_device_hwtimer_isr(rt_device_hwtimer_t *timer);

#ifdef __cplusplus
}
#endif

#endif
