#include "infrared_nec.h"
#include "zf_driver_gpio.h"
#include "zf_driver_timer.h"

#define IR_PIN P10_6

/**
 * @brief 测量P10_6引脚保持指定电平的时间长度，单位为微秒
 * @param level 要测量的电平（0或1）
 * @return 持续时间，单位us
 */
static uint32 IR_MeasurePulseLevel(uint8 level)
{
    uint32 start = system_getval_us();
    while (gpio_get_level(IR_PIN) == level);
    return system_getval_us() - start;
}

void NEC_Init(void)
{
    // 启动系统定时器并将P10_6配置为上拉输入
    system_start();
    gpio_init(IR_PIN, GPI, 1, GPI_PULL_UP);
}

bool IR_Read(void)
{
    return gpio_get_level(IR_PIN);
}

bool NEC_Receive(uint16 *data)
{
    uint32 duration;

    // 等待前导高电平结束
    duration = IR_MeasurePulseLevel(1);
    if (duration > 100000) {
        return false;
    }

    // 起始低脉冲 ~9ms
    duration = IR_MeasurePulseLevel(0);
    if (duration < 8500 || duration > 9500) {
        return false;
    }

    // 起始高脉冲 ~4.5ms
    duration = IR_MeasurePulseLevel(1);
    if (duration < 4000 || duration > 5000) {
        return false;
    }

    uint32 raw = 0;
    for (int i = 0; i < 32; i++) {
        // 数据位低脉冲 ~560us
        duration = IR_MeasurePulseLevel(0);
        if (duration < 400 || duration > 700) {
            return false;
        }

        // 数据位高脉冲区分0/1
        duration = IR_MeasurePulseLevel(1);
        if (duration > 1500) {
            raw |= (1UL << i);
        } else if (duration < 400 || duration > 700) {
            return false;
        }
    }

    *data = (uint16)(raw >> 16);
    return true;
} 