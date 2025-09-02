#include "key.h"

volatile uint8 key_flag = 0;
bool Last_IR_Flag = 1;
volatile bool nec_stop_flag = false;
void my_key_init(void)
{
    key_init(5);
    pit_ms_init(CCU60_CH1,5);
}

void key_scan(void)
{
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS || key_get_state(KEY_1) == KEY_LONG_PRESS)
    {
        key_flag = Button_1;
    }
    else if(key_get_state(KEY_2) == KEY_SHORT_PRESS || key_get_state(KEY_2) == KEY_LONG_PRESS)
    {
        key_flag = Button_2;
    }   
    else if(key_get_state(KEY_3) == KEY_SHORT_PRESS || key_get_state(KEY_3) == KEY_LONG_PRESS)
    {
        key_flag = Button_3;
    }
    else if(key_get_state(KEY_4) == KEY_SHORT_PRESS || key_get_state(KEY_4) == KEY_LONG_PRESS)
    {
        key_flag = Button_4;
    }
}


IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH1);
    key_scanner();
    key_scan();
    
    bool flag = IR_Read();
    if(!flag && !Last_IR_Flag)
    {
        pit_disable(CCU60_CH1);
        pwm_set_duty(ATOM0_CH4_P02_4, 0);
        pwm_set_duty(ATOM0_CH6_P02_6, 0);
        nec_stop_flag = true;
    }
    Last_IR_Flag = flag;
}




