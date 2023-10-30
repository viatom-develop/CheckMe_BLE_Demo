//*************************************************************************
//文件名称：EcgMainProc.cpp
//
//文件说明：Ecg算法主函数
//
//更改历史：Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************
//#include "stdafx.h"
#include "EcgMainProc.h"
#include "RespMain.h"
//=========================================================================
//1. 定义相关全局变量
//=========================================================================
ECG_DEBUG_INFO		gEcgDebugInfo;								// 调试信息输出变量
ECG_ALG_CONFIG		gEcgAlgConfig;								// 算法配置信息
ECG_ALG_RESULT		gEcgAlgResult;									// 算法实时分析结果，每秒输出
ECG_FINAL_RESULT		gEcgFinalResult;								// 算法最终分析结果，信号输入结束后最终输出一次
ECG_FINAL_RESULT		gEcgFinalFirst;									// 算法初次分析的结果，用于与重算结果比对
bool gbFirstExist;																		// 初次分析结果是否存在

ECG_HOLTER_RESULT gEcgHolterResult;							// Holter运算结果

QRS_INFO gQrsInfo;																// QRS平均宽度、斜率、RR间期等信息

short gViewDataBuff[ECG_DATA_BUFF_LEN];							// 外部滤波数据缓存区，作为ECG分析算法的输入
short gLpDataBuff[ECG_DATA_BUFF_LEN];								// 高通滤波数据缓存区，用于T波检测，ST分析等
short gHpDataBuff[ECG_DATA_BUFF_LEN];							// 高通滤波数据缓存区，用于T波检测，ST分析等
short gBpDataBuff[ECG_DATA_BUFF_LEN];								// 带通滤波数据缓存区
short gDiffDataBuff[ECG_DATA_BUFF_LEN];							// 带通差分信号缓存区
short gIntegDataBuff[ECG_DATA_BUFF_LEN];						// 平方积分数据缓存区，用于QRS波检测

unsigned int gTotalSampleBuff[ECG_DATA_BUFF_LEN];			// 总采样点数，最长支持2386小时数据
unsigned int gEcgTotalSampleNum;						// 总的Ecg采样点数
unsigned short gCurBuffPtr;										// 当前缓存区的指针位置
unsigned int gEcgHrvTimeCnt;									// 30s计时，用于HRV检测

unsigned short gEcgNoiseTimeCnt;						// 3秒计时，用于噪声检测
int gNoiseOrgSum;														// 用于噪声检测的3s原始数据之和

unsigned int	gEcgPauseTimeCnt;							// 心跳暂停计时
unsigned int	gEcgPauseNum;								// 心跳暂停计数，holter模式中每分钟更新
bool gIsEcgPause;													// 是否心跳暂停

RESP_ALG_RESULT Resp_Result_temp;  // 呼吸率相关参数

// 算法初始化函数
void EcgAlgInitialize(void)
{
	// 滤波器初始化
	InitializeEcgFilter();

	// 数据处理初始化
	EcgDataProcess(0, true);

	// 数据缓存区初始化
	EcgDataInitialize();

	// QRS搜波初始化
	QrsComplexDetect(true);

	// 计算初始化
	EcgResultInitialize();

	// QRS分类初始化
	QrsClassifyMain(true);

	// 实时分析结果初始化
	EcgAlgGetResult(0, true);

	// 心率计算初始化
	CalculateHeartRate(true);

	// ST计算初始化
	CalculateStValue(true);

	// 最终分析结果初始化
	gbFirstExist = false;
	EcgAlgGetFinalResult(0, true);

	// 这个要单独初始化一次，不在上面的EcgAlgGetResult（）里面初始化
	gEcgAlgResult.isAsystole = false;

	// 噪声检测初始化
	EcgNoiseEstimate(true);

	// 调试信息初始化
	EcgAlgGetDebugInfo(0, true);

	gEcgAlgConfig.sOrgSampleRate = 250;
	gEcgAlgConfig.cWorkMode = 1;					// 初始化为内部电极
	gEcgAlgConfig.nCeilingValue = 0;

	gQrsInfo.nAveInterval = 0;
	gQrsInfo.nAveSlope = 0;
	gQrsInfo.nAveWidth = 0;
	gQrsInfo.nAveQrsAmp = 0;

	gEcgPauseTimeCnt = 0;
	gIsEcgPause = false;
	gEcgPauseNum = 0;

	// 呼吸函数初始化
	RespMain(&Resp_Result_temp,0,1);
}

void EcgDataInitialize(void)
{
	int i = 0;

	// 初始化buffer
	for (i = 0; i < ECG_DATA_BUFF_LEN; i++)
	{
		gViewDataBuff[i] = 0;
		gLpDataBuff[i] = 0;
		gHpDataBuff[i] = 0;
		gBpDataBuff[i] = 0;
		gDiffDataBuff[i] = 0;
		gIntegDataBuff[i] = 0;
		gTotalSampleBuff[i] = 0;
	}

	// 初始化指针
	gCurBuffPtr = 0;
	gEcgTotalSampleNum = 0;
}

//Ecg算法配置
int EcgAlgSetup(ECG_ALG_CONFIG alg_config, char *ecg_alg_version)
{
	gEcgAlgConfig = alg_config;
	gEcgAlgConfig.nCeilingValue = (2 << 13) / 2;		// 计算钳位值，14bit，正负13bit

	// 判断算法设置是否正确
	if ((500 != gEcgAlgConfig.sOrgSampleRate) && (250 != gEcgAlgConfig.sOrgSampleRate))
	{
		// 纠错处理，若超出了设置范围则报错
		return 0;
	}
	else
	{
		memcpy(ecg_alg_version, ECG_ALG_VERSION, 8);

		return 1;
	}
}

// 第二次运算前对算法进行一些配置，避免二次运算的混乱
int EcgAlgRecalConfig(void)
{
	//=========================================================================
	// 算完以后，把所有的东西全部清零，准备进行回算
	//=========================================================================
	// 滤波器初始化
	InitializeEcgFilter();

	// 数据处理初始化
	EcgDataProcess(0, true);

	// 数据缓存区初始化
	EcgDataInitialize();

	// 将QRS波的个数清零
	gQrsPeakNum = 0;

	gQrsInfo.nAveInterval = 0;
	gQrsInfo.nAveSlope = 0;
	gQrsInfo.nAveWidth = 0;
	gQrsInfo.nAveQrsAmp = 0;

	// 将PVC个数置零
	gPvcTemplate.nTemplateNum = 0;
	gPvcTemplate.bIsTemplateExist = false;
	//PVC计数清零
	gPvcNumAll = 0;

	// HRV计时清零
	gEcgHrvTimeCnt = 0;
	gTotalHrvSqr = 0;
	gTotalHrvNum = 0;

	// 最后将总体结果复位，避免混乱
	EcgAlgGetFinalResult(0, true);

	// 将噪声检测部分复位
	EcgNoiseEstimate(true);

	return 1;
}

//Ecg算法的总入口函数
void EcgAlgAnalysis(int data)
{
	int temp_hrv = 0;

	// 初始化算法分析结果相关参数
	// EcgAlgGetResult(0, true);
	// 作此修改是为了保持停搏标识
	gEcgAlgResult.isQrsDetected = false;
	gEcgAlgResult.isHrvChange = false;

	// 噪声检测计时，HRV计时
	gEcgHrvTimeCnt++;
	gEcgNoiseTimeCnt++;

	// 初始化调试信息输出参数
	EcgAlgGetDebugInfo(0, true);

	//1. 进行心电数据滤波，填充各缓存区
	if (!EcgDataProcess(data, false))
	{
		// 本次滤波未填充算法数据缓存区，不执行后续计算，直接退出
		return;
	}

	// 进行QRS波检测
	if (QrsComplexDetect(false))
	{
		// 计算当前呼吸率	    
		RespMain(&Resp_Result_temp, 0, 0);
		gEcgAlgResult.RespRate = Resp_Result_temp.RespRate;
		//printf("Resp_Result_temp.RespRate = %d  \n", Resp_Result_temp.RespRate); // debug   

		// 计算当前QRS波的心率
		gEcgAlgResult.nEcgHeartRate = CalculateHeartRate(false);

		// 计算上一个QRS波的ST值（因为当前QRS波刚刚检出，T波可能尚未送入算法）
		CalculateStValue(false);

		// 计算上一个QRS波的QT值
		CalculateQTValue(false);

		// 对上一个QRS波进行分类
		QrsClassifyMain(false);

		// 计算HRV
		if (ECG_HRV_COUNT <= gEcgHrvTimeCnt)
		{
			temp_hrv = CalculateOntimeHrv(false);
			if (temp_hrv)
			{
				// 如果返回值不是0，则更新全局HRV
				gEcgAlgResult.nOntimeHrv = temp_hrv;
				gEcgAlgResult.isHrvChange = true;

				// 重设计时器，计算过一次后，只需要再有10秒，就应该算下一次
				gEcgHrvTimeCnt = ECG_SAMPLE_RATE * 20;
			}
		}
	}

	// 进行噪声检测
	if (ECG_NOISE_COUNT == gEcgNoiseTimeCnt)
	{
		// 每3秒进行一次噪声检测
		EcgNoiseEstimate(false);

		// 噪声检测时间复位
		gEcgNoiseTimeCnt = 0;
	}

	// 进行停搏检测
	if (gBpDataBuff[gCurBuffPtr] < QRS_PEAK_MIN_THRESH)
	{
		gEcgPauseTimeCnt++;

		// 超过三秒，判断为心跳暂停
		if (gEcgPauseTimeCnt > ECG_SAMPLE_RATE * 3)
		{
			if (!gIsEcgPause)
			{
				gIsEcgPause = true;
				gEcgPauseNum++;
			}
		}
		else
		{
			gIsEcgPause = false;
		}
	}
	else
	{
		// 幅度大于阈值，将暂停相关计数置零
		gEcgPauseTimeCnt = 0;
		gIsEcgPause = false;
	}

}

//Ecg数据滤波总函数
int EcgDataProcess(int data, bool reset)
{
	int flag = 0;
	static char sample_count = 0;			//采样计数器，实现1/2或1/4原始频率的采样
	static short buff_ptr = 0;						//缓存区内的数据个数，主要是为了方便gEcgBuffPtr的正常操作

	if (reset)
	{
		sample_count = 0;
		buff_ptr = 0;
		return 0;
	}

	gCurBuffPtr = buff_ptr;

	//=========================================================================
	//3. 进行降采样，将原始数据降采样为250Hz数据
	//=========================================================================
	if (0 == sample_count)
	{
		// 降采样点，进行归一化
		// 维护采样计数器
		sample_count ++;
		if (sample_count >= (gEcgAlgConfig.sOrgSampleRate / ECG_SAMPLE_RATE))
		{
			sample_count = 0;
		}

		// 将降采样和归一化后的数据存入显示数据缓存区
		gViewDataBuff[gCurBuffPtr] = data;
		gNoiseOrgSum += data;
	}
	else
	{
		// 非降采样点，不送入后续算法，直接退出
		// 维护采样计数器
		sample_count ++;
		if (sample_count >= (gEcgAlgConfig.sOrgSampleRate / ECG_SAMPLE_RATE))
		{
			sample_count = 0;
		}

		// 直接退出，不进行后续计算
		return 0;
	}

	//=========================================================================
	//3. 算法内部的带通、查分、平方积分等滤波处理
	//=========================================================================
	//data_temp = Fir5HighPass(gViewDataBuff[gCurBuffPtr], false);
	//3.1. 分析信号 高通 / 低通 滤波
	gLpDataBuff[gCurBuffPtr] = Fir20LowPass(gViewDataBuff[gCurBuffPtr], false);
	gHpDataBuff[gCurBuffPtr] = IndFir5HighPass(gViewDataBuff[gCurBuffPtr], false);

	//3.2. 分析信号带通滤波
	gBpDataBuff[gCurBuffPtr] = Fir5HighPass(gLpDataBuff[gCurBuffPtr], false);

	//3.3. 分析信号差分滤波
	gDiffDataBuff[gCurBuffPtr] = EcgDiffFilter(gBpDataBuff[gCurBuffPtr], false);

	//3.4. 分析信号求绝对值、滑动积分滤波
	gIntegDataBuff[gCurBuffPtr] = SquareIntegration(gDiffDataBuff[gCurBuffPtr], false);

	//3.5. 总采样点数自增，方便后续计算距离等
	gTotalSampleBuff[gCurBuffPtr] =  gEcgTotalSampleNum;

	//3.6. 维护指针队列，采样点总数自增
	if (buff_ptr < ECG_DATA_BUFF_LEN - 1)
	{
		buff_ptr++;
	}
	else
	{
		buff_ptr = 0;
	}
	gEcgTotalSampleNum ++;

	gEcgDebugInfo.bIsDebugInfo = true;
	return 1;
}


//	获取算法分析结果
int EcgAlgGetResult(ECG_ALG_RESULT *alg_result, bool reset)
{
	if (reset)
	{
		gEcgAlgResult.isQrsDetected = false;
		gEcgAlgResult.isHrvChange = false;
		gEcgAlgResult.bIsSolid = false;
		gEcgAlgResult.nEcgHeartRate = 0;
		gEcgAlgResult.nQrsAmp = 0;
		gEcgAlgResult.nQrsHpAmp = 0;
		gEcgAlgResult.nQrsInterval = 0;
		gEcgAlgResult.nQrsWidth = 0;
		gEcgAlgResult.nQrsPeakPos = 0;
		gEcgAlgResult.nOntimeHrv = 0;

		gEcgAlgResult.RespRate = 0;
		return 0;
	}

	*alg_result = gEcgAlgResult;
	return 1;
}


//获取算法调试信息
int EcgAlgGetDebugInfo(ECG_DEBUG_INFO *debug_info, bool reset)
{
	if (reset)
	{
		gEcgDebugInfo.nViewData = 0;
		gEcgDebugInfo.nLowPassData = 0;
		gEcgDebugInfo.nHighPassData = 0;
		gEcgDebugInfo.nBandData = 0;
		gEcgDebugInfo.nDiffData = 0;
		gEcgDebugInfo.nIntegData = 0;

		gEcgDebugInfo.bIntegPeak = false;
		gEcgDebugInfo.bIsDebugInfo = false;

		gEcgDebugInfo.BpDiffPeak.nDiffStartPos = 0;
		gEcgDebugInfo.BpDiffPeak.nDiffEndPos = 0;
		gEcgDebugInfo.BpDiffPeak.nDiffZeroPos = 0;
		gEcgDebugInfo.BpDiffPeak.nDiffMaxPos = 0;
		gEcgDebugInfo.BpDiffPeak.nDiffMinPos = 0;

		gEcgDebugInfo.BandPeak.nEndPos = 0;
		gEcgDebugInfo.BandPeak.nPeakPos = 0;
		gEcgDebugInfo.BandPeak.nStartPos = 0;

		gEcgDebugInfo.HighPeak.nEndPos = 0;
		gEcgDebugInfo.HighPeak.nPeakPos = 0;
		gEcgDebugInfo.HighPeak.nStartPos = 0;

		gEcgDebugInfo.ViewPeak.nEndPos = 0;
		gEcgDebugInfo.ViewPeak.nPeakPos = 0;
		gEcgDebugInfo.ViewPeak.nStartPos = 0;

		gEcgDebugInfo.IntegPeak.nPeakHight = 0;
		gEcgDebugInfo.IntegPeak.nPeakPos = 0;
		gEcgDebugInfo.IntegPeak.nStartHight = 0;
		gEcgDebugInfo.IntegPeak.nStartPos = 0;

		return 0;
	}

	// 如果调试信息存在，则返回调试信息
	if (gEcgDebugInfo.bIsDebugInfo)
	{
		gEcgDebugInfo.nViewData = gViewDataBuff[gCurBuffPtr];
		gEcgDebugInfo.nLowPassData = gLpDataBuff[gCurBuffPtr];
		gEcgDebugInfo.nHighPassData = gHpDataBuff[gCurBuffPtr];
		gEcgDebugInfo.nBandData = gBpDataBuff[gCurBuffPtr];
		gEcgDebugInfo.nDiffData = gDiffDataBuff[gCurBuffPtr];
		gEcgDebugInfo.nIntegData = gIntegDataBuff[gCurBuffPtr];

		*debug_info = gEcgDebugInfo;

		return 1;
	}
	else
	{
		return 0;
	}

}


// 获取最终的全部QRS波
int EcgAlgGetDebugQrs(QRS_PEAK *qrs_peak_buff, ECG_NOISETYPE *ecg_noise_buff, int *noise_buff_num, bool reset)
{
	int i = 0;

	if (reset)
	{
	}

	// 噪声检测缓存区赋值
	for (i = 0; i < gEcgNoiseBuffNum; i++)
	{
		ecg_noise_buff[i] = gEcgNoiseBuff[i];
	}
	*noise_buff_num = gEcgNoiseBuffNum;

	// QRS缓存区赋值
	for (i = 0; i < gQrsPeakNum; i++)
	{
		qrs_peak_buff[i] = gQrsPeakBuff[i];
	}

	return gQrsPeakNum;
}


//用于在第10s的时候检测是否判断为噪声大于2，需要重启的标志
bool EcgGetNeedResetFlag(void)
{
	int count = 0;
	for(int i = 0; i < 3; i++){
		count += gEcgNoiseBuff[i].nNoiseType;
	}
	if(count >= 3){
		return true;
	}else{
		return false;
	}
}

//=========================================================================
// 定义求模、进位、排序等通用函数
//=========================================================================
//求模运算
int mod(int i, int j)
{
	while(i < 0)
	{
		i += j;
	}

	while(i >= j)
	{
		i -= j;
	}

	return i;
}

//************************************************************************
// 函数名称: inline int LoopInc(int data, int length)
//
// 函数说明: 循环缓存区内的指针进位操作
//
// 输入变量: int data:	进位前的指针位置
//			int length:	循环缓存区的长度
//
// 输出变量: int result: 进位后的指针位置
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int LoopInc(int data, int inc, int length)
{
	if ((data + inc) < length)
	{
		return(data + inc);
	}
	else
	{
		return(data + inc - length);
	}
}

// 循环缓存区内的指针退位操作
int LoopDec(int data, int inc, int length)
{
	if ((data - inc) >= 0)
	{
		return(data - inc);
	}
	else
	{
		return(length + data - inc);
	}
}

// 将队列排序，并查找指定位置的值
int SortAndSearch(int org[], int len, int mid)
{
	int left = 0, right = 0, mid_value = 0, temp = 0, i = 0, j = 0;

	int value[50];		//暂定长度为10，后续如果需要可以改大一些

	//为了不改变原来序列的排列顺序，将输入值赋值到内部的局部变量
	for (i = 0; i < len; i++)
	{
		value[i] = org[i];
	}

	// 冒泡排序
	for (j = 0; j <= len - 1; j++)
	{
		for(i = 0; i < len - 1 - j; i++)
		{
			if(value[i] > value[i+1])
			{
				temp = value[i];
				value[i] = value[i + 1];
				value[i + 1] = temp;
			}
		}
	}

	// 返回待查位置的值
	return value[mid - 1];
}


//************************************************************************
// 函数名称: inline int RightShift(int data, char bit)
//
// 函数说明: 内联函数，进行右移操作，正负数分别处理，右移bit位
//
// 输入变量: int data:	输入数据
//			char bit:	右移的位数	
//
// 输出变量: int result: 右移后的输出数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int RightShift(int data, char bit)
{
	if (0 <= data)
	{
		return( data >> bit);
	}
	else
	{
		return(-(-(data) >> bit));
	}
}


//************************************************************************
// 函数名称: inline int GetLowBit12(int data)
//
// 函数说明: 内联函数，获取数据的低12位
//
// 输入变量: int data:   输入数据
//
// 输出变量: int result: 低12位输出数据
//
// 历    史: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int GetLowBit12(int data)
{
	if (0 <= data)
	{
		return( data & 0x00000FFF);
	}
	else
	{
		return(-(-(data) & 0x00000FFF));
	}
}

// 取绝对值
int Abs(int data)
{
	return data >= 0 ? data : (-data);
}