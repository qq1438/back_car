#include "image_deal.h"

#define MAX_AREAS 20
#define MAX_QUEUE_SIZE (MT9V03X_H * MT9V03X_W)

// �����������ü�ֵ��ָ����Χ
#define CLIP3(min, max, value) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

// �򵥵ľ���ֵ����
#define ABS(x) ((x) < 0 ? -(x) : (x))

// ��Ӷ�ֵ�����ұ�
static uint8 binary_lut[256];

float error[3] = {0};

// ��ȡ��ǰ֡���������ֵ
uint8 Get_Max_Brightness(void) {
    uint8 max_brightness = 0;
    for (uint16 i = 0; i < MT9V03X_H; i++) {
        for (uint16 j = 0; j < MT9V03X_W; j++) {
            if (Image[i][j] > max_brightness) {
                max_brightness = Image[i][j];
            }
        }
    }
    if(max_brightness < 200)max_brightness = 200;
    return max_brightness;
}

// ��ʼ����ֵ�����ұ�
void Init_Binary_LUT(uint8 threshold) {
    for (uint16 i = 0; i < 256; i++) {
        binary_lut[i] = (i > threshold) ? 255 : 0;
    }
}

// ���нṹ��
typedef struct {
    uint8 x[MAX_QUEUE_SIZE];
    uint8 y[MAX_QUEUE_SIZE];
    uint16 front;
    uint16 rear;
} Queue;

Connected_area Connected_areas[MAX_AREAS];
uint8 Image[MT9V03X_H][MT9V03X_W];

void Image_copy(void) {
    for (uint16 i = 0; i < MT9V03X_H; i++) {
        for (uint16 j = 0; j < MT9V03X_W; j++) {
            Image[i][j] = mt9v03x_image[i][j];
        }
    }
}

void Image_Binary(uint8 threshold) {
    // ���²��ұ�
    Init_Binary_LUT(threshold);
    // ʹ�ò��ұ���ж�ֵ��
    for (uint16 i = 0; i < MT9V03X_H; i++) {
        for (uint16 j = 0; j < MT9V03X_W; j++) {
            Image[i][j] = binary_lut[Image[i][j]];
        }
    }
}

bool Enqueue(Queue *q, uint8 x, uint8 y) {
    if (q->rear < MAX_QUEUE_SIZE) {
        q->x[q->rear] = x;
        q->y[q->rear] = y;
        q->rear++;
        return true;
    }
    return false;
}

bool Dequeue(Queue *q, uint8 *x, uint8 *y) {
    if (q->front < q->rear) {
        *x = q->x[q->front];
        *y = q->y[q->front];
        q->front++;
        return true;
    }
    return false;
}

void Connected_Find(uint8 start_x, uint8 start_y, Connected_area *current_area, uint8 brightness_threshold) {
    int8 dx[8] = { -1,  0, 1, -1, 1, -1, 0, 1 };
    int8 dy[8] = { -1, -1, -1,  0, 0,  1, 1, 1 };

    Queue q = { .front = 0, .rear = 0 };

    current_area->area = 0;
    current_area->location_x = 0;
    current_area->location_y = 0;

    if (!Enqueue(&q, start_x, start_y)) {
        return;
    }
    if(start_x >= MT9V03X_W || start_y >= MT9V03X_H) return;
    Image[start_y][start_x] = 0;

    // ��������ɵ���ͨ��������ֵ
    uint8 connect_threshold = brightness_threshold > 40 ? brightness_threshold - 40 : brightness_threshold / 2;

    while (Dequeue(&q, &start_x, &start_y)) {
        // ����Ѽ�¼�����ﵽ���ֵ���������˳�����ֹԽ��
        if (current_area->area >= MAX_POINT_NUM) {
            return;
        }
        current_area->all_point[current_area->area][0] = start_x;
        current_area->all_point[current_area->area][1] = start_y;
        current_area->area++;
        current_area->location_x += start_x;
        current_area->location_y += start_y;

        for (int i = 0; i < 8; i++) {
            uint8 new_x = start_x + dx[i];
            uint8 new_y = start_y + dy[i];

            if (dx[i] < 0 && start_x < -dx[i]) continue;
            if (dy[i] < 0 && start_y < -dy[i]) continue;
            if (new_x >= MT9V03X_W || new_y >= MT9V03X_H) continue;

            // ʹ�ø����ɵ���ֵ������ͨ������
            if (Image[new_y][new_x] >= connect_threshold) {
                if (Enqueue(&q, new_x, new_y)) {
                    Image[new_y][new_x] = 0;
                }
            }
        }
    }
}

point one_point, two_point;

uint8 mid_position = 94;
uint8 area_num = 0;

void Image_Deal(void) {
    uint8 i, tgt_cnt = 0;
    uint8 start_points[2][2] = {0}; // ���ڴ洢�������
    
    // ��ȡ��ǰ֡���������ֵ
    uint8 max_brightness = Get_Max_Brightness();
    
    // ���㶯̬��ֵ - ����һ���������ݲ�
    uint8 brightness_tolerance = max_brightness > 30 ? 30 : max_brightness / 2;
    uint8 min_search_brightness = max_brightness - brightness_tolerance;

    // �����ͨ��
    for (i = 0; i < MAX_AREAS; i++) {
        Connected_areas[i].area = 0;
    }

    // ʹ����һ֡�ĵ�λ����ΪBFS���
    if (one_point.now_point[0] > 0 && one_point.now_point[1] > 0) {
        start_points[0][0] = one_point.now_point[0];
        start_points[0][1] = one_point.now_point[1];
    }
    
    if (two_point.now_point[0] > 0 && two_point.now_point[1] > 0) {
        start_points[1][0] = two_point.now_point[0];
        start_points[1][1] = two_point.now_point[1];
    }
    
    // ��������㿪ʼBFS����
    for (i = 0; i < 2 && tgt_cnt < MAX_AREAS; i++) {
        if (start_points[i][0] > 0 && start_points[i][0] < MT9V03X_W && 
            start_points[i][1] > 0 && start_points[i][1] < MT9V03X_H) {
            
            // �������Ƿ�Ϊ�����ĵ㣬�������������Χ����
            if (Image[start_points[i][1]][start_points[i][0]] < min_search_brightness) {
                const int MAX_RADIUS = 25; // ���������뾶
                int orig_x = start_points[i][0];
                int orig_y = start_points[i][1];
                int x = orig_x, y = orig_y;
                int dx_dir[4] = {1, 0, -1, 0};
                int dy_dir[4] = {0, 1, 0, -1};
                int dir = 0, step_len = 1;
                bool found = false;
                // ������ɢɨ�裬������������չ��
                while (step_len <= MAX_RADIUS * 2 && !found) {
                    for (int rep = 0; rep < 2 && !found; rep++) {
                        for (int s = 0; s < step_len && !found; s++) {
                            x += dx_dir[dir];
                            y += dy_dir[dir];
                            // �޶�������Χ��������������
                            if (x < orig_x - MAX_RADIUS || x > orig_x + MAX_RADIUS ||
                                y < orig_y - MAX_RADIUS || y > orig_y + MAX_RADIUS) {
                                continue;
                            }
                            if (x >= 0 && x < MT9V03X_W && y >= 0 && y < MT9V03X_H &&
                                Image[y][x] >= min_search_brightness) { // �ſ���������
                                start_points[i][0] = (uint8)x;
                                start_points[i][1] = (uint8)y;
                                found = true;
                            }
                        }
                        dir = (dir + 1) % 4;
                    }
                    step_len++;
                }
                if (!found) continue; // ���û�ҵ��׵�������
            }
            
            // ��ʼ��ͨ������
            Connected_Find(start_points[i][0], start_points[i][1], &Connected_areas[tgt_cnt], max_brightness);
            if (Connected_areas[tgt_cnt].area > 1) { // �������������ֵ�����������ص�
                // ��������
                if (Connected_areas[tgt_cnt].area > 1) {
                    Connected_areas[tgt_cnt].location_x /= Connected_areas[tgt_cnt].area;
                    Connected_areas[tgt_cnt].location_y /= Connected_areas[tgt_cnt].area;
                }
                tgt_cnt++;
            }
        }
    }
    
    // ���û�ҵ������㣬�����ȫͼɨ��
    if (tgt_cnt < 2) {
        for (uint8 y = 0; y < MT9V03X_H && tgt_cnt < MAX_AREAS; y += 1) {
            for (uint8 x = 0; x < MT9V03X_W && tgt_cnt < MAX_AREAS; x += 1) {
                if (Image[y][x] == max_brightness) {
                    Connected_Find(x, y, &Connected_areas[tgt_cnt], max_brightness);
                    if (Connected_areas[tgt_cnt].area > 1) {
                        Connected_areas[tgt_cnt].location_x /= Connected_areas[tgt_cnt].area;
                        Connected_areas[tgt_cnt].location_y /= Connected_areas[tgt_cnt].area;
                        // ����������ͨ��
                        tgt_cnt++;
                    }
                }
            }
        }
    }
    
    // ѡ�� y ����������С�����������������ͨ��
    if (tgt_cnt >= 2) {
        uint8 best_i = 0, best_j = 1;
        uint32 y0 = Connected_areas[0].location_y;
        uint32 y1 = Connected_areas[1].location_y;
        uint32 best_dy = (y0 > y1) ? (y0 - y1) : (y1 - y0);
        uint32 best_area_sum = Connected_areas[0].area + Connected_areas[1].area;
        for (uint8 m = 0; m < tgt_cnt - 1; m++) {
            for (uint8 n = m + 1; n < tgt_cnt; n++) {
                uint32 ym = Connected_areas[m].location_y;
                uint32 yn = Connected_areas[n].location_y;
                uint32 dy = (ym > yn) ? (ym - yn) : (yn - ym);
                uint32 sum_area = Connected_areas[m].area + Connected_areas[n].area;
                if (dy < best_dy || (dy == best_dy && sum_area > best_area_sum)) {
                    best_i = m;
                    best_j = n;
                    best_dy = dy;
                    best_area_sum = sum_area;
                }
            }
        }
        area_num = 2;
        one_point.now_point[0] = (uint8)Connected_areas[best_i].location_x;
        one_point.now_point[1] = (uint8)Connected_areas[best_i].location_y;
        two_point.now_point[0] = (uint8)Connected_areas[best_j].location_x;
        two_point.now_point[1] = (uint8)Connected_areas[best_j].location_y;
    } else if (tgt_cnt == 1) {
        area_num = 1;
        one_point.now_point[0] = (uint8)Connected_areas[0].location_x;
        one_point.now_point[1] = (uint8)Connected_areas[0].location_y;
    } else {
        area_num = 0;
    }
}

uint8 Seek_Start(point *point1, point *point2, uint8 area)
{
    uint8 max_brightness = Get_Max_Brightness();
    uint8 found_cnt = 0;

    Connected_area temp_area[2];
    for(uint8 i = 0; i < 2; i++)
    {
        temp_area[i].area = 0;
        temp_area[i].location_x = 0;
        temp_area[i].location_y = 0;
    }

    uint8 *points[2] = { point1 ? point1->now_point : NULL, point2 ? point2->now_point : NULL };

    for(uint8 idx = 0; idx < 2; idx++)
    {
        if(!points[idx]) continue;
        int cx = points[idx][0];
        int cy = points[idx][1];
        bool started = false;
        for(int dy = -(int)area; dy <= (int)area && !started; dy++)
        {
            int y = cy + dy;
            if(y < 0 || y >= MT9V03X_H) continue;
            for(int dx = -(int)area; dx <= (int)area; dx++)
            {
                int x = cx + dx;
                if(x < 0 || x >= MT9V03X_W) continue;
                if(Image[y][x] == max_brightness)
                {
                    Connected_Find((uint8)x, (uint8)y, &Connected_areas[found_cnt], max_brightness);
                    if(Connected_areas[found_cnt].area > 0)
                    {
                        Connected_areas[found_cnt].location_x /= Connected_areas[found_cnt].area;
                        Connected_areas[found_cnt].location_y /= Connected_areas[found_cnt].area;
                        // ���������λ�õ�����
                        points[idx][0] = (uint8)Connected_areas[found_cnt].location_x;
                        points[idx][1] = (uint8)Connected_areas[found_cnt].location_y;
                        found_cnt++;
                        started = true;
                        break;
                    }
                }
            }
        }
        if(found_cnt == 2) break;
    }

    if(found_cnt == 2)
    {
        point1->now_point[0] = temp_area[0].location_x;
        point1->now_point[1] = temp_area[0].location_y;
        point2->now_point[0] = temp_area[1].location_x;
        point2->now_point[1] = temp_area[1].location_y;
        return 1;
    }

    return 0;
}

void Update_Points(point *point1, point *point2) {
    memcpy(point1->llast_point, point1->last_point, sizeof(point1->last_point));
    memcpy(point1->last_point, point1->now_point, sizeof(point1->now_point));
    memcpy(point2->llast_point, point2->last_point, sizeof(point2->last_point));
    memcpy(point2->last_point, point2->now_point, sizeof(point2->now_point));
}

float Deal_variance(void)
{
    // 1. ��ʼ���ģ�ʹ�����е㣩
    uint16 area0 = Connected_areas[0].area;
    uint16 area1 = Connected_areas[1].area;
    uint16 total_pts0 = area0 + area1;
    if (total_pts0 == 0) return 0.0f;

    float sum_x0 = 0.0f, sum_y0 = 0.0f;
    for (uint16 i = 0; i < area0; i++) {
        sum_x0 += Connected_areas[0].all_point[i][0];
        sum_y0 += Connected_areas[0].all_point[i][1];
    }
    for (uint16 i = 0; i < area1; i++) {
        sum_x0 += Connected_areas[1].all_point[i][0];
        sum_y0 += Connected_areas[1].all_point[i][1];
    }
    float cx0 = sum_x0 / total_pts0;
    float cy0 = sum_y0 / total_pts0;

    // 2. ����������벢ȷ���쳣��ֵ
    float sum_d20 = 0.0f;
    for (uint16 i = 0; i < area0; i++) {
        float dx0 = Connected_areas[0].all_point[i][0] - cx0;
        float dy0 = Connected_areas[0].all_point[i][1] - cy0;
        sum_d20 += dx0 * dx0 + dy0 * dy0;
    }
    for (uint16 i = 0; i < area1; i++) {
        float dx0 = Connected_areas[1].all_point[i][0] - cx0;
        float dy0 = Connected_areas[1].all_point[i][1] - cy0;
        sum_d20 += dx0 * dx0 + dy0 * dy0;
    }
    float mean_d20 = sum_d20 / total_pts0;
    const float OUTLIER_FACTOR = 4.0f;
    float thresh_sq = mean_d20 * OUTLIER_FACTOR;

    // 3. �޳��쳣�㲢���¼�������
    float sum_x1 = 0.0f, sum_y1 = 0.0f;
    uint16 count1 = 0;
    for (uint16 i = 0; i < area0; i++) {
        float dx0 = Connected_areas[0].all_point[i][0] - cx0;
        float dy0 = Connected_areas[0].all_point[i][1] - cy0;
        float d2 = dx0 * dx0 + dy0 * dy0;
        if (d2 <= thresh_sq) {
            sum_x1 += Connected_areas[0].all_point[i][0];
            sum_y1 += Connected_areas[0].all_point[i][1];
            count1++;
        }
    }
    for (uint16 i = 0; i < area1; i++) {
        float dx0 = Connected_areas[1].all_point[i][0] - cx0;
        float dy0 = Connected_areas[1].all_point[i][1] - cy0;
        float d2 = dx0 * dx0 + dy0 * dy0;
        if (d2 <= thresh_sq) {
            sum_x1 += Connected_areas[1].all_point[i][0];
            sum_y1 += Connected_areas[1].all_point[i][1];
            count1++;
        }
    }
    if (count1 == 0) return 0.0f;
    float cx = sum_x1 / count1;
    float cy = sum_y1 / count1;

    // 4. ���������ļ��㷽��
    float var_sum = 0.0f;
    for (uint16 i = 0; i < area0; i++) {
        float dx = Connected_areas[0].all_point[i][0] - cx;
        float dy = Connected_areas[0].all_point[i][1] - cy;
        float d2 = dx * dx + dy * dy;
        if (d2 <= thresh_sq) var_sum += d2;
    }
    for (uint16 i = 0; i < area1; i++) {
        float dx = Connected_areas[1].all_point[i][0] - cx;
        float dy = Connected_areas[1].all_point[i][1] - cy;
        float d2 = dx * dx + dy * dy;
        if (d2 <= thresh_sq) var_sum += d2;
    }
    return var_sum / count1;
}

float get_distance(float y)
{
    return 79.6174 * expf(-0.0277 * y) + 59.6435 * expf(-0.0025 * y);
}

void Calculate_Error(void) {
    // 1. ����ƽ�� x ���꣨���㱣��С������������
    float avg_x;
    if (area_num == 2) {
        avg_x = ((float)Connected_areas[0].location_x + (float)Connected_areas[1].location_x) * 0.5f;
    } else if (area_num == 1) {
        avg_x = (float)Connected_areas[0].location_x;
    } else {
        return;  // ûĿ��ʱֱ�ӷ���
    }
    error[0] = avg_x;
    float var_y = Deal_variance();
    float y = get_distance(var_y);
    error[2] = y;
    error[1] = var_y;
}
