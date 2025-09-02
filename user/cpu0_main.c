/*********************************************************************************************************************
* TC264 Opensourec Library ����TC264 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ��� TC264 ��Դ���һ����
*
* TC264 ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          cpu0_main
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          ADS v1.8.0
* ����ƽ̨          TC264D
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����              ����                ��ע
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "PTS_Deal.h"
#pragma section all "cpu0_dsram"
// ���������#pragma section all restore���֮���ȫ�ֱ���������CPU0��RAM��

// *************************** ����Ӳ������˵�� ***************************
// ���İ��������缴�� �����������
// ���ʹ��������� �������Ҫ�õ�ع���

// **************************** �������� ****************************
int core0_main(void)
{
    clock_init();                   // ��ȡʱ��Ƶ��<��ر���>
    debug_init();
    // �˴���д�û����� ���������ʼ�������
    NEC_Init();
    // �˴���д�û����� ���������ʼ�������
    cpu_wait_event_ready();         // �ȴ����к��ĳ�ʼ�����
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
// **************************** �������� ****************************

