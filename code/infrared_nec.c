#include "infrared_nec.h"
#include "zf_driver_gpio.h"
#include "zf_driver_timer.h"

#define IR_PIN P10_6

/**
 * @brief ����P10_6���ű���ָ����ƽ��ʱ�䳤�ȣ���λΪ΢��
 * @param level Ҫ�����ĵ�ƽ��0��1��
 * @return ����ʱ�䣬��λus
 */
static uint32 IR_MeasurePulseLevel(uint8 level)
{
    uint32 start = system_getval_us();
    while (gpio_get_level(IR_PIN) == level);
    return system_getval_us() - start;
}

void NEC_Init(void)
{
    // ����ϵͳ��ʱ������P10_6����Ϊ��������
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

    // �ȴ�ǰ���ߵ�ƽ����
    duration = IR_MeasurePulseLevel(1);
    if (duration > 100000) {
        return false;
    }

    // ��ʼ������ ~9ms
    duration = IR_MeasurePulseLevel(0);
    if (duration < 8500 || duration > 9500) {
        return false;
    }

    // ��ʼ������ ~4.5ms
    duration = IR_MeasurePulseLevel(1);
    if (duration < 4000 || duration > 5000) {
        return false;
    }

    uint32 raw = 0;
    for (int i = 0; i < 32; i++) {
        // ����λ������ ~560us
        duration = IR_MeasurePulseLevel(0);
        if (duration < 400 || duration > 700) {
            return false;
        }

        // ����λ����������0/1
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