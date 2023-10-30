//************************************************************************
// 文件名称: EcgFilter.h
//
// 文件说明: 定义Ecg滤波函数相关函数及变量
//
// 输入数据：	采样率: 250Hz、500Hz、1000Hz 三种采样率
//				采样精度: 最高支持24位采样精度
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************

#ifndef _ECGFILTER_H_
#define _ECGFILTER_H_

#include "EcgMainProc.h"


//=========================================================================
// 定义常量
//=========================================================================
// 外部显示滤波延迟
#define DELAY_VIEW_HP05		499			// 0.5Hz高通延时，499点（500Hz）
#define DELAY_VIEW_LP20			11			// 20Hz低通延时，11点（500Hz）
#define DELAY_VIEW_LP40			4				// 40Hz低通延时，4点（500Hz）

// 算法内部滤波延迟
#define HIGH_PASS_DELAY		32			//5Hz高通FIR滤波延迟，32点（250Hz）
#define LOW_PASS_DELAY		4				//20Hz低通FIR滤波延迟，4点（250Hz）
#define DIFF_PASS_DELAY			2				//差分滤波延迟，2点（250Hz）
#define BAND_PASS_DELAY		HIGH_PASS_DELAY + LOW_PASS_DELAY			//带通滤波延迟，36点（250Hz）



//======================================================================
// Methods declaration
//======================================================================
void InitializeEcgFilter(void);

int Fir20LowPass(int data, bool reset);
int Fir5HighPass(int data, bool reset);
int IndFir5HighPass(int data, bool reset);
int EcgDiffFilter(int data, bool reset);
int SquareIntegration(int data, bool reset);

#endif
