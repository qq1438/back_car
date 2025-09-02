#ifndef _W25N04_H_
#define _W25N04_H_

#include "zf_common_headfile.h"

// W25N04KV SPI 配置
// 定义DMA通道
#define W25N04_DMA_CH        IfxDma_ChannelId_7   // 使用DMA通道7，与中断处理函数一致

#define W25N04_SPI              SPI_3                      // 定义使用的SPI号
#define W25N04_SPI_SPEED        (100 * 1000 * 1000)         // 60MHz SPI速度（最大104MHz，但是TC264D平台限制较低）
#define W25N04_SPI_MODE         SPI_MODE0                  // SPI模式0 CPOL=0 CPHA=0

// W25N04KV 引脚定义
#define W25N04_SCK_PIN          SPI3_SCLK_P22_3            // SCK引脚
#define W25N04_MOSI_PIN         SPI3_MOSI_P22_0            // MOSI引脚
#define W25N04_MISO_PIN         SPI3_MISO_P22_1            // MISO引脚
#define W25N04_CS_PIN           P22_2                      // CS片选引脚 可以任意配置
#define W25N04_WP_PIN           P23_0                      // WP写保护引脚（上拉输出）
#define W25N04_HOLD_PIN         P23_1                      // HOLD保持引脚（上拉输出）

// W25N04KV 页大小和容量定义
#define W25N04_PAGE_SIZE        2112    // 实际页大小，包括数据区和冗余区 (2048+64)
#define W25N04_DATA_SIZE        2048    // 数据区大小
#define W25N04_SPARE_SIZE       64      // 冗余区大小
#define W25N04_BLOCK_SIZE       (64*W25N04_PAGE_SIZE)    // 每块大小 (64页，共128KB)
#define W25N04_BLOCKS_PER_DIE   4096    // 每个Die的块数
#define W25N04_TOTAL_PAGES      262144  // 总页数 (4Gbit/8bit=512MB)

// W25N04KV 指令码定义
#define W25N04_CMD_RESET                    0xFF    // 设备复位
#define W25N04_CMD_RESET_ENABLE             0x66    // 启用重置功能
#define W25N04_CMD_RESET_DEVICE             0x99    // 重置设备(与启用重置一起使用)
#define W25N04_CMD_READ_JEDEC_ID            0x9F    // 读取 JEDEC ID (制造商1字节+设备ID2字节)
#define W25N04_CMD_READ_SR                  0x05    // 读取状态寄存器
#define W25N04_CMD_WRITE_SR                 0x01    // 写入状态寄存器 (+1字节数据)
#define W25N04_CMD_WRITE_ENABLE             0x06    // 写使能
#define W25N04_CMD_WRITE_DISABLE            0x04    // 写禁止
#define W25N04_CMD_BLOCK_ERASE_128KB        0xD8    // 块擦除 128KB (+3字节地址)
#define W25N04_CMD_LOAD_PROGRAM_DATA        0x02    // 加载编程数据 (+2字节列地址) - 重置页缓冲区
#define W25N04_CMD_LOAD_RANDOM_PROGRAM_DATA 0x84    // 随机加载编程数据 (+2字节列地址) - 不重置页缓冲区
#define W25N04_CMD_PROGRAM_EXECUTE          0x10    // 执行编程 (+3字节页地址)
#define W25N04_CMD_PAGE_DATA_READ           0x13    // 页数据读取到缓冲区 (+3字节页地址)
#define W25N04_CMD_READ_DATA                0x03    // 从缓冲区读取数据 (+2字节列地址 + 1字节空字节)
#define W25N04_CMD_FAST_READ                0x0B    // 快速从缓冲区读取数据 (+2字节列地址 + 1字节空字节)
#define W25N04_CMD_FAST_READ_DUAL_OUTPUT    0x3B    // 快速读取双输出
#define W25N04_CMD_FAST_READ_QUAD_OUTPUT    0x6B    // 快速读取四输出
#define W25N04_CMD_FAST_READ_DUAL_IO        0xBB    // 快速读取双I/O
#define W25N04_CMD_FAST_READ_QUAD_IO        0xEB    // 快速读取四I/O
#define W25N04_CMD_DEEP_POWER_DOWN          0xB9    // 进入深度掉电模式
#define W25N04_CMD_RELEASE_DEEP_POWER_DOWN  0xAB    // 释放深度掉电模式/读取单字节设备ID

#define W25N04_CMD_GET_FEATURES             0x0F    // 读取功能寄存器 (+1字节功能地址)
#define W25N04_CMD_SET_FEATURES             0x1F    // 设置功能寄存器 (+1字节功能地址 + 1字节数据)

// 功能寄存器地址
#define W25N04_SR1_ADDR                 0xA0    // 状态寄存器1地址
#define W25N04_SR2_ADDR                 0xB0    // 状态寄存器2地址
#define W25N04_SR3_ADDR                 0xC0    // 状态寄存器3地址
#define W25N04_ECC_DETECT_ADDR          0x10    // ECC位翻转计数检测地址
#define W25N04_ECC_DETECT_STATUS_ADDR   0x20    // ECC位翻转计数检测状态地址
#define W25N04_ECC_MAX_BF_ADDR          0x30    // ECC最大位翻转计数报告地址
#define W25N04_ECC_BF_REPORT_L_ADDR     0x40    // ECC位翻转计数报告低字节地址
#define W25N04_ECC_BF_REPORT_H_ADDR     0x50    // ECC位翻转计数报告高字节地址

// 状态寄存器1 (SR1) 位定义 - 地址A0h
#define W25N04_SR1_SRP0         (1 << 0)    // 位0: 状态寄存器保护位0
#define W25N04_SR1_WP_E         (1 << 1)    // 位1: 写保护使能位
#define W25N04_SR1_TB           (1 << 2)    // 位2: 块保护区域选择 (0=顶部, 1=底部)
#define W25N04_SR1_BP0          (1 << 3)    // 位3: 块保护位0
#define W25N04_SR1_BP1          (1 << 4)    // 位4: 块保护位1
#define W25N04_SR1_BP2          (1 << 5)    // 位5: 块保护位2
#define W25N04_SR1_BP3          (1 << 6)    // 位6: 块保护位3
#define W25N04_SR1_SRP1         (1 << 7)    // 位7: 状态寄存器保护位1

// 状态寄存器2 (SR2) 位定义 - 地址B0h
#define W25N04_SR2_BUF          (1 << 0)    // 位0: 读取模式 (0=顺序读取, 1=缓冲区读取[默认])
#define W25N04_SR2_H_DIS        (1 << 1)    // 位1: 保持功能禁用
#define W25N04_SR2_ODS0         (1 << 2)    // 位2: 输出驱动强度位0
#define W25N04_SR2_ODS1         (1 << 3)    // 位3: 输出驱动强度位1
#define W25N04_SR2_ECC_E        (1 << 4)    // 位4: ECC使能 (0=禁用, 1=使能[默认])
#define W25N04_SR2_SR1_L        (1 << 5)    // 位5: 状态寄存器1锁定位
#define W25N04_SR2_OTP_E        (1 << 6)    // 位6: 进入OTP访问模式位
#define W25N04_SR2_OTP_L        (1 << 7)    // 位7: 一次性编程锁定位

// 状态寄存器3 (SR3) 位定义 - 地址C0h
#define W25N04_SR3_BUSY         (1 << 0)    // 位0: 忙标志 (0=就绪, 1=忙)
#define W25N04_SR3_WEL          (1 << 1)    // 位1: 写使能锁存 (0=禁用, 1=使能)
#define W25N04_SR3_E_FAIL       (1 << 2)    // 位2: 擦除失败标志 (0=成功, 1=失败)
#define W25N04_SR3_P_FAIL       (1 << 3)    // 位3: 编程失败标志 (0=成功, 1=失败)
#define W25N04_SR3_ECCS0        (1 << 4)    // 位4: ECC状态位0
#define W25N04_SR3_ECCS1        (1 << 5)    // 位5: ECC状态位1
// ECCS[1:0]: 00=成功/无需ECC, 01=1位纠错成功, 10=2位纠错成功(需确认), 11=无法纠错

// 输出驱动强度定义
#define W25N04_ODS_100_PERCENT   0x00    // 00: 100%(默认)
#define W25N04_ODS_75_PERCENT    0x04    // 01: 75%
#define W25N04_ODS_50_PERCENT    0x08    // 10: 50%
#define W25N04_ODS_25_PERCENT    0x0C    // 11: 25%

// 制造商和设备ID
#define W25N04_MANUFACTURER_ID       0xEF    // Winbond制造商ID
#define W25N04_DEVICE_ID             0xAA23  // W25N04KV设备ID

// 常用值定义
#define W25N04_BLOCK_COUNT           4096    // 总块数
#define W25N04_PAGES_PER_BLOCK       64      // 每块页数
#define W25N04_MAX_COLUMN_ADDR       0x0FFF  // 最大列地址 (12位)
#define W25N04_TIMEOUT_PAGE_READ     25      // 页读取超时(us)
#define W25N04_TIMEOUT_PAGE_PROG     700     // 页编程超时(us)
#define W25N04_TIMEOUT_BLOCK_ERASE   10000   // 块擦除超时(us)


/**
 * @brief 初始化W25N04KV闪存
 * 
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_init(void);

/**
 * @brief 读取W25N04KV的JEDEC ID
 * 
 * @param manufacturer_id 存储制造商ID的指针
 * @param device_id 存储设备ID的指针
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_read_id(uint8 *manufacturer_id, uint16 *device_id);

/**
 * @brief 读取W25N04KV的状态寄存器
 * 
 * @param reg_addr 寄存器地址(0xA0/0xB0/0xC0)
 * @return uint8 状态寄存器值
 */
uint8 w25n04_read_status(uint8 reg_addr);

/**
 * @brief 等待W25N04KV不忙
 * 
 * @param timeout_ms 超时时间(毫秒)，0表示一直等待直到不忙
 * @return uint8 0:超时，1:成功(设备不忙)
 */
uint8 w25n04_wait_busy(uint32 timeout_ms);

/**
 * @brief 系统复位W25N04KV芯片
 * 
 * @param reset_mode 复位模式：0-使用单一复位指令(0xFF)，1-使用复位使能+复位设备指令(0x66/0x99)
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_reset(uint8 reset_mode);

/**
 * @brief 解除W25N04KV的写保护
 * 
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_disable_write_protection(void);

/**
 * @brief 擦除指定的128KB块
 * 
 * @param block_addr 块地址（0-4095）
 * @return uint8 0:失败，1:成功，2:写使能失败，3:擦除失败(E_FAIL位置1)
 */
uint8 w25n04_block_erase(uint16 block_addr);

/**
 * @brief 读取指定页的数据
 * 
 * @param page_addr 页地址
 * @param column_addr 列地址
 * @param data 读取数据的存储位置
 * @param len 要读取的数据长度
 * @return uint8 0:失败，1:成功无ECC错误，2:成功但有ECC错误可纠正，3:ECC错误无法纠正
 */
uint8 w25n04_read_page(uint32 page_addr, uint16 column_addr, uint8 *data, uint16 len);

/**
 * @brief 写入数据到指定页
 * 
 * @param page_addr 页地址
 * @param column_addr 列地址
 * @param data 要写入的数据缓冲区
 * @param len 数据长度
 * @return uint8 0:失败，1:成功，2:程序执行失败
 */
uint8 w25n04_write_page(uint32 page_addr, uint16 column_addr, uint8 *data, uint16 len);

/**
 * @brief 随机加载数据到内部页缓冲区(不重置缓冲区)
 * 
 * @param column_addr 列地址(0-2111)
 * @param data 要加载的数据
 * @param len 数据长度
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_random_program_data_load(uint16 column_addr, uint8 *data, uint16 len);

/**
 * @brief 写使能
 * 
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_write_enable(void);

/**
 * @brief 关闭W25N04KV的ECC功能
 * 
 * @return uint8 0:失败，1:成功
 */
uint8 w25n04_disable_ecc(void);

#endif /* _W25N04_H_ */ 
