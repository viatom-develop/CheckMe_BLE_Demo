//*************************************************************************
//文件名称：AF.h
//
//文件说明：ECG心电R值,散点图判断是否房颤
//
//更改历史：Powered by MeiHuan [8 / 10 / 2018]
//*************************************************************************

#ifndef	_AF_H_
#define _AF_H_

extern unsigned char af_buf[];
extern unsigned char af_buf_num;
extern unsigned char af_24[4];
extern unsigned char af_24_num;

//AF算法初始化
void ecg_af_init(void);

//获取R值
void get_Rvalue(unsigned  value);

//开始散点图计算，返回房颤结果
char ecg_af_begin(void);

#endif
