//*************************************************************************
//文件名称：
//
//文件说明：
//
//更改历史：Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************

#ifndef _ECGMAINPROC_H_
#define _ECGMAINPROC_H_

#include "EcgDataType.h"
#include "EcgFilter.h"
#include "EcgQrsDetect.h"
#include "EcgResult.h"
#include "EcgQrsClassify.h"
#include "AF.h"

#include <string.h>

//=========================================================================
//1. 定义常量
//=========================================================================
#define ECG_ALG_VERSION				"01.19.27"									// 定义算法包版本

#define ECG_SAMPLE_RATE				250												// 分析算法采用的采样率，250Hz
#define ECG_DATA_BUFF_LEN			ECG_SAMPLE_RATE * 3			// 定义数据缓存区的长度
#define ECG_HRV_COUNT					ECG_SAMPLE_RATE * 25			// 每30s进行一次HRV计算
#define ECG_NOISE_COUNT				ECG_SAMPLE_RATE * 3			// 每3s进行一次Noise检测
#define ECG_HR_MAX_THRESH		251												// 心率最大阈值，250BPM
#define ECG_HR_MIN_THRESH			29												// 心率最小阈值，30BPM

//=========================================================================
//2. 变量声明
//=========================================================================

extern short gViewDataBuff[ECG_DATA_BUFF_LEN];						// 外部滤波数据缓存区，作为ECG分析算法的输入
extern short gLpDataBuff[ECG_DATA_BUFF_LEN];							// 高通滤波数据缓存区，用于T波检测，ST分析等
extern short gHpDataBuff[ECG_DATA_BUFF_LEN];							// 高通滤波数据缓存区，用于T波检测，ST分析等
extern short gBpDataBuff[ECG_DATA_BUFF_LEN];							// 带通滤波数据缓存区
extern short gDiffDataBuff[ECG_DATA_BUFF_LEN];							// 带通差分信号缓存区
extern short gIntegDataBuff[ECG_DATA_BUFF_LEN];						// 平方积分数据缓存区，用于QRS波检测

extern unsigned int gTotalSampleBuff[ECG_DATA_BUFF_LEN];		// 总采样点数，最长支持4min数据
extern unsigned int gEcgTotalSampleNum;										// 总的Ecg采样点数
extern unsigned short gCurBuffPtr;														// 当前缓存区的指针位置
extern unsigned int gEcgHrvTimeCnt;												// 30s计时，用于HRV检测
extern unsigned short gEcgNoiseTimeCnt;										// 3秒计时，用于噪声检测
extern int gNoiseOrgSum;																		// 用于噪声检测的3s原始数据之和

extern unsigned int	gEcgPauseTimeCnt;												// 心跳暂停计时
extern unsigned int	gEcgPauseNum;													// 心跳暂停计数，holter模式中每分钟更新
extern bool				gIsEcgPause;															// 是否心跳暂停


extern ECG_ALG_RESULT	gEcgAlgResult;											// 最终输出结果
extern ECG_ALG_CONFIG gEcgAlgConfig;											// 算法配置信息
extern ECG_DEBUG_INFO gEcgDebugInfo;											// 调试信息输出变量
extern ECG_FINAL_RESULT gEcgFinalResult;										// 算法最终分析结果，信号输入结束后最终输出一次
extern ECG_FINAL_RESULT gEcgFinalFirst;											// 算法初次分析的结果，用于与重算结果比对
extern bool gbFirstExist;																		// 初次分析结果是否存在

extern ECG_HOLTER_RESULT gEcgHolterResult;								// Holter运算结果

extern QRS_INFO gQrsInfo;																	// QRS平均宽度、斜率、RR间期等信息


//=========================================================================
//3. 函数声明
//=========================================================================

// 算法配置信息设置函数（对外接口）
int EcgAlgSetup(ECG_ALG_CONFIG alg_config, char *ecg_alg_version);

// 算法初始化函数（对外接口）
void EcgAlgInitialize(void);

// 算法分析总入口函数（对外接口）
void EcgAlgAnalysis(int data);

// 获取算法内部的调试信息（对外接口）
int EcgAlgGetDebugInfo(ECG_DEBUG_INFO *debug_info, bool reset);

// 获取最终的全部QRS波（对外接口）
int EcgAlgGetDebugQrs(QRS_PEAK *qrs_peak_buff, ECG_NOISETYPE *ecg_noise_buff, int *noise_buff_num, bool reset);

// 获取算法的分析结果（对外接口）
int EcgAlgGetResult(ECG_ALG_RESULT *alg_result, bool reset);

// Ecg数据缓存区初始化函数
void EcgDataInitialize(void);

// Ecg数据滤波总入口函数
int EcgDataProcess(int data, bool reset);

// 第二次运算前对算法进行一些配置，避免二次运算的混乱
int EcgAlgRecalConfig(void);

//用于在第10s的时候检测是否判断为噪声大于2，需要重启的标志
bool EcgGetNeedResetFlag(void);
//=========================================================================
//4. 通用函数声明
//=========================================================================
// 求模运算
int mod(int i, int j);

// 循环缓存区内的指针进位操作
int LoopInc(int data, int inc, int length);

// 循环缓存区内的指针退位操作
int LoopDec(int data, int inc, int length);

// 将队列排序，并查找指定位置的值
int SortAndSearch(int org[], int len, int mid);

// 进行右移操作，右移指定的bit位
int RightShift(int data, char bit);

// 取得数据的低12位
int GetLowBit12(int data);

// 取绝对值
int Abs(int data);


#endif

