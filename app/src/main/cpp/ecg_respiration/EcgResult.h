//*************************************************************************
//文件名称：EcgResult.cpp
//
//文件说明：Ecg参数计算函数
//
//更改历史：Created by Chenmaomao [5 / 15 / 2014]
//*************************************************************************



#ifndef _ECGRESULT_H_
#define _ECGRESULT_H_

#include "EcgMainProc.h"
//=========================================================================
//1. 定义局部结构体
//=========================================================================
typedef struct Local_Peak
{
	int nStartPos;								//peak起点位置
	int nStartValue;								//peak起点的值
	int nPeakPos;								//peak波峰位置
	int nPeakValue;								//peak波峰的值
	int nPeakHight;								//peak幅度
}LOCAL_PEAK;


//=========================================================================
//1. 定义函数
//=========================================================================
// 计算初始化
void EcgResultInitialize(void);

// 计算心率
int CalculateHeartRate(bool reset);

// 搜索当前QRS波的IOS点，计算上一个QRS波的ST值
int CalculateStValue(bool reset);

// 计算上一个QRS波的QT值
int CalculateQTValue(bool reset);

// 获取算法最终分析结果（对外接口）
int EcgAlgGetFinalResult(ECG_FINAL_RESULT *final_result, bool reset);

// 计算HRV相关参数
int CalculateFinalHrv(void);

// 判断不规则节律
int JudgeIrregRhythm(int rri_num, int *rri_value, int *rri_sort);

// 获取全部的QRS波峰位置（对外接口，提供给PWTT）
int EcgAlgGetQrsPeakPos(int *qrs_pos_buff);


//=========================================================================
//1. Holter相关接口函数
//=========================================================================

// 从Flash送入所有的RR间期，计算最终的HRV
int EcgAlgCalculateHolterHRV(unsigned short curRRI);

// holter每分钟获取结果
int EcgAlgGetHolterOneMinResult(ECG_FINAL_RESULT *final_result, bool reset);

// holter24小时最终结果
int EcgAlgGetHolterFinalResult(ECG_HOLTER_RESULT *final_result, bool reset);


#endif
