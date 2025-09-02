#include "All_Init.h"
#include "menu.h"
#include "exposure_adjust.h"

float FOC_PWM = 500;
volatile bool Send_state = false;
seekfree_assistant_oscilloscope_struct oscilloscope_data;

void send_signel(void)
{
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_DEBUG_UART);

    oscilloscope_data.data[0] = 0;
    oscilloscope_data.data[1] = 0;
    oscilloscope_data.data[2] = 0;
    oscilloscope_data.data[3] = 0;
    oscilloscope_data.data[4] = 0;

    oscilloscope_data.data[5] = 0;
    oscilloscope_data.data[6] = 0;
    oscilloscope_data.data[7] = 0;

    oscilloscope_data.channel_num = 8;

}

void send_value(void){
    seekfree_assistant_oscilloscope_send(&oscilloscope_data);
    oscilloscope_data.data[0] = add_speed;
    oscilloscope_data.data[1] = pid_speed;
    oscilloscope_data.data[2] = error[1];
    oscilloscope_data.data[3] = 0;
    oscilloscope_data.data[4] = 0;
    oscilloscope_data.data[5] = 0;
    oscilloscope_data.data[6] = 0;
    oscilloscope_data.data[7] = 0;
}

void Device_Init(void)
{
    system_delay_init();
    system_delay_ms(500);
    ips200_init(IPS200_TYPE_SPI);
    adc_init(ADC0_CH11_A11,ADC_12BIT);
    pwm_init(ATOM2_CH3_P11_6,300,2900);
    PID_Init(&angle_pid);
    PID_Init(&acc_pid);
    PID_Init(&position_pid);
    LADRC_Init(&Lspeed_ladrc, 0.003);
    LADRC_Init(&Rspeed_ladrc, 0.003);
    my_key_init();
    system_start();
    uint16 adc_value = adc_mean_filter_convert(ADC0_CH11_A11,1000);
    float voltage = adc_value * 0.008614748;
    if(voltage < 11.7)
    {
        while(1)
        {
            adc_value = adc_mean_filter_convert(ADC0_CH11_A11,1000);
            voltage = adc_value * 0.008614748;
            if(key_flag == Button_3)
            {
                key_flag = 0;
                break;
            }
            ips200_show_string(20, 80, "NO POWER!");
            ips200_show_string(20, 100, "PRESS BUTTON 3 TO CONTINUE!");
            ips200_show_float(0, 0, voltage,3,3);
        }
    }
    while(imu660ra_init());
    system_delay_ms(500);
    while(mt9v03x_init());
    uint16 boot_exp;
    ExposureTime_flash_load(&boot_exp);
    mt9v03x_set_exposure_time(boot_exp);
    w25n04_init();
    //Parameter_reset_to_default();
    menu_open();
    Parameter_flash_load();
    LADRC_CopyParams(&Rspeed_ladrc, &Lspeed_ladrc);
    pwm_init(ATOM0_CH0_P21_2,50,500);
    pwm_init(ATOM0_CH1_P21_3,50,500);
    for(uint8 i = 0; i <= (FOC_PWM - 500) / 10; i++)
    {
        pwm_set_duty(ATOM0_CH0_P21_2,500 + i * 10);
        pwm_set_duty(ATOM0_CH1_P21_3,500 + i * 10);
        system_delay_ms(20);
    }
    ips200_set_color(RGB565_RED, RGB565_WHITE);
    encoder_quad_init(TIM2_ENCODER,TIM2_ENCODER_CH1_P33_7,TIM2_ENCODER_CH2_P33_6);
    encoder_quad_init(TIM4_ENCODER,TIM4_ENCODER_CH2_P00_9,TIM4_ENCODER_CH1_P02_8);
    gpio_init(P02_7,GPO,1,GPO_PUSH_PULL);   
    gpio_init(P02_5,GPO,0,GPO_PUSH_PULL);  
    pit_ms_init(CCU60_CH0,3);
    pwm_init(ATOM0_CH4_P02_4,17*1000,0);
    pwm_init(ATOM0_CH6_P02_6,17*1000,0);
    //send_signel();
    mt9v03x_finish_flag = 0;
}

uint32 seekfree_assistant_transfer (const uint8 *buff, uint32 length)
{
    uint32 len = 0;

    len = wireless_uart_send_buffer(buff, length);

    return len;
}
