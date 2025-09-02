/**************************************
 * �ļ����� pico_link_ii.h
 * ���� ����
 * QQ 1626632460
 * QQ����Ⱥ 613773009
 * ���ù̼��汾 v2.31
 * Copyright (c) 2023 ����
 *************************************/

#ifndef __PICO_LINK_II_H__
#define __PICO_LINK_II_H__

// UART
#define PICO_PROTOCOL_UART (0)
// ȫ˫��SPI 1bit
#define PICO_PROTOCOL_FD_SPI (1)
// ��˫��SPI 1bit
#define PICO_PROTOCOL_HD_SPI (0)

#if PICO_PROTOCOL_UART
#define pico_send_8bit(data) pico_send_8bit_uart(data)
#define pico_send_8bit_array(array, length) pico_send_8bit_array_uart(array, length)
#define pico_send_8bit_array_multi(multi_src) pico_send_8bit_array_multi_uart(multi_src)
#elif PICO_PROTOCOL_FD_SPI
#define pico_send_8bit(data) pico_send_8bit_fd_spi(data)
#define pico_send_8bit_array(array, length) pico_send_8bit_array_fd_spi(array, length)
#define pico_send_8bit_array_multi(multi_src) pico_send_8bit_array_multi_fd_spi(multi_src)
#elif PICO_PROTOCOL_HD_SPI
#define pico_send_8bit(data) pico_send_8bit_hd_spi(data)
#define pico_send_8bit_array(array, length) pico_send_8bit_array_hd_spi(array, length)
#define pico_send_8bit_array_multi(multi_src) pico_send_8bit_array_multi_hd_spi(multi_src)
#endif

// ������������
typedef unsigned char pico_uint8;
typedef unsigned short int pico_uint16;
typedef unsigned long int pico_uint32;

// ��Դ��ʾ�ṹ��
#define PICO_SRC_MAX_NUM (10)
typedef struct
{
    pico_uint8 src_num;                    // ����Դ����
    pico_uint8 *src[PICO_SRC_MAX_NUM];     // ����Դ�׵�ַ����
    pico_uint32 src_len[PICO_SRC_MAX_NUM]; // ����Դ��������
} pico_multi_src_t;

void pico_multi_src_clear(pico_multi_src_t *multi_src_ptr);
pico_uint8 pico_multi_src_add(pico_multi_src_t *multi_src_ptr, pico_uint8 *src, pico_uint32 length);

void pico_send_8bit_uart(pico_uint8 data);
void pico_send_8bit_array_uart(pico_uint8 *array, pico_uint32 length);
void pico_send_8bit_array_multi_uart(pico_multi_src_t multi_src);

void pico_send_8bit_fd_spi(pico_uint8 data);
void pico_send_8bit_array_fd_spi(pico_uint8 *array, pico_uint32 length);
void pico_send_8bit_array_multi_fd_spi(pico_multi_src_t multi_src);

void pico_send_8bit_hd_spi(pico_uint8 data);
void pico_send_8bit_array_hd_spi(pico_uint8 *array, pico_uint32 length);
void pico_send_8bit_array_multi_hd_spi(pico_multi_src_t multi_src);
pico_uint32 pico_read_8bit_array_hd_spi(pico_uint8 *buffer, pico_uint32 length);
void ipc_send_image(pico_uint8 *image, pico_uint32 image_h, pico_uint32 image_w);

#endif