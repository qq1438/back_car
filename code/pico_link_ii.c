/**************************************
 * �ļ����� pico_link_ii.c
 * ���� ����
 * QQ 1626632460
 * QQ����Ⱥ 613773009
 * ���ù̼��汾 v2.31
 * Copyright (c) 2023 ����
 *************************************/

#include "pico_link_ii.h"

//------------�û��滻������Ҫ������ͷ�ļ�-----------------------
#include "zf_common_headfile.h" // ���TC264��汾3.1.9

//--------------------UART �궨��------------------------------
// UART���Ͷ���ֽڣ���Ҫ�Լ��Һ�������, array:���������׵�ַ, length:���ͳ���
#define UART_SEND_BYTES(array, length) uart_write_buffer((UART_2), (array), (length))
//----------------------SPI �궨��-----------------------------
// SPIһ�δ�������ֽ���, ���16384, ����˫��ȫ˫��SPI���4096
#define SPI_MAX_TRANS_SIZE (16384)
// SPI CS��������, ��Ҫ�Լ��Һ�������
#define SPI_CS_LOW gpio_set_level(P22_2, 0)
// SPI CS��������, ��Ҫ�Լ��Һ�������
#define SPI_CS_HIGH gpio_set_level(P22_2, 1)
// ȫ˫��SPI���Ͷ���ֽ�, ��Ҫ�Լ��Һ�������, array:���������׵�ַ, length:���䳤��
#define SPI_SEND_BYTES(array, length) spi_write_8bit_array((SPI_3), (array), (length))
// ȫ˫��SPI��ȡ����ֽ�, ��Ҫ�Լ��Һ�������, buffer:���������׵�ַ, length:���䳤��
#define SPI_READ_BYTES(buffer, length) spi_read_8bit_array((SPI_3), (buffer), (length))

//*********************************************************************************************************

/**
 * @description:
 * @param {pico_multi_src_t} *multi_src_ptr
 * @return {*}
 */
void pico_multi_src_clear(pico_multi_src_t *multi_src_ptr)
{
    multi_src_ptr->src_num = 0;
}

/**
 * @description:
 * @param {pico_multi_src_t} *multi_src_ptr
 * @param {pico_uint8} *src
 * @param {pico_uint32} length
 * @return {*}
 */
pico_uint8 pico_multi_src_add(pico_multi_src_t *multi_src_ptr, pico_uint8 *src, pico_uint32 length)
{
    if (multi_src_ptr->src_num < PICO_SRC_MAX_NUM)
    {
        multi_src_ptr->src[multi_src_ptr->src_num] = src;
        multi_src_ptr->src_len[multi_src_ptr->src_num] = length;
        multi_src_ptr->src_num++;
        return 1;
    }
    else
    {
        return 0;
    }
}

//*********************************************************************************************************

/**
 * @description: Pico Link II UART���͵����ֽ�
 * @param {pico_uint8} data
 * @return {*}
 */
void pico_send_8bit_uart(pico_uint8 data)
{
    UART_SEND_BYTES(&data, 1);
}

/**
 * @description: Pico Link II UART���Ͷ���ֽ�
 * @param {pico_uint8} *array �����׵�ַ
 * @param {pico_uint32} length ���ݳ���
 * @return {*}
 */
void pico_send_8bit_array_uart(pico_uint8 *array, pico_uint32 length)
{
    if (length > 0)
    {
        UART_SEND_BYTES(array, length);
    }
}

/**
 * @description: Pico Link II UART��Դ����
 * @param {pico_multi_src_t} multi_src ��Դ�ṹ��
 * @return {*}
 */
void pico_send_8bit_array_multi_uart(pico_multi_src_t multi_src)
{
    for (pico_uint8 i = 0; i < multi_src.src_num; i++)
    {
        if (multi_src.src_len[i] > 0)
        {
            UART_SEND_BYTES(multi_src.src[i], multi_src.src_len[i]);
        }
    }
}

//*********************************************************************************************************

/**
 * @description: Pico Link II ȫ˫��SPI���͵����ֽ�
 * @param {pico_uint8} data
 * @return {*}
 */
void pico_send_8bit_fd_spi(pico_uint8 data)
{
    SPI_CS_LOW;
    SPI_SEND_BYTES(&data, 1);
    SPI_CS_HIGH;
}

/**
 * @description: Pico Link II ȫ˫��SPI���Ͷ���ֽ�
 * @param {pico_uint8} *array �����׵�ַ
 * @param {pico_uint32} length ���ݳ���
 * @return {*}
 */
void pico_send_8bit_array_fd_spi(pico_uint8 *array, pico_uint32 length)
{
    while (length > SPI_MAX_TRANS_SIZE)
    {
        SPI_CS_LOW;
        SPI_SEND_BYTES(array, SPI_MAX_TRANS_SIZE);
        SPI_CS_HIGH;
        array += SPI_MAX_TRANS_SIZE;
        length -= SPI_MAX_TRANS_SIZE;
    }
    if (length > 0)
    {
        SPI_CS_LOW;
        SPI_SEND_BYTES(array, length);
        SPI_CS_HIGH;
    }
}

/**
 * @description: Pico Link II ȫ˫��SPI��Դ����
 * @param {pico_multi_src_t} multi_src ��Դ�ṹ��
 * @return {*}
 */
void pico_send_8bit_array_multi_fd_spi(pico_multi_src_t multi_src)
{
    pico_uint32 send_len = 0; // �ѷ��ͳ���
    pico_uint32 part_len = 0;
    SPI_CS_LOW;
    for (pico_uint8 i = 0; i < multi_src.src_num; i++)
    {
        if (send_len + multi_src.src_len[i] > SPI_MAX_TRANS_SIZE)
        {
            part_len = SPI_MAX_TRANS_SIZE - send_len;
            SPI_SEND_BYTES(multi_src.src[i], part_len);
            SPI_CS_HIGH; // ������һ��SPI����
            multi_src.src[i] += part_len;
            multi_src.src_len[i] -= part_len;
            send_len = 0; // ����
            i--;          // �ع�
            SPI_CS_LOW;   // ������һ��SPI����
        }
        else if (multi_src.src_len[i] > 0)
        {
            SPI_SEND_BYTES(multi_src.src[i], multi_src.src_len[i]);
            send_len += multi_src.src_len[i];
        }
    }
    SPI_CS_HIGH;
}

//*********************************************************************************************************

#define SPI_WRITE_BUF (0x02)
#define SPI_READ_BUF (0x02)
#define SPI_WRITE_DATA (0x03)
#define SPI_READ_DATA (0x04)
#define SPI_WRITE_END (0x07)
#define SPI_READ_END (0x08)

#define RD_STATUS_REG (0x04)

typedef struct
{
    pico_uint8 cmd;
    pico_uint8 addr;
    pico_uint8 dummy;
} __attribute__((packed)) hd_spi_header_t;

typedef struct
{
    pico_uint8 magic;
    pico_uint8 sequence;
    pico_uint16 length;
} __attribute__((packed)) hd_rd_status_t; // �洢��˫��SPI��״̬�Ĵ�������

static inline void pico_hd_spi_send_done(void)
{
    hd_spi_header_t header = {
        .cmd = SPI_WRITE_END,
        .addr = 0x00,
        .dummy = 0x00,
    };
    SPI_CS_LOW;
    SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
    SPI_CS_HIGH;
}

/**
 * @description: Pico Link II ��˫��SPI���͵����ֽ�
 * @param {pico_uint8} data
 * @return {*}
 */
void pico_send_8bit_hd_spi(pico_uint8 data)
{
    hd_spi_header_t header = {
        .cmd = SPI_WRITE_DATA,
        .addr = 0x00,
        .dummy = 0x00,
    };
    SPI_CS_LOW;
    SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
    SPI_SEND_BYTES(&data, 1);
    SPI_CS_HIGH;
    pico_hd_spi_send_done();
}

/**
 * @description: Pico Link II ��˫��SPI���Ͷ���ֽ�
 * @param {pico_uint8} *array �����׵�ַ
 * @param {pico_uint32} length ���ݳ���
 * @return {*}
 */
void pico_send_8bit_array_hd_spi(pico_uint8 *array, pico_uint32 length)
{
    hd_spi_header_t header = {
        .cmd = SPI_WRITE_DATA,
        .addr = 0x00,
        .dummy = 0x00,
    };
    while (length > SPI_MAX_TRANS_SIZE)
    {
        SPI_CS_LOW;
        SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
        SPI_SEND_BYTES(array, SPI_MAX_TRANS_SIZE);
        SPI_CS_HIGH;
        pico_hd_spi_send_done();
        array += SPI_MAX_TRANS_SIZE;
        length -= SPI_MAX_TRANS_SIZE;
    }
    if (length > 0)
    {
        SPI_CS_LOW;
        SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
        SPI_SEND_BYTES(array, length);
        SPI_CS_HIGH;
        pico_hd_spi_send_done();
    }
}

/**
 * @description: Pico Link II ��˫��SPI��Դ����
 * @param {pico_multi_src_t} multi_src ��Դ�ṹ��
 * @return {*}
 */
void pico_send_8bit_array_multi_hd_spi(pico_multi_src_t multi_src)
{
    hd_spi_header_t header = {
        .cmd = SPI_WRITE_DATA,
        .addr = 0x00,
        .dummy = 0x00,
    };
    pico_uint32 send_len = 0; // �ѷ��ͳ���
    pico_uint32 part_len = 0;
    SPI_CS_LOW;
    SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
    for (pico_uint8 i = 0; i < multi_src.src_num; i++)
    {
        if (send_len + multi_src.src_len[i] > SPI_MAX_TRANS_SIZE)
        {
            part_len = SPI_MAX_TRANS_SIZE - send_len;
            SPI_SEND_BYTES(multi_src.src[i], part_len);
            SPI_CS_HIGH; // ������һ��SPI����
            pico_hd_spi_send_done();
            multi_src.src[i] += part_len;
            multi_src.src_len[i] -= part_len;
            send_len = 0; // ����
            i--;          // �ع�
            SPI_CS_LOW;   // ������һ��SPI����
            SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
        }
        else if (multi_src.src_len[i] > 0)
        {
            SPI_SEND_BYTES(multi_src.src[i], multi_src.src_len[i]);
            send_len += multi_src.src_len[i];
        }
    }
    SPI_CS_HIGH;
    pico_hd_spi_send_done();
}

/**
 * @description: Pico Link II ��˫��SPI��ȡ״̬�Ĵ���
 * @param {pico_uint32} *read_len �ɶ�ȡ���ݳ���ָ��
 * @return {*}
 */
static inline pico_uint8 pico_hd_spi_read_status(pico_uint32 *read_len)
{
    hd_spi_header_t header = {
        .cmd = SPI_READ_BUF,
        .addr = RD_STATUS_REG,
        .dummy = 0x00,
    };
    hd_rd_status_t status;
    SPI_CS_LOW;
    SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
    SPI_READ_BYTES((pico_uint8 *)(&status), sizeof(hd_rd_status_t));
    SPI_CS_HIGH;
    *read_len = (pico_uint32)(status.length);
    return status.magic;
}

/**
 * @description: Pico Link II ��˫��SPI��ȡ���
 * @return {*}
 */
static inline void pico_hd_spi_read_done(void)
{
    hd_spi_header_t header = {
        .cmd = SPI_READ_END,
        .addr = 0x00,
        .dummy = 0x00,
    };
    SPI_CS_LOW;
    SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
    SPI_CS_HIGH;
}

/**
 * @description: Pico Link II ��˫��SPI��ȡ����ֽ�
 * @param {pico_uint8} *buffer ��Ҫ���յ����ݵ�ַ
 * @param {pico_uint32} length Ҫ���յ�������󳤶�
 * @return {*}
 */
pico_uint32 pico_read_8bit_array_hd_spi(pico_uint8 *buffer, pico_uint32 length)
{
    pico_uint32 read_len = 0;
    if (pico_hd_spi_read_status(&read_len))
    {
        read_len = read_len > length ? length : read_len;
        hd_spi_header_t header = {
            .cmd = SPI_READ_DATA,
            .addr = 0x00,
            .dummy = 0x00,
        };
        SPI_CS_LOW;
        SPI_SEND_BYTES((pico_uint8 *)(&header), sizeof(hd_spi_header_t));
        SPI_READ_BYTES(buffer, read_len);
        SPI_CS_HIGH;
        pico_hd_spi_read_done();
    }
    return read_len;
}

//*********************************************************************************************************

/**
 * @description: Pico Link II ����ͼ��������λ��
 * @param {pico_uint8} *image ͼ��ָ��
 * @param {pico_uint32} image_h ͼ��߶�
 * @param {pico_uint32} image_w ͼ����
 * @return {*}
 */
void ipc_send_image(pico_uint8 *image, pico_uint32 image_h, pico_uint32 image_w)
{
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, (pico_uint8 *)"CSU", 3);
    pico_multi_src_add(&multi_src, image, image_w * image_h);
    pico_multi_src_add(&multi_src, (pico_uint8 *)"USC", 3);
    pico_send_8bit_array_multi(multi_src);
}
