 //#include <stdbool.h>
#include <string>

//=========================================================================
// 定义QRS搜波相关结构体
//=========================================================================
//积分峰结构体
typedef struct Integ_Peak
{
	unsigned int nStartHight;				//积分峰起点幅度
	unsigned int nStartPos;				//积分峰起点位置
	unsigned int nPeakHight;				//积分峰波峰幅度
	unsigned int nPeakPos;				//积分峰波峰位置
}INTEG_PEAK;

// 带通差分峰结构体
typedef struct Bp_Diff_Peak
{
	unsigned int nDiffStartPos;			//差分峰起始位置，理论上对应于带通信号的起点位置
	unsigned int nDiffEndPos;			//差分峰终点位置，理论上对应于带通信号的终点位置
	unsigned int nDiffZeroPos;			//差分峰过零点位置，理论上对应于带通信号的峰值点位置
	unsigned int nDiffMaxPos;			//差分峰最大值位置
	unsigned int nDiffMinPos;			//差分峰最小值位置
}BP_DIFF_PEAK;

// 显示信号峰结构提
typedef struct Qrs_Peak_Pos
{
	unsigned int nStartPos;				//QRS起点位置
	unsigned int nEndPos;					//QRS终点位置
	unsigned int nPeakPos;				//QRS波峰位置
}QRS_PEAK_POS;

typedef struct Qrs_Peak
{
	// 位置信息
	unsigned short nStartPos;			// 数据缓存区中的起点位置
	unsigned short nPeakPos;			// 数据缓存区中的峰值点位置
	unsigned short nEndPos;			// 数据缓存区中的终点位置
	unsigned short nIsoPos;				// 数据缓存区中的ISO点位置
	unsigned short nStPos;				// 数据缓存区中的ST点位置
	unsigned short nTpeakPos;		// T波位置
	unsigned short nTendPos;			// T波结束的位置，计算QT所用
	unsigned short nPpeakPos;		// P波位置
	unsigned short nBpPeakPos;		// 带通信号的波峰位置，主要是为了后面模板匹配而准备

	unsigned int nPeakPosAll;			// 总采样点中的峰值点

	bool isPeakBefore;						// 差分峰类型，判断该峰的类型，0：波峰在后、波谷在前，1：波峰在前，波谷在后
	bool isPositive;								// 该波类型，0：负向波；1：正向波
	bool isSolid;									// 该QRS波是否可信（根据平均RR间期判断）
	bool isNoisePeak;						// 是否范围内的QRS波

	// QRS波测量结果
	int nQrsAmp;								// QRS波幅度
	int nQrsHpAmp;
	int nStValue;									// St值
	int nQtValue;									// QT值
	unsigned short nQrsWidth;		// QRS波宽度
	unsigned short nRRInterval;		// RR间期

	// QRS波分类信息
	unsigned short nQrsType;			// 该QRS的类型，0=初始，1=正常，2=PVC，3=未分类
	short nCoefNormal;						// 该QRS波与正常模板的相关系数

}QRS_PEAK;

typedef struct Qrs_Info
{
	unsigned short nAveWidth;
	unsigned short nAveSlope;
	unsigned short nAveInterval;
	int nAveQrsAmp;
}QRS_INFO;

typedef struct Ecg_NoiseType
{
	unsigned int nNoiseStartPosAll;				// 本次检测的起始位置
	unsigned short nNoiseType;						// 本段数据的噪声类型，0-无噪声，1-有噪声
	unsigned int nNoiseStd;								// 用于判断噪声的标准差
	unsigned short nPeakRatio;						// 差分峰是否超过QRS峰个数的1.5倍
	unsigned int	nNoiseThr;								// 判读噪声的阈值
}ECG_NOISETYPE;



//=========================================================================
//	定义全局相关结构体
//=========================================================================
// 算法配置结构体
typedef struct Ecg_Alg_Config
{
	unsigned short	sOrgSampleRate;					// Ecg数据原始采样频率，算法内部均采用250Hz。可设置为250 / 500 / 1000
	unsigned char	cWorkMode;						// 设备的工作模式，1：内部电极；2：外部电缆
	int					nCeilingValue;						// 钳位值，防止越界，根据AD的采样位数计算得出
	bool					bJapanVersion;					// 日本版本，true为日本版本，PVC和Irreg灵敏度较高，false为普通版本
}ECG_ALG_CONFIG;

// 算法相关运算结果结构体
typedef struct Ecg_Alg_Result
{
	bool isQrsDetected;				// 是否有QRS波检出，有检出则更新心率、宽度、幅度等信息
	bool isHrvChange;					// 实时rMSSD是否改变，这个主要是用来便于输出
	bool isAsystole;						// 判断是否停搏，连续5秒无脉搏波判断为停搏
	bool bIsSolid;							// 该QRS波是否可信
	int nEcgHeartRate;					// 心率，每检测到一个QRS波更新一次
	int nQrsWidth;							// QRS波宽度
	int nQrsAmp;							// QRS波幅度
	int nQrsHpAmp;						// 带通信号的QRS波幅度
	int nQrsInterval;						// QRS波RR间期
	int nQrsPeakPos;						// QRS波的波峰位置
	int nOntimeHrv;						// 30s实时HRV值，单位为ms，放大100倍

	QRS_PEAK EcgOntimeQrs;		// 实时搜索到的QRS波的相关信息

	short RespRate;					//呼吸率
}ECG_ALG_RESULT;


// HRV（暂时不需要这么多HRV数据）
// typedef struct Hrv_Para
// {
// 	int nHrvMean;					// RR平均值
// 	int nHrvSdnn;
// 	int nHrvRmssd;					// 差值均方平方根，相邻RR计算，反映快变成分
// 	int nHrvPnn50;
// }HRV_PARA;

// 算法最终的分析结果
typedef struct Ecg_Final_Result
{
	unsigned short nEcgHeartRate;			// 心率，单位为BPM
	unsigned short nEcgQrsWidth;				// QRS波平均宽度，单位为ms
	unsigned short nEcgQrsAmp;				// QRS波平均幅度，单位为LSB
	unsigned short nPvcNum;						// PVC的个数
	unsigned short nIrregCoef;					// 不规则节律系数
	unsigned char SignalNoise;					// 信号是否存在噪声 0-无噪声；1-有噪声;2-噪声大

	unsigned short nEcgQtValue;				// QT值，单位为ms
	unsigned short nEcgQTc;						// QTc，根据HR校正的QT
	unsigned short nPausePerMin;
	unsigned char nQTcStatus;					// QTc状态，0：Short QTc；1：Normal QTc；2：Prolonged QTc

	float nEcgStValue;				// ST值，单位为uV
	int nEcgFinalHrv;					// 最终的HRV结果

	bool Bradycardia;					// 心动过缓，小于60
	bool Tachycardia;				// 心动过速，大于100
	bool Irregular;						// 不规则节律
	bool WideQrs;						// Qrs波过宽
	bool HighStValue;				// St偏高
	bool LowStValue;					// ST偏低
	bool LowQrsAmp;				// QRS波幅度过低
	bool PvcBeat;						// 存在室性早搏
	bool PauseBeat;
	
	//HRV_PARA HrvResult;			// 不需要这么多HRV结果，一个就够了

}ECG_FINAL_RESULT;


typedef struct Ecg_Holter_Result
{
	unsigned short nAveHeartRate;
	unsigned int nSumHeartRate;
	unsigned short nAveRRinterval;				// 单位为采样点
	unsigned short nMinHeartRate;
	unsigned short nMaxHeartRate;
	unsigned int nHolterSdnn;
	unsigned int nHolterRmssd;
	unsigned int nHolterPnn50;
	unsigned int nTotalRhythmNum;
	unsigned int nRRintervalNum;
	unsigned int nPnn50Num;
	unsigned int nPauseNum;
}ECG_HOLTER_RESULT;


// 算法内部调试信息结构体
typedef struct  Ecg_Debug_Info
{
	bool bIsDebugInfo;						//调试信息是否存在，若为非降采样点，则不返回调试信息
	bool bIntegPeak;							//是否存在积分峰。若积分峰存在则下面的调试信息存在

	int nViewData;
	int nLowPassData;
	int nHighPassData;
	int nBandData;
	int nDiffData;
	int nIntegData;
	int nEcgQrsAmp;
	unsigned short nEcgHeartRate;
	int nOntimeHrv;
	
	INTEG_PEAK IntegPeak;
	BP_DIFF_PEAK BpDiffPeak;				//带通差分信号上的搜波结果，输出便于画图
	QRS_PEAK_POS BandPeak;				//带通信号上的搜波结果
	QRS_PEAK_POS HighPeak;				//带通信号上的搜波结果
	QRS_PEAK_POS ViewPeak;				//显示信号上的搜波结果

}ECG_DEBUG_INFO;
