#ifndef _W25N04_H_
#define _W25N04_H_

#include "zf_common_headfile.h"

// W25N04KV SPI ����
// ����DMAͨ��
#define W25N04_DMA_CH        IfxDma_ChannelId_7   // ʹ��DMAͨ��7�����жϴ�����һ��

#define W25N04_SPI              SPI_3                      // ����ʹ�õ�SPI��
#define W25N04_SPI_SPEED        (100 * 1000 * 1000)         // 60MHz SPI�ٶȣ����104MHz������TC264Dƽ̨���ƽϵͣ�
#define W25N04_SPI_MODE         SPI_MODE0                  // SPIģʽ0 CPOL=0 CPHA=0

// W25N04KV ���Ŷ���
#define W25N04_SCK_PIN          SPI3_SCLK_P22_3            // SCK����
#define W25N04_MOSI_PIN         SPI3_MOSI_P22_0            // MOSI����
#define W25N04_MISO_PIN         SPI3_MISO_P22_1            // MISO����
#define W25N04_CS_PIN           P22_2                      // CSƬѡ���� ������������
#define W25N04_WP_PIN           P23_0                      // WPд�������ţ����������
#define W25N04_HOLD_PIN         P23_1                      // HOLD�������ţ����������

// W25N04KV ҳ��С����������
#define W25N04_PAGE_SIZE        2112    // ʵ��ҳ��С�������������������� (2048+64)
#define W25N04_DATA_SIZE        2048    // ��������С
#define W25N04_SPARE_SIZE       64      // ��������С
#define W25N04_BLOCK_SIZE       (64*W25N04_PAGE_SIZE)    // ÿ���С (64ҳ����128KB)
#define W25N04_BLOCKS_PER_DIE   4096    // ÿ��Die�Ŀ���
#define W25N04_TOTAL_PAGES      262144  // ��ҳ�� (4Gbit/8bit=512MB)

// W25N04KV ָ���붨��
#define W25N04_CMD_RESET                    0xFF    // �豸��λ
#define W25N04_CMD_RESET_ENABLE             0x66    // �������ù���
#define W25N04_CMD_RESET_DEVICE             0x99    // �����豸(����������һ��ʹ��)
#define W25N04_CMD_READ_JEDEC_ID            0x9F    // ��ȡ JEDEC ID (������1�ֽ�+�豸ID2�ֽ�)
#define W25N04_CMD_READ_SR                  0x05    // ��ȡ״̬�Ĵ���
#define W25N04_CMD_WRITE_SR                 0x01    // д��״̬�Ĵ��� (+1�ֽ�����)
#define W25N04_CMD_WRITE_ENABLE             0x06    // дʹ��
#define W25N04_CMD_WRITE_DISABLE            0x04    // д��ֹ
#define W25N04_CMD_BLOCK_ERASE_128KB        0xD8    // ����� 128KB (+3�ֽڵ�ַ)
#define W25N04_CMD_LOAD_PROGRAM_DATA        0x02    // ���ر������ (+2�ֽ��е�ַ) - ����ҳ������
#define W25N04_CMD_LOAD_RANDOM_PROGRAM_DATA 0x84    // ������ر������ (+2�ֽ��е�ַ) - ������ҳ������
#define W25N04_CMD_PROGRAM_EXECUTE          0x10    // ִ�б�� (+3�ֽ�ҳ��ַ)
#define W25N04_CMD_PAGE_DATA_READ           0x13    // ҳ���ݶ�ȡ�������� (+3�ֽ�ҳ��ַ)
#define W25N04_CMD_READ_DATA                0x03    // �ӻ�������ȡ���� (+2�ֽ��е�ַ + 1�ֽڿ��ֽ�)
#define W25N04_CMD_FAST_READ                0x0B    // ���ٴӻ�������ȡ���� (+2�ֽ��е�ַ + 1�ֽڿ��ֽ�)
#define W25N04_CMD_FAST_READ_DUAL_OUTPUT    0x3B    // ���ٶ�ȡ˫���
#define W25N04_CMD_FAST_READ_QUAD_OUTPUT    0x6B    // ���ٶ�ȡ�����
#define W25N04_CMD_FAST_READ_DUAL_IO        0xBB    // ���ٶ�ȡ˫I/O
#define W25N04_CMD_FAST_READ_QUAD_IO        0xEB    // ���ٶ�ȡ��I/O
#define W25N04_CMD_DEEP_POWER_DOWN          0xB9    // ������ȵ���ģʽ
#define W25N04_CMD_RELEASE_DEEP_POWER_DOWN  0xAB    // �ͷ���ȵ���ģʽ/��ȡ���ֽ��豸ID

#define W25N04_CMD_GET_FEATURES             0x0F    // ��ȡ���ܼĴ��� (+1�ֽڹ��ܵ�ַ)
#define W25N04_CMD_SET_FEATURES             0x1F    // ���ù��ܼĴ��� (+1�ֽڹ��ܵ�ַ + 1�ֽ�����)

// ���ܼĴ�����ַ
#define W25N04_SR1_ADDR                 0xA0    // ״̬�Ĵ���1��ַ
#define W25N04_SR2_ADDR                 0xB0    // ״̬�Ĵ���2��ַ
#define W25N04_SR3_ADDR                 0xC0    // ״̬�Ĵ���3��ַ
#define W25N04_ECC_DETECT_ADDR          0x10    // ECCλ��ת��������ַ
#define W25N04_ECC_DETECT_STATUS_ADDR   0x20    // ECCλ��ת�������״̬��ַ
#define W25N04_ECC_MAX_BF_ADDR          0x30    // ECC���λ��ת���������ַ
#define W25N04_ECC_BF_REPORT_L_ADDR     0x40    // ECCλ��ת����������ֽڵ�ַ
#define W25N04_ECC_BF_REPORT_H_ADDR     0x50    // ECCλ��ת����������ֽڵ�ַ

// ״̬�Ĵ���1 (SR1) λ���� - ��ַA0h
#define W25N04_SR1_SRP0         (1 << 0)    // λ0: ״̬�Ĵ�������λ0
#define W25N04_SR1_WP_E         (1 << 1)    // λ1: д����ʹ��λ
#define W25N04_SR1_TB           (1 << 2)    // λ2: �鱣������ѡ�� (0=����, 1=�ײ�)
#define W25N04_SR1_BP0          (1 << 3)    // λ3: �鱣��λ0
#define W25N04_SR1_BP1          (1 << 4)    // λ4: �鱣��λ1
#define W25N04_SR1_BP2          (1 << 5)    // λ5: �鱣��λ2
#define W25N04_SR1_BP3          (1 << 6)    // λ6: �鱣��λ3
#define W25N04_SR1_SRP1         (1 << 7)    // λ7: ״̬�Ĵ�������λ1

// ״̬�Ĵ���2 (SR2) λ���� - ��ַB0h
#define W25N04_SR2_BUF          (1 << 0)    // λ0: ��ȡģʽ (0=˳���ȡ, 1=��������ȡ[Ĭ��])
#define W25N04_SR2_H_DIS        (1 << 1)    // λ1: ���ֹ��ܽ���
#define W25N04_SR2_ODS0         (1 << 2)    // λ2: �������ǿ��λ0
#define W25N04_SR2_ODS1         (1 << 3)    // λ3: �������ǿ��λ1
#define W25N04_SR2_ECC_E        (1 << 4)    // λ4: ECCʹ�� (0=����, 1=ʹ��[Ĭ��])
#define W25N04_SR2_SR1_L        (1 << 5)    // λ5: ״̬�Ĵ���1����λ
#define W25N04_SR2_OTP_E        (1 << 6)    // λ6: ����OTP����ģʽλ
#define W25N04_SR2_OTP_L        (1 << 7)    // λ7: һ���Ա������λ

// ״̬�Ĵ���3 (SR3) λ���� - ��ַC0h
#define W25N04_SR3_BUSY         (1 << 0)    // λ0: æ��־ (0=����, 1=æ)
#define W25N04_SR3_WEL          (1 << 1)    // λ1: дʹ������ (0=����, 1=ʹ��)
#define W25N04_SR3_E_FAIL       (1 << 2)    // λ2: ����ʧ�ܱ�־ (0=�ɹ�, 1=ʧ��)
#define W25N04_SR3_P_FAIL       (1 << 3)    // λ3: ���ʧ�ܱ�־ (0=�ɹ�, 1=ʧ��)
#define W25N04_SR3_ECCS0        (1 << 4)    // λ4: ECC״̬λ0
#define W25N04_SR3_ECCS1        (1 << 5)    // λ5: ECC״̬λ1
// ECCS[1:0]: 00=�ɹ�/����ECC, 01=1λ����ɹ�, 10=2λ����ɹ�(��ȷ��), 11=�޷�����

// �������ǿ�ȶ���
#define W25N04_ODS_100_PERCENT   0x00    // 00: 100%(Ĭ��)
#define W25N04_ODS_75_PERCENT    0x04    // 01: 75%
#define W25N04_ODS_50_PERCENT    0x08    // 10: 50%
#define W25N04_ODS_25_PERCENT    0x0C    // 11: 25%

// �����̺��豸ID
#define W25N04_MANUFACTURER_ID       0xEF    // Winbond������ID
#define W25N04_DEVICE_ID             0xAA23  // W25N04KV�豸ID

// ����ֵ����
#define W25N04_BLOCK_COUNT           4096    // �ܿ���
#define W25N04_PAGES_PER_BLOCK       64      // ÿ��ҳ��
#define W25N04_MAX_COLUMN_ADDR       0x0FFF  // ����е�ַ (12λ)
#define W25N04_TIMEOUT_PAGE_READ     25      // ҳ��ȡ��ʱ(us)
#define W25N04_TIMEOUT_PAGE_PROG     700     // ҳ��̳�ʱ(us)
#define W25N04_TIMEOUT_BLOCK_ERASE   10000   // �������ʱ(us)


/**
 * @brief ��ʼ��W25N04KV����
 * 
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_init(void);

/**
 * @brief ��ȡW25N04KV��JEDEC ID
 * 
 * @param manufacturer_id �洢������ID��ָ��
 * @param device_id �洢�豸ID��ָ��
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_read_id(uint8 *manufacturer_id, uint16 *device_id);

/**
 * @brief ��ȡW25N04KV��״̬�Ĵ���
 * 
 * @param reg_addr �Ĵ�����ַ(0xA0/0xB0/0xC0)
 * @return uint8 ״̬�Ĵ���ֵ
 */
uint8 w25n04_read_status(uint8 reg_addr);

/**
 * @brief �ȴ�W25N04KV��æ
 * 
 * @param timeout_ms ��ʱʱ��(����)��0��ʾһֱ�ȴ�ֱ����æ
 * @return uint8 0:��ʱ��1:�ɹ�(�豸��æ)
 */
uint8 w25n04_wait_busy(uint32 timeout_ms);

/**
 * @brief ϵͳ��λW25N04KVоƬ
 * 
 * @param reset_mode ��λģʽ��0-ʹ�õ�һ��λָ��(0xFF)��1-ʹ�ø�λʹ��+��λ�豸ָ��(0x66/0x99)
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_reset(uint8 reset_mode);

/**
 * @brief ���W25N04KV��д����
 * 
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_disable_write_protection(void);

/**
 * @brief ����ָ����128KB��
 * 
 * @param block_addr ���ַ��0-4095��
 * @return uint8 0:ʧ�ܣ�1:�ɹ���2:дʹ��ʧ�ܣ�3:����ʧ��(E_FAILλ��1)
 */
uint8 w25n04_block_erase(uint16 block_addr);

/**
 * @brief ��ȡָ��ҳ������
 * 
 * @param page_addr ҳ��ַ
 * @param column_addr �е�ַ
 * @param data ��ȡ���ݵĴ洢λ��
 * @param len Ҫ��ȡ�����ݳ���
 * @return uint8 0:ʧ�ܣ�1:�ɹ���ECC����2:�ɹ�����ECC����ɾ�����3:ECC�����޷�����
 */
uint8 w25n04_read_page(uint32 page_addr, uint16 column_addr, uint8 *data, uint16 len);

/**
 * @brief д�����ݵ�ָ��ҳ
 * 
 * @param page_addr ҳ��ַ
 * @param column_addr �е�ַ
 * @param data Ҫд������ݻ�����
 * @param len ���ݳ���
 * @return uint8 0:ʧ�ܣ�1:�ɹ���2:����ִ��ʧ��
 */
uint8 w25n04_write_page(uint32 page_addr, uint16 column_addr, uint8 *data, uint16 len);

/**
 * @brief ����������ݵ��ڲ�ҳ������(�����û�����)
 * 
 * @param column_addr �е�ַ(0-2111)
 * @param data Ҫ���ص�����
 * @param len ���ݳ���
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_random_program_data_load(uint16 column_addr, uint8 *data, uint16 len);

/**
 * @brief дʹ��
 * 
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_write_enable(void);

/**
 * @brief �ر�W25N04KV��ECC����
 * 
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
 */
uint8 w25n04_disable_ecc(void);

#endif /* _W25N04_H_ */ 
