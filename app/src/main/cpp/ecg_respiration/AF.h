//*************************************************************************
//�ļ����ƣ�AF.h
//
//�ļ�˵����ECG�ĵ�Rֵ,ɢ��ͼ�ж��Ƿ񷿲�
//
//������ʷ��Powered by MeiHuan [8 / 10 / 2018]
//*************************************************************************

#ifndef	_AF_H_
#define _AF_H_

extern unsigned char af_buf[];
extern unsigned char af_buf_num;
extern unsigned char af_24[4];
extern unsigned char af_24_num;

//AF�㷨��ʼ��
void ecg_af_init(void);

//��ȡRֵ
void get_Rvalue(unsigned  value);

//��ʼɢ��ͼ���㣬���ط������
char ecg_af_begin(void);

#endif
