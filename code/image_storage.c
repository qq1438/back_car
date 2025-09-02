#include "image_storage.h"
#include "zf_common_headfile.h"
#include <string.h>

// 图像数据存储相关定义
#define IMAGE_SIZE             (MT9V03X_W * MT9V03X_H)  // 一帧图像大小
#define COMPRESSED_IMAGE_SIZE  (IPCH * IPCW)  // 压缩后图像大小
#define COMPRESSED_PAGES_PER_FRAME ((COMPRESSED_IMAGE_SIZE + W25N04_DATA_SIZE - 1) / W25N04_DATA_SIZE)  // 压缩后每帧所需页数

// 定点数计算相关定义
#define FIXED_POINT_BITS 8
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)

// 临时存储缓冲区
static uint8 storage_buffer[STORAGE_BUFFER_SIZE];
storage_config_t storage_config;

// 图像压缩函数
void image_compress(uint8 src[MT9V03X_H][MT9V03X_W])
{
    memset(storage_buffer, 0xff, STORAGE_BUFFER_SIZE);
    storage_config.current_num = 0;
    storage_buffer[storage_config.current_num] = image_type;
    storage_config.current_num++;
    const int32_t scale_h = ((int32_t)(MT9V03X_H << FIXED_POINT_BITS)) / IPCH;
    const int32_t scale_w = ((int32_t)(MT9V03X_W << FIXED_POINT_BITS)) / IPCW;
    for (int i = 0; i < IPCH; i++)
    {
        for (int j = 0; j < IPCW; j++)
        {
            int32_t y = (i * scale_h + (scale_h >> 1)) >> FIXED_POINT_BITS;
            int32_t x = (j * scale_w + (scale_w >> 1)) >> FIXED_POINT_BITS;
            
            if (y >= MT9V03X_H) y = MT9V03X_H - 1;
            if (x >= MT9V03X_W) x = MT9V03X_W - 1;
            
            storage_buffer[storage_config.current_num] = src[y][x];
            storage_config.current_num++;
        }
    }
}

void binary_image_compress(uint8 src[MT9V03X_H][MT9V03X_W])
{
    memset(storage_buffer, 0xff, STORAGE_BUFFER_SIZE);
    storage_config.current_num = 0;
    storage_buffer[storage_config.current_num] = binary_image_type;
    storage_config.current_num++;
    
    uint8 bit_pos = 0;
    uint8 byte_val = 0;
    
    for(uint16 i = 0; i < MT9V03X_H; i++)
    {
        for(uint16 j = 0; j < MT9V03X_W; j++)
        {
            if(src[i][j])
            {
                byte_val |= (1 << (7 - bit_pos));
            }
            
            bit_pos++;
            
            if(bit_pos == 8)
            {
                storage_buffer[storage_config.current_num] = byte_val;
                storage_config.current_num++;
                bit_pos = 0;
                byte_val = 0;
            }
        }
    }
}

void point_compress(uint8 type)
{
    point points;
    if(type == one_point_type)
    {
        points = one_point;
    }
    else if(type == two_point_type)
    {
        points = two_point;
    }
    storage_buffer[storage_config.current_num] = type;
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = points.now_point[0];
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = points.now_point[1];
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = points.last_point[0];
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = points.last_point[1];
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = points.llast_point[0];
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = points.llast_point[1];
    storage_config.current_num++;
}

void parameter_compress(float parameter[2])
{
    storage_buffer[storage_config.current_num] = parameter_type;
    storage_config.current_num++;
    for(uint8 i = 0; i < 3; i++)
    {
        uint32_t *p = (uint32_t *)&parameter[i];
        storage_buffer[storage_config.current_num] = (*p >> 24) & 0xFF;
        storage_config.current_num++;
        storage_buffer[storage_config.current_num] = (*p >> 16) & 0xFF; 
        storage_config.current_num++;
        storage_buffer[storage_config.current_num] = (*p >> 8) & 0xFF;
        storage_config.current_num++;
        storage_buffer[storage_config.current_num] = *p & 0xFF;
        storage_config.current_num++;
    }
}

void pid_compress(uint8 type, float parameter)
{
    storage_buffer[storage_config.current_num] = type;
    storage_config.current_num++;
    uint32_t *p = (uint32_t *)&parameter;
    storage_buffer[storage_config.current_num] = (*p >> 24) & 0xFF;
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = (*p >> 16) & 0xFF; 
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = (*p >> 8) & 0xFF;
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = *p & 0xFF;
    storage_config.current_num++;
}

void parameter_compress_float(float parameter, uint8 type)
{
    storage_buffer[storage_config.current_num] = type;
    storage_config.current_num++;
    uint32_t *p = (uint32_t *)&parameter;
    storage_buffer[storage_config.current_num] = (*p >> 24) & 0xFF;
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = (*p >> 16) & 0xFF;
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = (*p >> 8) & 0xFF;
    storage_config.current_num++;
    storage_buffer[storage_config.current_num] = *p & 0xFF;
    storage_config.current_num++;
}


// 图像还原函数
void image_decompress(uint8 *src, uint8 dst[MT9V03X_H][MT9V03X_W])
{
    const int32_t scale_h = ((int32_t)(IPCH << FIXED_POINT_BITS)) / MT9V03X_H;
    const int32_t scale_w = ((int32_t)(IPCW << FIXED_POINT_BITS)) / MT9V03X_W;

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < MT9V03X_H; i++)
    {
        for (int j = 0; j < MT9V03X_W; j++)
        {
            int32_t y = (i * scale_h);
            int32_t x = (j * scale_w);

            int y_int = y >> FIXED_POINT_BITS;
            int x_int = x >> FIXED_POINT_BITS;

            if (y_int >= IPCH - 1) y_int = IPCH - 2;
            if (x_int >= IPCW - 1) x_int = IPCW - 2;

            int32_t wy = y & (FIXED_POINT_SCALE - 1);
            int32_t wx = x & (FIXED_POINT_SCALE - 1);

            uint8 p00 = src[y_int * IPCW + x_int];
            uint8 p01 = src[y_int * IPCW + x_int + 1];
            uint8 p10 = src[(y_int + 1) * IPCW + x_int];
            uint8 p11 = src[(y_int + 1) * IPCW + x_int + 1];

            int32_t col0 = p00 * (FIXED_POINT_SCALE - wx) + p01 * wx;
            int32_t col1 = p10 * (FIXED_POINT_SCALE - wx) + p11 * wx;
            
            int32_t value = col0 * (FIXED_POINT_SCALE - wy) + col1 * wy;
            
            dst[i][j] = ((value + (FIXED_POINT_SCALE * FIXED_POINT_SCALE / 2)) 
                         >> (FIXED_POINT_BITS * 2));
        }
    }
}

void binary_image_decompress(uint8 *src, uint8 dst[MT9V03X_H][MT9V03X_W])
{   
    uint32_least src_pos = 0;  // 使用uint16以避免大图像时可能的溢出
    uint8 bit_pos = 0;
    uint32 total_bits = MT9V03X_H * MT9V03X_W;  // 总比特数
    uint32 current_bit = 0;  // 当前处理的比特位
    
    memset(dst, 0, MT9V03X_H * MT9V03X_W);
    
    for(uint16 i = 0; i < MT9V03X_H; i++)
    {
        for(uint16 j = 0; j < MT9V03X_W; j++) 
        {
            // 边界检查，防止读取超出src缓冲区范围
            if(current_bit < total_bits && src_pos < COMPRESSED_IMAGE_SIZE) 
            {
                // 检查对应比特位是否为1
                dst[i][j] = (src[src_pos] & (1 << (7 - bit_pos))) ? 255 : 0;
                
                bit_pos++;
                current_bit++;
                
                if(bit_pos == 8)
                {
                    src_pos++;
                    bit_pos = 0;
                }
            }
            else
            {
                // 超出范围，将剩余像素设置为0
                dst[i][j] = 0;
            }
        }
    }
}

uint32_t point_decompress(point *point,uint32_t address)
{
    point->now_point[0] = storage_buffer[address];
    address++;
    point->now_point[1] = storage_buffer[address];
    address++;
    point->last_point[0] = storage_buffer[address];
    address++;
    point->last_point[1] = storage_buffer[address];
    address++;
    point->llast_point[0] = storage_buffer[address];
    address++;
    point->llast_point[1] = storage_buffer[address];
    address++;
    return address;
}

uint32_t parameter_decompress(float parameter[2],uint32_t address)
{
    for(uint8 i = 0; i < 3; i++)
    {
        uint32_t *p = (uint32_t *)&parameter[i];
        *p = (storage_buffer[address] << 24) | (storage_buffer[address + 1] << 16) | (storage_buffer[address + 2] << 8) | storage_buffer[address + 3];
        address += 4;
    }
    return address;
}

uint32_t pid_decompress(PID *pid,uint32_t address)
{
    uint32_t *p = (uint32_t *)&pid->target;
    *p = (storage_buffer[address] << 24) | (storage_buffer[address + 1] << 16) | (storage_buffer[address + 2] << 8) | storage_buffer[address + 3];
    address += 4;
    return address;
}

uint32 image_decompress_float(float *error,uint32 address)
{
    uint32_t *p = (uint32_t *)error;
    *p = (storage_buffer[address] << 24) | (storage_buffer[address + 1] << 16) | (storage_buffer[address + 2] << 8) | storage_buffer[address + 3];
    address += 4;
    return address;
}

// 初始化存储系统
storage_error_t image_storage_init(void)
{
    // 复位设备
    if (!w25n04_reset(1))
    {
        storage_config.error = STORAGE_INIT_FAILED;
        storage_config.state = STORAGE_ERROR;
        return STORAGE_INIT_FAILED;
    }
    
    // 解除写保护
    if (!w25n04_disable_write_protection())
    {
        storage_config.error = STORAGE_INIT_FAILED;
        storage_config.state = STORAGE_ERROR;
        return STORAGE_INIT_FAILED;
    }
    
    storage_config.state = STORAGE_IDLE;
    storage_config.error = STORAGE_OK;
    storage_config.current_num = 0;
    storage_config.frame_count = 0;
    
    return STORAGE_OK;
}

// 存储压缩图像
storage_error_t store_compressed_image(void)
{
    if (storage_config.state == STORAGE_WRITING || storage_config.state == STORAGE_READING) return STORAGE_BUSY;
    
    // 检查是否有数据需要写入
    if (storage_config.current_num == 0) {
        storage_config.error = STORAGE_INVALID_PARAM;
        return STORAGE_INVALID_PARAM;  // 不允许写入空数据
    }
    
    storage_config.state = STORAGE_WRITING;
    
    // 写入数据
    for (uint16 i = 0; i < storage_config.current_num; i += W25N04_DATA_SIZE)
    {
        if (!w25n04_write_enable() ||
            !w25n04_program_data_load(0, storage_buffer + i, W25N04_DATA_SIZE) ||
            w25n04_program_execute((storage_config.frame_count * STORAGE_BUFFER_SIZE + i) / W25N04_DATA_SIZE) != 1 ||
            !w25n04_wait_busy(20))
        {
            storage_config.error = STORAGE_WRITE_FAILED;
            storage_config.state = STORAGE_ERROR;
            return STORAGE_WRITE_FAILED;
        }
    }
    
    storage_config.frame_count++;
    storage_config.state = STORAGE_IDLE;
    return STORAGE_OK;
}

// 读取压缩图像
uint32 video_frame = 0;
storage_error_t read_compressed_image(video_process_t video_process)
{
    if (storage_config.state == STORAGE_WRITING || storage_config.state == STORAGE_READING) return STORAGE_BUSY;
    if (storage_config.frame_count == 0) return STORAGE_OK;  // 没有存储的帧
    
    storage_config.state = STORAGE_READING;
    
        // 读取压缩数据
    for (uint16 i = 0; i < STORAGE_BUFFER_SIZE; i += W25N04_DATA_SIZE)
    {
        if (!w25n04_read_page((video_frame * STORAGE_BUFFER_SIZE + i) / W25N04_DATA_SIZE, 0, storage_buffer + i, W25N04_DATA_SIZE))
        {
            storage_config.error = STORAGE_READ_FAILED;
            storage_config.state = STORAGE_ERROR;
            return STORAGE_READ_FAILED;
        }
    }
    switch(video_process) {
        case go:
            video_frame = video_frame + 1;
            break;
        case go_go:
            video_frame = video_frame + 20;
            break;
        case back_back:
            video_frame = video_frame > 20 ? video_frame - 20 : 0;
            break;
        default:
            video_frame = video_frame > 0 ? video_frame - 1 : 0;
    }
    video_frame = video_frame > storage_config.frame_count ? storage_config.frame_count : video_frame;
    ips200_show_uint(188,260,video_frame,6);
    ips200_show_uint(188,280,storage_config.frame_count,6);
    storage_config.state = STORAGE_IDLE;
    return STORAGE_OK;
}

storage_error_t read_compressed_data()
{
    if (storage_config.state == STORAGE_WRITING || storage_config.state == STORAGE_READING) return STORAGE_BUSY;
    if (storage_config.frame_count == 0) return STORAGE_OK;  // 没有存储的帧
    
    storage_config.state = STORAGE_READING;
    
        // 读取压缩数据
    for (uint16 i = 0; i < STORAGE_BUFFER_SIZE; i += W25N04_DATA_SIZE)
    {
        if (!w25n04_read_page((video_frame * STORAGE_BUFFER_SIZE + i) / W25N04_DATA_SIZE, 0, storage_buffer + i, W25N04_DATA_SIZE))
        {
            storage_config.error = STORAGE_READ_FAILED;
            storage_config.state = STORAGE_ERROR;
            return STORAGE_READ_FAILED;
        }
    }
    video_frame++;
    ips200_show_uint(188,260,video_frame,6);
    ips200_show_uint(188,280,storage_config.frame_count,6);
    if(video_frame==storage_config.frame_count)return STORAGE_END;
    storage_config.state = STORAGE_IDLE;
    return STORAGE_OK;
}

void image_data_analysis(void)
{
    uint32 address = 0;
    while(address < STORAGE_BUFFER_SIZE)
    {
        switch(storage_buffer[address])
        {
            case image_type:
                address += 1;
                image_decompress(storage_buffer + address, Image);
                address += COMPRESSED_IMAGE_SIZE;
                break;

            case binary_image_type:
                address += 1;
                binary_image_decompress(storage_buffer + address, Image);
                address += (MT9V03X_H * MT9V03X_W / 8);
                break;

            case one_point_type:
                address += 1;
                address = point_decompress(&one_point,address);
                break;
            case two_point_type:
                address += 1;
                address = point_decompress(&two_point,address);
                break;
            case parameter_type:
                address += 1;
                address = parameter_decompress(error,address);
                break;
            case acc_target_type:
                address += 1;
                address = image_decompress_float(&acc_pid.target,address);
                break;
            case acc_pid_type:
                address += 1;
                address = image_decompress_float(&acc_x,address);
                break;
            default:
                address = STORAGE_BUFFER_SIZE;
        }
    }
}


void data_analysis(void)
{
    uint32 address = 0;
    while(address < STORAGE_BUFFER_SIZE)
    {
        switch(storage_buffer[address])
        {
            case image_type:
                address += 1;
                address += COMPRESSED_IMAGE_SIZE;
                break;

            case binary_image_type:
                address += 1;
                address += (MT9V03X_H * MT9V03X_W / 8);
                break;

            case one_point_type:
                address += 1;
                address = point_decompress(&one_point,address);
                break;
            case two_point_type:
                address += 1;
                address = point_decompress(&two_point,address);
                break;
            case parameter_type:
                address += 1;
                address = parameter_decompress(error,address);
                break;
            case acc_target_type:
                address += 1;
                address = image_decompress_float(&add_speed,address);
                break;
            case acc_pid_type:
                address += 1;
                address = image_decompress_float(&pid_speed,address);
                break;
            default:
                address = STORAGE_BUFFER_SIZE;
        }
    }
}


// 获取存储状态
storage_state_t get_storage_state(void)
{
    return storage_config.state;
}

// 获取最后一次错误
storage_error_t get_last_error(void)
{
    return storage_config.error;
}

// 获取当前帧数
uint32 get_frame_count(void)
{
    return storage_config.frame_count;
}

// 擦除存储块
storage_error_t erase_storage_block(void)
{
    // 仅在空闲状态下允许擦除
    if (storage_config.state != STORAGE_IDLE) return STORAGE_BUSY;

    // 先读取当前已存帧数
    image_read_frame_count();

    uint32 total_frames = storage_config.frame_count;

    // 如果没有数据，直接返回
    if (total_frames == 0) {
        return STORAGE_OK;
    }

    // 计算每个块可容纳的帧数
    const uint32 frames_per_block = W25N04_PAGES_PER_BLOCK / image_storage_page; // 64/4 = 16

    // 计算需要擦除的块数量（向上取整）
    uint32 blocks_to_erase = (total_frames + frames_per_block - 1) / frames_per_block;

    for (uint16 block = 0; block < blocks_to_erase; block++)
    {
        if (w25n04_block_erase(block) != 1)
        {
            storage_config.error = STORAGE_WRITE_FAILED;
            storage_config.state = STORAGE_ERROR;
            return STORAGE_WRITE_FAILED;
        }

        // 等待擦除完成
        if (w25n04_wait_busy(100) == 0)
        {
            storage_config.error = STORAGE_WRITE_FAILED;
            storage_config.state = STORAGE_ERROR;
            return STORAGE_WRITE_FAILED;
        }
    }

    // 更新状态信息
    storage_config.frame_count = 0;
    storage_config.error = STORAGE_OK;
    return STORAGE_OK;
}

void image_read_frame_count(void)
{
    uint32 left = 0;
    uint32 right = W25N04_TOTAL_PAGES / image_storage_page;  // 总帧数
    uint32 last_valid_frame = 0;
    uint8 temp_buffer[W25N04_DATA_SIZE];
    bool is_all_ff;

    if (storage_config.state != STORAGE_IDLE) return;
    
    storage_config.state = STORAGE_READING;
    
    // 使用二分查找找到最后一个非全0xFF的帧
    while (left <= right)
    {
        uint32 mid = left + (right - left) / 2;
        uint32 frame_start_page = mid * image_storage_page;  // 当前帧的起始页
        
        // 读取该帧的第一页
        if (!w25n04_read_page(frame_start_page, 0, temp_buffer, W25N04_DATA_SIZE))
        {
            storage_config.error = STORAGE_READ_FAILED;
            storage_config.state = STORAGE_ERROR;
            return;
        }
        
        // 检查该页是否全为0xFF
        is_all_ff = true;
        for (uint16 i = 0; i < W25N04_DATA_SIZE; i++)
        {
            if (temp_buffer[i] != 0xFF)
            {
                is_all_ff = false;
                break;
            }
        }
        
        if (is_all_ff)
        {
            right = mid - 1;
        }
        else
        {
            last_valid_frame = mid;
            left = mid + 1;
        }
    }
    
    // 处理结果
    if (last_valid_frame == 0 && is_all_ff)
    {
        // 检查第一帧是否为有效帧
        if (!w25n04_read_page(0, 0, temp_buffer, W25N04_DATA_SIZE))
        {
            storage_config.error = STORAGE_READ_FAILED;
            storage_config.state = STORAGE_ERROR;
            return;
        }
        
        is_all_ff = true;
        for (uint16 i = 0; i < W25N04_DATA_SIZE; i++)
        {
            if (temp_buffer[i] != 0xFF)
            {
                is_all_ff = false;
                break;
            }
        }
        
        if (is_all_ff)
        {
            storage_config.frame_count = 0;
        }
        else
        {
            storage_config.frame_count = 1;
        }
    }
    else
    {
        storage_config.frame_count = last_valid_frame - 1;
    }
    
    storage_config.state = STORAGE_IDLE;
}
