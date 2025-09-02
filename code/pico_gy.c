// Э��汾3.6.3
#include "pico_gy.h"

#define BUFFER_SIZE (300) // ���ͻ�������С, �����Դ���ͼ���ֽ���, ̫���ռ�ù����ڴ�
static pico_uint8 gy_buffer[BUFFER_SIZE] = {0};
static pico_uint32 len = 0;

// ������������
void gy_buffer_put(void);
void gy_buffer_add(pico_uint8 dat);
void gy_buffer_add_array(pico_uint8* dat, pico_uint32 data_len);

/*
 * GL-Shw���ܳ���λ��  qqȺ:297809802 qq1849057843
 *
 *Ŀ¼��
 *|||||||||||||||||||������ͼ��Э��ֻ��ѡ��һ�֡�
 * һ���Ҷ�ͼ��
 *  ����ͨ�Ҷ�ͼ��
 *  �ڿ����ŻҶ�ͼ��
 *  ��ѹ���Ҷ�ͼ��
 * ������ֵ��ͼ����*��Э�����40�����ҵ�ѹ���ʺ�У�� ��[ǿ���Ƽ�]
 *  �ٶ�ֵ��ͼ��
 *  �ڴ�У���ֵ��ͼ��
 *  ���޸�ͼ���С�Ķ�ֵ��ͼ��
 *
 * ||||||||||||||||||������Э�������ͼ��Э�鲢��ʹ�á�
 * ����ͼ���߽磨Ѱ�ߵı߽���Ϣ���Ƽ������ֵͼ����ʵʱ�鿴������Ѱ�߲��ߣ�[�Ƽ�]
 * �ġ����ݱ�ǩ������ʾ����������ɫ�궨�����ݼ��ӣ��Ǳ��̣�[ǿ���Ƽ�]
 *                          ����ʱ�䣺2022��12��9��
 */

//--------------��ͨ�Ҷ�ͼ��-------------------//
//*imageͼ���ַ widthͼ��� heightͼ���
// ����sendimg(mt9v03x_image_dvp[0], MT9V03X_DVP_W, MT9V03X_DVP_H);
// ���ݰ���С:6+width * height(ͼ��һ֡���ֽ���)
void sendimg(pico_uint8 *image, pico_uint8 width, pico_uint8 height)
{
    pico_uint8 dat[6] = { 0x21, 0x7A, width, height, 0x21, 0x7A };
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, dat, 6);
    pico_multi_src_add(&multi_src, image, width * height);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}
//--------------�����ŻҶ�ͼ��-------------------//
// ����ʧ���ݵ�����£���Э�������¶�λ����ʵ��һ���̶ȿ���������
//*imageͼ���ַ widthͼ��� heightͼ���
// ����sendimg(mt9v03x_image_dvp[0], MT9V03X_DVP_W, MT9V03X_DVP_H);
// ���ݰ���С:6+��width+3�� * height(ͼ��һ֡���ֽ���)
void sendimg_A(pico_uint8 *image, pico_uint8 width, pico_uint8 height)
{
    gy_buffer[len++] = 0x21; 
    gy_buffer[len++] = 0x7A;
    gy_buffer[len++] = width;
    gy_buffer[len++] = height;
    gy_buffer[len++] = (width+height)/2;
    gy_buffer[len++] = 0x7A;

    pico_uint8 line=0,col=0;
    for(line=0;line<width;line++)
    {
        gy_buffer[len++] = 21;
        gy_buffer[len++] = line;
        gy_buffer[len++] = 133;
        for(col=0;col<height;col++)
        {
            gy_buffer[len++] = *(image+line*height+col);
        }
    }
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, gy_buffer, len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
    len = 0;
}
//--------------ѹ���Ҷ�ͼ��-------------------//
// ����ѹ��ͼ�� ���� 120x180��ͼ��̫�� �����ٶ���  ����Է��� 60x90��ͼ��������ٶ�
//*imageͼ���ַ widthͼ��� heightͼ���dis_widthѹ�����ͼ��� dis_heightѹ�����ͼ���
// ����sendimg(mt9v03x_image_dvp[0], MT9V03X_DVP_W, MT9V03X_DVP_H,MT9V03X_DVP_W/2,MT9V03X_DVP_H/2);
// ���ݰ���С:6+dis_width * dis_height(ͼ��һ֡���ֽ���)
void sendimg_zoom(pico_uint8 *image, pico_uint8 width, pico_uint8 height, pico_uint8 dis_width, pico_uint8 dis_height)
{
    pico_uint8 head[6] = {0x21, 0x7A, dis_width, dis_height, 0x21, 0x7A};
    pico_uint32 data_len = 0;
    for (pico_uint8 j = 0; j < dis_height; j++)
    {
        for (pico_uint8 i = 0; i < dis_width; i++)
        {
            gy_buffer[data_len++] = *(image + (j * height / dis_height) * width + i * width / dis_width); // ��ȡ���ص�
        }
    }
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, head, 6);
    pico_multi_src_add(&multi_src, gy_buffer, data_len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}

//--------------��ֵ��ͼ��-------------------//
// imageͼ���ַ widthͼ��� heightͼ��� otu��ֵ����ֵ
// ����sendimg_binary(mt9v03x_image_dvp[0], MT9V03X_DVP_W, MT9V03X_DVP_H,100);
void sendimg_binary(pico_uint8 *image, pico_uint8 width, pico_uint8 height, pico_uint8 otu)
{
    pico_uint8 head[6] = {0x7A, 0x21, width, height, 0x7A, 0x21};
    int databool = 255;
    pico_uint8 lon = 1;
    int data = 255;
    pico_uint8 line = 0, col = 0;
    pico_uint32 data_len = 0;
    for (line = 0; line < width; line++)
    {
        for (col = 0; col < height; col++)
        {
            if (*(image + line * height + col) > otu)
                data = 255;
            else
                data = 0;
            if (data == databool)
            {
                lon++;
            }
            else
            {
                gy_buffer[data_len++] = lon;
                lon = 1;
            }
            if (lon == 190)
            {
                gy_buffer[data_len++] = lon - 1;
                gy_buffer[data_len++] = 0;
                lon = 1;
            }
            databool = data;
        }
    }
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, head, 6);
    pico_multi_src_add(&multi_src, gy_buffer, data_len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}
// ѹ����ֵͼ��
// pico_uint16 dis_width, pico_uint16 dis_height Ҫѹ��ͼ���С
void sendimg_binary_zoom(pico_uint8 *image, pico_uint8 width, pico_uint8 height, pico_uint8 dis_width, pico_uint8 dis_height, pico_uint8 otu)
{
    pico_uint8 head[6] = {0x7A, 0x21, dis_width, dis_height, 0x7A, 0x21};
    int databool = 255;
    pico_uint8 lon = 1;
    int data = 255;
    pico_uint8 i, j;
    pico_uint32 data_len = 0;
    for (j = 0; j < dis_height; j++)
    {
        for (i = 0; i < dis_width; i++)
        {
            if (*(image + (j * height / dis_height) * width + i * width / dis_width) > otu) // ��ȡ���ص�
                data = 255;
            else
                data = 0;
            if (data == databool)
            {
                lon++;
            }
            else
            {
                gy_buffer[data_len++] = lon;
                lon = 1;
            }
            if (lon == 190)
            {
                gy_buffer[data_len++] = lon - 1;
                gy_buffer[data_len++] = 0;
                lon = 1;
            }
            databool = data;
        }
    }
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, head, 6);
    pico_multi_src_add(&multi_src, gy_buffer, data_len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}
// ����У��Ķ�ֵͼ��
// chkֵԽ�� ������Խǿ ֵ0-55
// �����ʵ��ʹ��������е���
void sendimg_binary_CHK(pico_uint8 *image, pico_uint8 width, pico_uint8 height, pico_uint8 otu, pico_uint8 chk)
{
    chk = chk > 0 ? chk : 0;
    chk = chk < 56 ? chk : 55;
    pico_uint8 dat[7] = {0x7A, 0x21, width, height, 0x7A, 0x21, 200 + chk};
    gy_buffer[len++] = 0x7A;
    gy_buffer[len++] = 0x21;
    gy_buffer[len++] = width;
    gy_buffer[len++] = height;
    gy_buffer[len++] = 0x7A;
    gy_buffer[len++] = 0x21;
    gy_buffer[len++] = 200 + chk;
    
    int databool = 255;
    pico_uint8 lon = 0;
    int data = 255;
    pico_uint8 line = 0, col = 0;
    int imglon = 0;
    int imgdatlo = width * height / chk;
    pico_uint8 CHK = 0;
    
    for (line = 0; line < width; line++)
    {
        for (col = 0; col < height; col++)
        {
            imglon++;
            if (*(image + line * height + col) > otu)
                data = 255;
            else
                data = 0;
            if (data == databool)
            {
                lon++;
            }
            else
            {
                gy_buffer[len++] = lon;
                lon = 1;
            }
            if (imglon == imgdatlo)
            {
                CHK++;
                gy_buffer[len++] = lon;
                data = 255;
                databool = 255;
                gy_buffer[len++] = 200 + CHK;
                lon = 0;
                imglon = 0;
            }
            if (lon == 190)
            {
                gy_buffer[len++] = lon;
                gy_buffer[len++] = 0;
                lon = 0;
            }
            databool = data;
        }
    }
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, gy_buffer, len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
    len = 0;
}

//--------------��ֵ��ͼ�����߽�-------------------//
//! ע��:�ȷ��߽� �� ��־�� ��ͼ����ͼ��
//----------------------���ͻ�����------------------------------
#define Line_SIZE (300) // �߽绺������С ��̫�� ���Լ�������� (�߽糤��+5)*�߽����+7*��־�����
static pico_uint8 Line_Buffer[Line_SIZE] = {0};
static int Line_lon = 0;
// �߽� *�߽����Ͷ�����Ҫ��uint8
void sendimgAndLine(pico_uint8 color, pico_uint8 *buff, pico_uint32 len)
{
    Line_Buffer[Line_lon]=(0x21);Line_lon++;
    Line_Buffer[Line_lon]=(color);Line_lon++;
    Line_Buffer[Line_lon]=(len);Line_lon++;
    Line_Buffer[Line_lon]=(255);Line_lon++;
    Line_Buffer[Line_lon]=(255);Line_lon++;
    for(int i=0;i<len;i++){ Line_Buffer[Line_lon]=*(buff+i);Line_lon++;}
}
/*���Ʊ���   color������ɫ  uint8_t *buff ���͵ı��������ַ  len���͵ı��߳���
 * �����ű߽��������int���� size_lon��4(int ռ4�ֽ�)
 *�����ű߽��������uint6���� size_lon��2(uint6 ռ2�ֽ�)
 * �����ű߽��������uint8���� size_lon��1(pico_uint8 ռ1�ֽ�) ��ͬ��sendline( pico_uint8 color,pico_uint8 *buff, pico_uint32 len)
 */
void sendline_type(pico_uint8 color, pico_uint8 *buff, pico_uint32 len, pico_uint8 size_lon)
{
    gy_buffer[len++] = 0x21;
    gy_buffer[len++] = color;
    gy_buffer[len++] = len;
    gy_buffer[len++] = 255;
    gy_buffer[len++] = 255;
    for(int i=0;i<len*size_lon;i+=size_lon)
    {
        gy_buffer[len++] = *(buff+i);
    }
    pico_send_8bit_array(gy_buffer, len);
    len = 0;
}
// #define swj_point_type1 1 //Сʮ�� 3x3
// #define swj_point_type2 2 //��ʮ�� 5x5
// #define swj_point_type3 3 //��ʮ�� 7x7
// #define swj_point_type4 4 //С  X  3x3
// #define swj_point_type5 5 //��  X  5x5
// #define swj_point_type6 6 //��  X  7x7
// #define swj_point_type7 7 //ȫ��ʮ��
// #define swj_point_type8 8 //����һ��ֱ�� y������ x��������
// #define swj_point_type9 9 //����һ��ֱ�� x������ y��������
// ��־�㣨����յ� ���ⲹ�ߵĵ㣩
// ���� �㣨10,11��sendpoint(swj_RED,10,11,swj_point_type1);//��10��11�������ֺ�ɫСʮ��
// ��ɫ ����x ����y �����ͣ�������ĺ궨�壩

// ͼ��
void sendimg_BinaryAndLine(pico_uint8 *image, pico_uint8 width, pico_uint8 height, pico_uint8 otu)
{
    pico_uint8 chk = 50;
    pico_uint8 dat2[6]={0x77,0x22,0x77,0x22,Line_lon/255%255,Line_lon%255};
    gy_buffer[len++] = 0x77;
    gy_buffer[len++] = 0x22;
    gy_buffer[len++] = 0x77;
    gy_buffer[len++] = 0x22;
    gy_buffer[len++] = Line_lon/255%255;
    gy_buffer[len++] = Line_lon%255;
    
    pico_uint8 dat[7]={0x7A,0x21,width,height,0x7A,0x21,200+chk};
    gy_buffer[len++] = 0x7A;
    gy_buffer[len++] = 0x21;
    gy_buffer[len++] = width;
    gy_buffer[len++] = height;
    gy_buffer[len++] = 0x7A;
    gy_buffer[len++] = 0x21;
    gy_buffer[len++] = 200+chk;
    
    int databool=255;
    pico_uint8 lon=0;
    int data=255;
    pico_uint8 line=0,col=0;
    int imglon=0;
    int imgdatlo=width*height/chk;
    pico_uint8 CHK=0;
    
    for(line=0;line<width;line++)
    {
        for(col=0;col<height;col++)
        {
            imglon++;

            if(*(image+line*height+col)>otu)data=255;
            else data=0;
            if(data==databool)
            {lon++;}
            else{gy_buffer[len++]=lon;lon=1;}

            if(imglon==imgdatlo)
            {CHK++;gy_buffer[len++]=lon;data=255; databool=255;gy_buffer[len++]=200+CHK;lon=0;imglon=0;}
            if(lon==190){gy_buffer[len++]=lon;gy_buffer[len++]=0;lon=0;}
            databool=data;
        }
    }
    
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, gy_buffer, len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
    len = 0;
    
    pico_uint8 dat3[6]={0x34,0xFC,0x34,0xFC,0x34,0xFC};
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, dat3, 6);
    pico_multi_src_add(&multi_src, Line_Buffer, Line_lon);
    pico_send_8bit_array_multi_fd_spi(multi_src);
    Line_lon = 0;
}

// ��ɫͼ�� imageͼ���ַ lon���ݳ�
void sendimg_JPEG(pico_uint8 *image, int lon)
{
    pico_uint8 dat[7] = { 0x21,0xFE, 0x7A, lon/255, lon%255, 0x21, 0x7A };
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, dat, 7);
    pico_multi_src_add(&multi_src, image, lon);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}

// �ڲ�����
static inline pico_uint16 swj_CRC(pico_uint8 *Buf, pico_uint8 CRC_sta, pico_uint8 CRC_CNT)
{
    pico_uint16 CRC_Temp;
    pico_uint8 i, j;
    CRC_Temp = 0xffff;

    for (i = CRC_sta; i < CRC_CNT; i++)
    {
        CRC_Temp ^= Buf[i];
        for (j = 0; j < 8; j++)
        {
            if (CRC_Temp & 0x01)
                CRC_Temp = (CRC_Temp >> 1) ^ 0xa001;
            else
                CRC_Temp = CRC_Temp >> 1;
        }
    }
    return (CRC_Temp);
}

//--------------���ݱ�ǩ������ʾ����������ɫ�궨�����ݼ��ӣ��Ǳ��̣�-------------------//
// һ������ռһ����ַ,��ֱ�ӳ�����ͼ��ҳ������� ��һ�¿���ʾ�����Ҽ�����������ɫ�궨 �趨��
// ��ֵ����ɫ�궨���� ���ټ���������û�д�����Ҳ���԰󶨱��̣��ٶȸ�ֱ�ۡ�¼��ʱ��ͬ��¼��
// �����ں�ϻ�ӣ�С��ʲô״̬һ�۱�֪
// name���ݱ�ʶ(ͨ������ַ)[0-255]  dat:����
// ��:int a=0;put_int32(0,a);

// ����У�������ʾ��
void put_int32(pico_uint8 name, int dat)
{
    pico_uint8 datc[10] = { 197, name,1,0,0,0,0,0,0,198};
    datc[3] = (pico_uint8)(dat & 0xFF);
    datc[4] = (pico_uint8)((dat & 0xFF00) >> 8);
    datc[5] = (pico_uint8)((dat & 0xFF0000) >> 16);
    datc[6] = (pico_uint8)((dat >> 24) & 0xFF);
    pico_uint8 crc[6] = { name, 1,datc[3],datc[4],datc[5],datc[6]};
    pico_uint16 CRC16 = swj_CRC(crc,0,6);
    datc[7] = (pico_uint8)(CRC16&0xff);
    datc[8] = (pico_uint8)(CRC16>>8);
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, datc, 10);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}

void put_float(pico_uint8 name, float dat)
{
    pico_uint8 datc[10] = {197, name, 2, 0, 0, 0, 0, 0, 0, 198};
    char farray[4] = {0};

    *(float*)farray=dat;
    unsigned char *p = (unsigned char*)&dat + 3;
    datc[3] = *(p-3);
    datc[4] = *(p-2);
    datc[5] = *(p-1);
    datc[6] = *p;

    pico_uint8 crc[6] = { name, 2, datc[3], datc[4], datc[5], datc[6]};
    pico_uint16 CRC16 = swj_CRC(crc, 0, 6);
    datc[7] = (pico_uint8)(CRC16 & 0xff);
    datc[8] = (pico_uint8)(CRC16 >> 8);
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, datc, 10);
    pico_send_8bit_array_multi_fd_spi(multi_src);
}

//--------------����-------------------//
/*
 * ���Խ�Ѱ�ߵõ������ұ��ߣ�������߷��͵���λ���鿴
 * ���磺
 * ���� ͼ��img[H][W];�õ����ұ��ߴ����zx[H] yx[H] �����������Ϊw[H]
 * sendline_clear(swj_BLACK,W,H);//���� ������ɫ
 * sendline(swj_WHITE,zx,H);//���������
 * sendline(swj_WHITE,yx,H);//�����ұ���
 * sendline(swj_WHITE,wx,H);//��������
 * ���磺
 * sendline_clear(swj_BLACK,W,H);//���� ������ɫ
 * sendline_xy(zx,yx,H)//�������ұ���
 * sendline(swj_WHITE,wx,H);//��������
 *
 * ������������Ч��һ�µģ��������������λ���϶�Ӧʹ��������ԭ����
 * ע�⣺
 * ��ÿ������һ֡��ͼ����� ��Ҫ����sendline_clear��������
 * ���������sendline_xy������ʹ����λ��������ԭ����ʱ�����ٵ���sendline һ������sendline_xy���� ��ֹ������
 * */
// #define swj_BLACK 0
// #define swj_WHITE 1
// #define swj_RED 2
// #define swj_GREEN 3
// #define swj_BLUE 4
// #define swj_PURPLE 5
// #define swj_YELLOW 6
// #define swj_CYAN 7
// #define swj_ORANGE 8

// �������   color������ı�����ɫ  pico_uint16 width pico_uint16 height ͼ��Ĵ�С
void sendline_clear(pico_uint8 color, pico_uint8 width, pico_uint8 height)
{
    gy_buffer_add(0x21); 
    gy_buffer_add(0x7A);
    gy_buffer_add(width);
    gy_buffer_add(height);
    gy_buffer_add(color);
    gy_buffer_add(0x21);
    gy_buffer_put();
}

/*Ĭ��ÿ��һ����*/
// ���Ʊ���   color������ɫ  uint8_t *buff ���͵ı��������ַ  len���͵ı��߳���
void sendline(pico_uint8 color, pico_uint8 *buff, pico_uint32 len_val)
{
    gy_buffer_add(0x21);
    gy_buffer_add(color);
    gy_buffer_add(len_val);
    gy_buffer_add(255);
    gy_buffer_add(255);
    
    for(int i=0; i<len_val; i++)
    {
        gy_buffer_add(*(buff+i));
    }
}

/*˵��:
 * ������������ a(x1,y1)b(x2,y2)c(x3,y3)
 * �� pico_uint8 x[3]={x1,x2,x3};pico_uint8 y[3]={y1,y2,y3};
 *  sendline2(swj_WHITE,x,y,3);
 *  sendline����ֻ�ܰ�˳��ÿ��һ�㷢�ͱ߽��
 *  sendline2��������ڰ������ ÿ�в��̶������ı߽�
 *           Ҳ�����ڷ�������� ����յ� �������
 *
 * */
// ������Ʊ���  color������ɫ linex��Ӧ���x���꼯�� liney��Ӧ���y���꼯��  len���͵ı��߳���
void sendline2(pico_uint8 color, pico_uint8 *linex, pico_uint8 *liney, pico_uint32 len_val)
{
    gy_buffer_add(0x21);
    gy_buffer_add(color);
    gy_buffer_add(len_val);
    gy_buffer_add(254);
    gy_buffer_add(255);
    
    for(int i=0; i<len_val; i++)
    {
        gy_buffer_add(*(linex+i));
    }
    
    for(int i=0; i<len_val; i++)
    {
        gy_buffer_add(*(liney+i));
    }
}

// ͼ�������߽�  uint8_t *zx:��߽�   uint8_t *yx:�ұ߽�, uint32_t len���͵ı��߳���
// �ú������·ź����ֱ������������кβ�ͬ? �ú����ɶ�Ӧ��λ����ԭ�����Ĺ���*  ע���Ⱥ�˳��
void sendline_xy(pico_uint8 *line_zx, pico_uint8 *line_yx, pico_uint32 len_val)
{
    gy_buffer_add(0x21);
    gy_buffer_add(9);
    gy_buffer_add(len_val);
    gy_buffer_add(255);
    gy_buffer_add(255);
    
    for(int i=0; i<len_val; i++)
    {
        gy_buffer_add(*(line_zx+i));
    }
    
    for(int i=0; i<len_val; i++)
    {
        gy_buffer_add(*(line_yx+i));
    }
    
}

// #define swj_point_type1 1 //Сʮ�� 3x3
// #define swj_point_type2 2 //��ʮ�� 5x5
// #define swj_point_type3 3 //��ʮ�� 7x7
// #define swj_point_type4 4 //С  X  3x3
// #define swj_point_type5 5 //��  X  5x5
// #define swj_point_type6 6 //��  X  7x7
// #define swj_point_type7 7 //ȫ��ʮ��
// #define swj_point_type8 8 //����һ��ֱ�� y������ x��������
// #define swj_point_type9 9 //����һ��ֱ�� x������ y��������
// ��־�㣨����յ� ���ⲹ�ߵĵ㣩
// ���� �㣨10,11��sendpoint(swj_RED,10,11,swj_point_type1);//��10��11�������ֺ�ɫСʮ��
// ��ɫ ����x ����y �����ͣ�������ĺ궨�壩
void sendpoint(pico_uint8 color, pico_uint8 x, pico_uint8 y, pico_uint8 type)
{
    gy_buffer_add(0x22);
    gy_buffer_add(color);
    gy_buffer_add(type);
    gy_buffer_add(254);
    gy_buffer_add(x);
    gy_buffer_add(y);
    gy_buffer_add(255);

}

// ��ͼ��ʹ�ccd����ͬʱ����
void send_CCD(pico_uint16 *dat)
{
    pico_uint8 head[4] = {0x00, 0xff, 0x01, 0x00};
    
    for (pico_uint8 i = 0; i < 128; i++)
    {
        gy_buffer_add(dat[i]);
        gy_buffer_add(dat[i + 1]);
    }
    
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, head, 4);
    pico_multi_src_add(&multi_src, gy_buffer, len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
    len = 0;
}

void swj_stop()
{
    pico_uint8 buff[8] = {0xFB, 0xFE, 0x00, 0x20, 0xFA, 0x20, 0xFA, 0x20};
    gy_buffer_add_array(buff, 8);
    gy_buffer_put();
}

void swj_start()
{
    pico_uint8 buff[8] = {0xFB, 0xFE, 0x01, 0xEC, 0xFA, 0x20, 0xFA, 0x20};
    gy_buffer_add_array(buff, 8);
    gy_buffer_put();
}

// ��ӻ�����������ʵ��
void gy_buffer_put(void)
{
    pico_multi_src_t multi_src;
    pico_multi_src_clear(&multi_src);
    pico_multi_src_add(&multi_src, gy_buffer, len);
    pico_send_8bit_array_multi_fd_spi(multi_src);
    len = 0;
}

void gy_buffer_add(pico_uint8 dat)
{
    gy_buffer[len++] = dat;
    if(len == BUFFER_SIZE) gy_buffer_put();
}

void gy_buffer_add_array(pico_uint8* dat, pico_uint32 data_len)
{
    for(pico_uint32 i = 0; i < data_len; i++)
        gy_buffer_add(*(dat + i));
}
