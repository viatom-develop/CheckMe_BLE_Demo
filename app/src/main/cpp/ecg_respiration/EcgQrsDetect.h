//*************************************************************************
//文件名称：EcgMainProc.cpp
//
//文件说明：Ecg算法主函数
//
//更改历史：Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************

#ifndef _ECGQRSDETECTION_H_
#define _ECGQRSDETECTION_H_

#include "EcgMainProc.h"

//=========================================================================
//定义结构体、常量
//=========================================================================

#define INTEG_PEAK_MAX_WIDTH		500 * ECG_SAMPLE_RATE / 1000		// 积分峰宽度上限阈值，500ms
#define INTEG_PEAK_MIN_WIDTH		20   * ECG_SAMPLE_RATE / 1000		// 积分峰宽度下限阈值，20ms
#define INTEG_PEAK_BUFF_LEN			5																// 积分峰缓存区长度
#define INTEG_PEAK_MIN_THRESH		50															// 积分峰最小检测阈值

#define IRREG_THR_NRM						15															// 归一化 r-MSSD 的不规则节律参数，普通版本，灵敏度较低
#define IRREG_THR_JAP							10															// 归一化 r-MSSD 的不规则节律参数，日本版本，灵敏度较高

#define SEARCH_WINDOW					180 * ECG_SAMPLE_RATE / 1000		// 正常QRS波宽度为120ms，因此定义搜波宽度为180ms，否则可能搜不到起点
#define QRS_PEAK_BUFF_LEN				120															// Qrs波的缓存区，因为要支持90s的测量，所以开辟120个缓存区

#define ECG_NOISE_BUFF_LEN				30															// 噪声质量缓存区的数量，3秒检测一次，30秒一般只有10个缓存，一分钟20个缓存，留一点余地设定为30
#define ECG_NOISE_STD_THR				200															// 三秒原始信号的标准差

//=========================================================================
//  跟幅度相关的一些常量。LSB与实际幅度的换算关系是 1.25uV = 1 LSB
//=========================================================================
//#define uV_PER_LSB			1.25
#define uV_PER_LSB				2.467
#define QRS_PEAK_MIN_THRESH		0.2 * 1000 / uV_PER_LSB							// 定义QRS幅度的最小阈值，最小阈值为0.2mV / 1.25uV per LSB = 200LSB
																															// 此处原本是0.25mV = 200LSB，但是因为滤波之后幅度有所衰减
																															// 导致0.25mV脉搏波无法检出, 因此更改为0.2mV = 160LSB
#define QRS_PEAK_MAX_THRESH		10 * 1000 / uV_PER_LSB							// 定义QRS幅度的最大阈值，最大阈值为10 mV / 1.25uV per LSB = 8000LSB
#define QRS_LOW_AMP_THRESH		0.375 * 1000 / uV_PER_LSB						// 若平均QRS幅度小于该阈值，则报幅度过低，300LSB = 0.375mV
#define QRS_LOW_AMP_NO_PVC		0.4 * 1000 / uV_PER_LSB							// 若平均QRS幅度小于该阈值，则将PVC清零，320LSB = 0.25mV
#define QRS_LOW_AMP_NO_IRREG	0.4 * 1000 / uV_PER_LSB							// 若平均QRS幅度小于该阈值，则将Irreg报警清零，320LSB = 0.4mV

#define ST_HORIZON_THRESH		0.01 * 1000 / uV_PER_LSB						// ST点搜索时，判断水平线的阈值，8LSB = 0.01mV
#define T_PEAK_THRESH			0.05 * 1000 / uV_PER_LSB						// T波幅度阈值，40LSB = 0.05mV

#define SHORT_QTC_THRESH		350																// 正常QTc下限
#define LONG_QTC_THRESH			470																// 临界QTc上限


//=========================================================================
//定义变量
//=========================================================================
extern QRS_PEAK gQrsPeakBuff[QRS_PEAK_BUFF_LEN];						//QRS波缓存区，用于实时计算，只需要较少的QRS波即可
extern unsigned short gQrsPeakNum;													//QRS缓存区内波的个数

extern ECG_NOISETYPE gEcgNoiseBuff[ECG_NOISE_BUFF_LEN];		//ECG信号质量缓存区，每3秒检测一次，30秒约有10个缓存。0-非常好， 1-有噪声，2-非常差
extern unsigned short gEcgNoiseBuffNum;											//ECG信号质量缓存区数量

extern float gTotalHrvSqr;																			// 所计算的所有HRV的平方和，用于最终的HRV计算
extern int gTotalHrvNum;																			// 所计算的所有HRV的QRS个数，用于最终的HRV计算

//=========================================================================
//函数声明
//=========================================================================

// QRS复合波搜波总入口函数
int QrsComplexDetect(bool reset);

// 积分峰搜波阈值初始化函数
int IntegThreshInitalize(bool reset);

// 更新积分峰搜波阈值
void IntegThreshUpdate(void);

// 积分峰搜波函数
int IntegPeakDetection(int cur_data, int cur_pos, INTEG_PEAK *result, bool reset);

// 判断是否有效的QRS波
int QrsComplexJudge(void);

// 在带通差分信号上进行QRS波位置初步定位
int BpDiffDataPeakDetect(BP_DIFF_PEAK *diff_peak);

// 在显示信号上进行QRS波位置检测
int BandPeakDetect(BP_DIFF_PEAK *diff_peak, QRS_PEAK_POS *band_peak);

// 在显示信号上进行QRS波位置检测
int ViewPeakDetect(QRS_PEAK_POS *band_peak, QRS_PEAK_POS *view_peak);

// 根据形态等信息，排除误检的QRS波
int QrsComplexExclusion(QRS_PEAK_POS *view_peak, QRS_PEAK_POS *band_peak);

// 维护QRS波队列，更新相关QRS波参数
void QrsComplexUpdate(void);

// 噪声检测函数
int EcgNoiseEstimate(bool reset);

// 计算3秒钟的标准差
int EcgNoiseCalStd(void);

// 噪声峰检测函数
int EcgNoisePeakDetect(void);

// 计算全局HRV值，30s一次
float CalculateOntimeHrv(bool reset);




#endif
