/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          cpu0_main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.8.0
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "PTS_Deal.h"
#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

// *************************** 例程硬件连接说明 ***************************
// 核心板正常供电即可 无需额外连接
// 如果使用主板测试 主板必须要用电池供电

// **************************** 代码区域 ****************************
int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();
    // 此处编写用户代码 例如外设初始化代码等
    NEC_Init();
    // 此处编写用户代码 例如外设初始化代码等
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
	while (TRUE)
	{
       if(is_compress)
       {
           uint8 ret = store_compressed_image();
           if(ret != STORAGE_OK)
           {
                ips200_show_uint(0,0,ret,2);
                pit_disable(CCU60_CH0);
                pwm_set_duty(ATOM0_CH4_P02_4,0);
                pwm_set_duty(ATOM0_CH6_P02_6,0);
                while(1);
           }
           is_compress = false;
       }
           if(nec_stop_flag)
           {
               pit_disable(CCU60_CH0);
               pwm_set_duty(ATOM0_CH4_P02_4,0);
               pwm_set_duty(ATOM0_CH6_P02_6,0);
               while(1)
               {
                    pwm_set_duty(ATOM0_CH0_P21_2,0);
                    pwm_set_duty(ATOM0_CH1_P21_3,0);
                    pwm_set_duty(ATOM0_CH4_P02_4,0);
                    pwm_set_duty(ATOM0_CH6_P02_6,0);
                    pwm_set_duty(ATOM2_CH3_P11_6,2900);
               }
           }
	}
}

#pragma section all restore
// **************************** 代码区域 ****************************

