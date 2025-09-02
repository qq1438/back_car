#include "exposure_adjust.h"
#include "zf_driver_flash.h"
#include "zf_device_mt9v03x.h"
#include "key.h"

void ExposureTime_flash_load(uint16 *exp_time)
{
    if (exp_time == NULL) return;
    // 从内部 Flash 读取数据到缓冲区
    flash_read_page_to_buffer(0, EXPOSURE_FLASH_PAGE);
    uint16 read_val = flash_union_buffer[0].uint16_type;
    // 检查读取值的有效性
    if (read_val < 1 || read_val > 10000)
    {
        *exp_time = MT9V03X_EXP_TIME_DEF;
    }
    else
    {
        *exp_time = read_val;
    }
}

void ExposureTime_flash_save(uint16 exp_time)
{
    // 准备缓冲区
    flash_buffer_clear();
    flash_union_buffer[0].uint16_type = exp_time;
    // 擦除 Flash 页
    flash_erase_page(0, EXPOSURE_FLASH_PAGE);
    // 写入缓冲区到 Flash
    flash_write_page_from_buffer(0, EXPOSURE_FLASH_PAGE);
}

// 曝光时间调整功能实现
void exposure_time_adjust(void)
{
    uint16 exp_time; // 从内部 Flash 加载曝光时间
    ExposureTime_flash_load(&exp_time);
    uint8 key = 0;
    bool exit_flag = false;

    // 清屏并显示标题
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(20, 10, "Exposure Time Adjust");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // 显示当前曝光时间
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(20, 130, "Current Value:");
    ips200_show_uint(150, 130, exp_time, 5);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // 应用初始曝光时间
    mt9v03x_set_exposure_time(exp_time);

    key_flag = 0;
    // 调整循环
    while (!exit_flag)
    {
        // 显示当前图像
        if (mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            ips200_show_gray_image(0, 0, (uint8 *)mt9v03x_image,
                                   MT9V03X_W, MT9V03X_H,
                                   MT9V03X_W, MT9V03X_H, 0);
        }

        // 检查按键
        if (key_flag != 0)
        {
            key = key_flag;
            key_flag = 0;

            switch (key)
            {
            case Button_1:  // 增加曝光时间
                if (exp_time < 800)
                {
                    exp_time += 1;
                    mt9v03x_set_exposure_time(exp_time);
                    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
                    ips200_show_uint(150, 130, exp_time, 5);
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                }
                break;

            case Button_2:  // 减少曝光时间
                if (exp_time > 10)
                {
                    exp_time -= 1;
                    mt9v03x_set_exposure_time(exp_time);
                    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
                    ips200_show_uint(150, 130, exp_time, 5);
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                }
                break;

            case Button_3:  // 保存并退出
                mt9v03x_set_reg(0xAB, exp_time);
                ExposureTime_flash_save(exp_time);
                ips200_set_color(RGB565_GREEN, RGB565_BLACK);
                ips200_show_string(20, 150, "Settings Saved!");
                ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                system_delay_ms(500);
                exit_flag = true;
                break;

            case Button_4:  // 取消并退出
                mt9v03x_set_exposure_time(MT9V03X_EXP_TIME_DEF);
                ips200_set_color(RGB565_RED, RGB565_BLACK);
                ips200_show_string(20, 150, "Changes Cancelled!");
                ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                system_delay_ms(500);
                exit_flag = true;
                break;

            default:
                break;
            }
        }

        system_delay_ms(10);
    }

    // 模拟返回按钮，退出到上级菜单
    key_flag = Button_4;
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(20, 100, "Returning to Menu...");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    system_delay_ms(500);
} 