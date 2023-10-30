//*************************************************************************
//文件名称：EcgInterface.h
//
//文件说明：定义算法借口，包含函数和自定义变量类型
//
//更改历史：Created by Chenmaomao [8 / 20 / 2013]
//*************************************************************************

#ifndef _ECGINTERFACE_H_
#define _ECGINTERFACE_H_

#include "EcgMainProc.h"
#include "EcgResult.h"


//=========================================================================
// 2、输入接口函数
//=========================================================================

// 算法配置函数
extern int EcgAlgSetup(ECG_ALG_CONFIG alg_config, char *ecg_alg_version);

// 算法包总体初始化函数
extern void EcgAlgInitialize(void);

// 算法包总入口函数
extern void EcgAlgAnalysis(int data);


//=========================================================================
//3. 输出接口函数
//=========================================================================
//	输出算法分析结果
extern int EcgAlgGetResult(ECG_ALG_RESULT alg_result, bool reset);

// 获取算法最终分析结果
extern int EcgAlgGetFinalResult(ECG_FINAL_RESULT *final_result, bool reset);

// 获取全部的QRS波峰位置（对外接口，提供给PWTT）
extern int EcgAlgGetQrsPeakPos(int *qrs_pos_buff);


//=========================================================================
//3. 调试信息接口函数
//=========================================================================
// 输出算法内部的实时调试信息
extern int EcgAlgGetDebugInfo(ECG_DEBUG_INFO *debug_info, bool reset);

// 获取最终的全部QRS波
extern int EcgAlgGetDebugQrs(QRS_PEAK *qrs_peak_buff, ECG_NOISETYPE *ecg_noise_buff, int *noise_buff_num, bool reset);






#endif