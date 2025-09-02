#ifndef _IMAGE_STORAGE_H_
#define _IMAGE_STORAGE_H_

#include "zf_common_headfile.h"
#include "w25n04.h"
#include "image_deal.h"

// ѹ�����ͼ��ߴ�
#define IPCH 30
#define IPCW 80
#define image_storage_page 4
#define STORAGE_BUFFER_SIZE  image_storage_page * W25N04_DATA_SIZE

#define image_type 0x08
#define binary_image_type 0x09
#define one_point_type 0x0A
#define two_point_type 0x0B
#define parameter_type 0x0C
#define mid_speed_type 0x0D
#define left_speed_type 0x0E
#define right_speed_type 0x0F
#define acc_target_type 0x10    
#define acc_pid_type 0x11

typedef enum
{
    go = 0,
    back = 1,
    go_go = 2,
    back_back = 3,
}video_process_t;

// �洢ϵͳ״̬��
typedef enum {
    STORAGE_IDLE = 0,        // ����״̬
    STORAGE_WRITING = 1,     // ����д��
    STORAGE_READING = 2,     // ���ڶ�ȡ
    STORAGE_ERROR = 3        // ����״̬
} storage_state_t;

// �����붨��
typedef enum {
    STORAGE_OK = 0,          // �����ɹ�
    STORAGE_INIT_FAILED,     // ��ʼ��ʧ��
    STORAGE_WRITE_FAILED,    // д��ʧ��
    STORAGE_READ_FAILED,     // ��ȡʧ��
    STORAGE_END,           // ��ȡ����
    STORAGE_BUSY,           // �豸æ
    STORAGE_INVALID_PARAM   // ��Ч����
} storage_error_t;

// �洢ϵͳ���ýṹ��
typedef struct {
    uint32 frame_count;    // ֡��
    uint32 current_num;   // ��ǰ��д��ַ
    storage_state_t state;   // ��ǰ״̬
    storage_error_t error;   // ������
} storage_config_t;


/**
 * @brief ��ʼ��ͼ��洢ϵͳ
 * @param config �洢ϵͳ����
 * @return storage_error_t ������
 */
storage_error_t image_storage_init(void);

/**
 * @brief ͼ��ѹ������������ƽ������
 * @param src Դͼ������ָ��
 * @param dst Ŀ��ѹ��ͼ������ָ��
 */
void image_compress(uint8 src[MT9V03X_H][MT9V03X_W]);

/**
 * @brief ��ֵ��ͼ��ѹ������
 * @param src Դͼ������ָ��
 * @param dst Ŀ��ѹ��ͼ������ָ��
 */
void binary_image_compress(uint8 src[MT9V03X_H][MT9V03X_W]);

/**
* @brief ��ѹ������
* @param type ������
*/
void point_compress(uint8 type);

/**
 * @brief ����ѹ������
 * @param parameter ��������ָ��
 */
void parameter_compress(float parameter[2]);

/**
 * @brief PID����ѹ������
 * @param pid PID�����ṹ��
 */
void pid_compress(uint8 type, float parameter);

/**
 * @brief ����ѹ������
 * @param parameter ��������ָ��
 */
void parameter_compress_float(float parameter, uint8 type);

/**
 * @brief �洢һ֡ѹ��ͼ������
 * @param config �洢ϵͳ����
 * @param image_data ԭʼͼ������ָ��
 * @return storage_error_t ������
 */
storage_error_t store_compressed_image(void);

/**
 * @brief PID������ԭ
 * @param pid PID�����ṹ��
 * @param address �洢��ַ
 * @return uint32_t ������һ����ַ
 */
uint32_t pid_decompress(PID *pid, uint32_t address);

/**
 * @brief ��ȡһ֡ѹ��ͼ�����ݲ���ԭ
 * @param config �洢ϵͳ����
 * @param image_data ͼ���������ָ��
 * @return storage_error_t ������
 */
storage_error_t read_compressed_image(video_process_t video_process);

/**
 * @brief ͼ�����ݷ���
 */
void image_data_analysis(void);

/**
 * @brief ��ȡѹ������
 */
storage_error_t read_compressed_data(void);

/**
 * @brief ���ݷ���
 */
void data_analysis(void);

/**
 * @brief ��ȡ�洢ϵͳ״̬
 * @param config �洢ϵͳ����
 * @return storage_state_t ��ǰ״̬
 */
storage_state_t get_storage_state(void);

/**
 * @brief ��ȡ���һ�δ���
 * @param config �洢ϵͳ����
 * @return storage_error_t ������
 */
storage_error_t get_last_error(void);

/**
 * @brief ��ȡ��ǰ֡��
 * @return uint32_t ��ǰ֡��
 */
uint32 get_frame_count(void);

/**
 * @brief ��������оƬ
 * @return storage_error_t ������
 */
storage_error_t erase_storage_block(void);

/**
 * @brief ��ȡflash�е�֡��
 */
void image_read_frame_count(void);

#endif /* _IMAGE_STORAGE_H_ */
