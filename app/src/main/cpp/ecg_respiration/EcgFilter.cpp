//************************************************************************
// 文件名称: EcgFilter.c
//
// 文件说明: 定义Ecg滤波函数相关函数及变量
//
// 输入数据：	采样率: 250Hz、500Hz、1000Hz 三种采样率
//				采样精度: 最高支持24位采样精度
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************

//#include "stdafx.h"
#include "EcgFilter.h"


//=========================================================================
//定义FIR滤波器的相关长度系数
//=========================================================================
#define FIR_LP20_LENGTH 5
#define FIR_LP20_250	4

#define FIR_HP5_LENGTH	33
#define FIR_HP5_250		32

#define ECG_DIFF_LENGTH	5
#define ECG_DIFF_250	4

#define SQUARE_INTEG_LENGTH	33
#define SQUARE_INTEG_250	32

//======================================================================
// 搜波算法滤波器变量定义
//======================================================================

// 20HzFir低通滤波器，第一级缓存区，最高支持1000Hz采样率，缓存长度位16点
int gFir20LpFormerBuff[FIR_LP20_LENGTH];
int gFir20LpFormerSum;

int gFir20LpLatterBuff[FIR_LP20_LENGTH];
int gFir20LpLatterSum;
int gFir20LpPtr;

// 5HzFir高通滤波器，第一级缓存区，最高支持1000Hz采样率，缓存长度为100点
int gFir5HpFormerBuff[FIR_HP5_LENGTH];
int gFir5HpFormerSum;

int gFir5HpLatterBuff[FIR_HP5_LENGTH];
int gFir5HpLatterSum;
int gFir5HpPtr;

// 5HzFir独立高通滤波器，第一级缓存区，最高支持1000Hz采样率，缓存长度为100点
int gIndFir5HpFormerBuff[FIR_HP5_LENGTH];
int gIndFir5HpFormerSum;

int gIndFir5HpLatterBuff[FIR_HP5_LENGTH];
int gIndFir5HpLatterSum;
int gIndFir5HpPtr;

// 差分滤波器循环缓存区，长度为16点
int gDiffFilterBuff[ECG_DIFF_LENGTH];
int gDiffPtr;

// 平方积分缓存区，长度为128点
int gSquareIntegBuff[SQUARE_INTEG_LENGTH];
int gSquareIntegPtr;
int gSquareIntegSum;


//************************************************************************
// 函数名称: void InitializeEcgFilter(void)
//
// 函数说明: 滤波器初始化函数
//
// 输入变量: 无	
//
// 输出变量: 无
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
void InitializeEcgFilter(void)
{
	//======================================================================
	// 搜波滤波部分初始化
	//======================================================================
	Fir20LowPass(0, true);
	Fir5HighPass(0, true);
	IndFir5HighPass(0, true);
	EcgDiffFilter(0, true);
	SquareIntegration(0, true);

	//	返回
	return ;
}


//======================================================================
// 搜波算法采用的相关滤波器
//======================================================================

//************************************************************************
// 函数名称: int Fir20LowPass(int data,   bool reset)
//
// 函数说明: 20Hz FIR低通滤波器
//
// 输入变量: int data:   滤波前的输入数据
//			 int gEcgOrgSampleRate:  采样率
//			 bool reset: 初始化标志	
//
// 输出变量: int result: 滤波后的数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int Fir20LowPass(int data, bool reset)
{
	int temp = 0, i = 0, final_result = 0, sub_ptr = 0;
	int buffer_len = 0, shift_bit = 0;

	if (reset)
	{
		for (i = 0; i < FIR_LP20_LENGTH; i++)
		{
			gFir20LpFormerBuff[i] = 0;
			gFir20LpLatterBuff[i] = 0;
		}
		gFir20LpPtr = 0;
		gFir20LpFormerSum = 0;
		gFir20LpLatterSum = 0;
	}
	else
	{
		//======================================================================
		// Part 1: 根据采样率赋值相应的系数，目前仅支持250Hz
		//======================================================================
		buffer_len = FIR_LP20_250;
		shift_bit = 2;

		//======================================================================
		// Part 1: 第一级平均
		//======================================================================
		sub_ptr = LoopInc(gFir20LpPtr, 1, (buffer_len + 1));

		gFir20LpFormerBuff[gFir20LpPtr] = data;

		gFir20LpFormerSum += data;
		gFir20LpFormerSum -= gFir20LpFormerBuff[sub_ptr];

		temp = gFir20LpFormerSum >> shift_bit;

		//======================================================================
		// Part 2: 第二级平均，然后输出结果
		//======================================================================
		gFir20LpLatterBuff[gFir20LpPtr] = temp;
		gFir20LpLatterSum += temp;
		gFir20LpLatterSum -= gFir20LpLatterBuff[sub_ptr];

		final_result = gFir20LpLatterSum >> shift_bit;

		// 指针自增，指向下一个位置
		gFir20LpPtr = LoopInc(gFir20LpPtr, 1, (buffer_len + 1));
	}

	return final_result;
}

//************************************************************************
// 函数名称: int Fir5HighPass(int data,   bool reset)
//
// 函数说明: 4.5Hz FIR高通滤波器
//
// 输入变量: int data:   滤波前的输入数据
//			 int gEcgOrgSampleRate:  采样率
//			 bool reset: 初始化标志	
//
// 输出变量: int result: 滤波后的数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int Fir5HighPass(int data, bool reset)
{
	int i = 0, sub_ptr = 0, add_ptr = 0, temp = 0, final_result = 0;
	int buffer_len = 0, section_num = 0, shift_bit = 0;

	if (reset)
	{
		for (i = 0; i < FIR_HP5_LENGTH; i++)
		{
			gFir5HpFormerBuff[i] = 0;
			gFir5HpLatterBuff[i] = 0;
		}
		gFir5HpFormerSum = 0;
		gFir5HpLatterSum = 0;
		gFir5HpPtr = 0;
	}
	else
	{
		//======================================================================
		// Part 1: 根据采样率赋值相应的系数，仅支持250Hz
		//======================================================================
		buffer_len = FIR_HP5_250;
		shift_bit = 5;

		//======================================================================
		// Part 1: 第一级平均
		//======================================================================
		sub_ptr = LoopInc(gFir5HpPtr, 1, (buffer_len + 1));

		gFir5HpFormerBuff[gFir5HpPtr] = data;

		gFir5HpFormerSum += data;
		gFir5HpFormerSum -= gFir5HpFormerBuff[sub_ptr];

		temp = gFir5HpFormerSum >> shift_bit;

		//======================================================================
		// Part 2: 第二级平均，然后输出结果
		//======================================================================
		gFir5HpLatterBuff[gFir5HpPtr] = temp;
		gFir5HpLatterSum += temp;
		gFir5HpLatterSum -= gFir5HpLatterBuff[sub_ptr];

		temp = gFir5HpLatterSum >> shift_bit;

		//======================================================================
		// Part 3: 原始数据延时，然后减去基线，得到高通数据
		//======================================================================
		//final_result = gFir5HpFormerBuff[sub_ptr] - (temp >> shift_bit);
		final_result = gFir5HpFormerBuff[sub_ptr] - temp;

		// 指针指向循环缓存区的下一个位置
		gFir5HpPtr = LoopInc(gFir5HpPtr, 1, (buffer_len + 1));
	}

	return final_result;
}


//************************************************************************
// 函数名称: int Fir5HighPassInd(int data,   bool reset)
//
// 函数说明: 4.5Hz FIR高通滤波器
//					该函数独立使用，用于获取高通滤波数据
//
// 输入变量: int data:   滤波前的输入数据
//			 int gEcgOrgSampleRate:  采样率
//			 bool reset: 初始化标志	
//
// 输出变量: int result: 滤波后的数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int IndFir5HighPass(int data, bool reset)
{
	int i = 0, sub_ptr = 0, add_ptr = 0, temp = 0, final_result = 0;
	int buffer_len = 0, section_num = 0, shift_bit = 0;

	if (reset)
	{
		for (i = 0; i < FIR_HP5_LENGTH; i++)
		{
			gIndFir5HpFormerBuff[i] = 0;
			gIndFir5HpLatterBuff[i] = 0;
		}
		gIndFir5HpFormerSum = 0;
		gIndFir5HpLatterSum = 0;
		gIndFir5HpPtr = 0;
	}
	else
	{
		//======================================================================
		// Part 1: 根据采样率赋值相应的系数，仅支持250Hz
		//======================================================================
		buffer_len = FIR_HP5_250;
		shift_bit = 5;

		//======================================================================
		// Part 1: 第一级平均
		//======================================================================
		sub_ptr = LoopInc(gIndFir5HpPtr, 1, (buffer_len + 1));

		gIndFir5HpFormerBuff[gIndFir5HpPtr] = data;

		gIndFir5HpFormerSum += data;
		gIndFir5HpFormerSum -= gIndFir5HpFormerBuff[sub_ptr];

		temp = gIndFir5HpFormerSum >> shift_bit;

		//======================================================================
		// Part 2: 第二级平均，然后输出结果
		//======================================================================
		gIndFir5HpLatterBuff[gIndFir5HpPtr] = temp;
		gIndFir5HpLatterSum += temp;
		gIndFir5HpLatterSum -= gIndFir5HpLatterBuff[sub_ptr];

		temp = gIndFir5HpLatterSum >> shift_bit;

		//======================================================================
		// Part 3: 原始数据延时，然后减去基线，得到高通数据
		//======================================================================
		//final_result = gFir5HpFormerBuff[sub_ptr] - (temp >> shift_bit);
		final_result = gIndFir5HpFormerBuff[sub_ptr] - temp;

		// 指针指向循环缓存区的下一个位置
		gIndFir5HpPtr = LoopInc(gIndFir5HpPtr, 1, (buffer_len + 1));
	}

	return final_result;
}


//************************************************************************
// 函数名称: int EcgDiffFilter(int data,   bool reset)
//
// 函数说明: 差分滤波器
//
// 输入变量: int data:   滤波前的输入数据
//			 int gEcgOrgSampleRate:  采样率
//			 bool reset: 初始化标志	
//
// 输出变量: int result: 滤波后的数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int EcgDiffFilter(int data, bool reset)
{
	int i = 0, result = 0, buffer_len = 0, sub_ptr = 0;

	if (1 == reset)
	{
		for (i = 0; i < ECG_DIFF_LENGTH; i++)
		{
			gDiffFilterBuff[i] = 0;
		}
		gDiffPtr = 0;
	}
	else
	{
		//=========================================================================
		//1. 根据采样率赋值缓存区长度，仅支持250Hz
		//=========================================================================
		buffer_len = ECG_DIFF_250;	

		//=========================================================================
		//2. 缓存区赋值，计算差分结果，维护队列指针
		//=========================================================================
		sub_ptr = LoopInc(gDiffPtr, 1, buffer_len);

		gDiffFilterBuff[gDiffPtr] = data;
		result = gDiffFilterBuff[gDiffPtr] * 3 - gDiffFilterBuff[sub_ptr] * 3;

		gDiffPtr = LoopInc(gDiffPtr, 1, buffer_len);
	}

	return result;
}

//************************************************************************
// 函数名称: int SquareIntegration(int data,   bool reset)
//
// 函数说明: 取绝对值（平方太大，不适用于24bit数据），然后进行滑动积分
//
// 输入变量: int data:   滤波前的输入数据
//			 int gEcgOrgSampleRate:  采样率
//			 bool reset: 初始化标志	
//
// 输出变量: int result: 滤波后的数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int SquareIntegration(int data, bool reset)
{
	int i = 0, result = 0, buffer_len = 0, sub_ptr = 0, shift_bit = 0;

	if (1 == reset)
	{
		for (i = 0; i < SQUARE_INTEG_LENGTH; i++)
		{
			gSquareIntegBuff[i] = 0;
		}
		gSquareIntegPtr = 0;
		gSquareIntegSum = 0;
	}
	else
	{
		//=========================================================================
		//1. 根据采样率赋值缓存区长度，250Hz
		//=========================================================================
		buffer_len = SQUARE_INTEG_250;
		shift_bit = 5;

		//=========================================================================
		//2. 取绝对值，并赋值缓存区
		//=========================================================================
		sub_ptr = LoopInc(gSquareIntegPtr, 1, (buffer_len + 1));
		gSquareIntegBuff[gSquareIntegPtr] = (data >= 0) ? data : (-data);	//为避免大数运算，采用绝对值

		//=========================================================================
		//3. 滑动积分
		//=========================================================================
		gSquareIntegSum += gSquareIntegBuff[gSquareIntegPtr];
		gSquareIntegSum -= gSquareIntegBuff[sub_ptr];

		result = gSquareIntegSum >> shift_bit;

		//维护指针，指向下一个点
		gSquareIntegPtr = LoopInc(gSquareIntegPtr, 1, (buffer_len + 1));
	}

	return result;
}

