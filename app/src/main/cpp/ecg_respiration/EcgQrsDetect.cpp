//*************************************************************************
//文件名称：EcgMainProc.cpp
//
//文件说明：Ecg算法主函数
//
//更改历史：Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************
//include "stdafx.h"
#include "EcgQrsDetect.h"
#include <math.h>

//=========================================================================
//定义QRS相关的全局
//=========================================================================
QRS_PEAK gQrsPeakBuff[QRS_PEAK_BUFF_LEN];					//QRS波缓存区
unsigned short gQrsPeakNum;													//QRS缓存区内波的个数

ECG_NOISETYPE gEcgNoiseBuff[ECG_NOISE_BUFF_LEN];		//ECG信号质量缓存区，每3秒检测一次，30秒约有10个缓存。0-非常好， 1-有噪声，2-非常差
unsigned short gEcgNoiseBuffNum;										//ECG信号质量缓存区数量

float gTotalHrvSqr;																		// 所计算的所有HRV的平方和，用于最终的HRV计算
int gTotalHrvNum;																		// 所计算的所有HRV的QRS个数，用于最终的HRV计算

//=========================================================================
//定义本文件内部使用的局部变量
//=========================================================================
INTEG_PEAK wIntegPeakBuff[INTEG_PEAK_BUFF_LEN];		// 积分波峰的缓存区
int wIntegPeakNum;
int wIntegPeakAmp[INTEG_PEAK_BUFF_LEN];						// 保存有效积分峰的幅度，目的是方便后续查找中值
int wPeakAmpNum;
int wIntegNoiseAmp[INTEG_PEAK_BUFF_LEN];						// 保存噪声峰的幅度，目的是方便后续查找中值
int wNoiseAmpNum;
int wIntegAverageAmp;

int wIntegSigThresh;																	// 积分信号峰的幅度阈值	
int wIntegNoiseThresh;																// 积分噪声峰的幅度阈值
int wTimeNoQrs;																			// 未检测到QRS波的时间
QRS_PEAK wCurQrsPeak;															// 当前搜索到的QRS波的相关信息

// 噪声检测相关
int wAveDiffPeak;																			// 3秒内QRS波相对应的差分峰的平均幅度，用于噪声检测
int wAveDiffPeakNum;																// 3秒内QRS对应的差分峰的个数
int wDiffNoisePeakBuff[ECG_NOISE_BUFF_LEN]	;					// 3秒内所检测到的查分峰缓存区
int wDiffNoisePeakNum;																// 3秒内所监测到的查分峰个数


//QRS复合波检测总入口函数
int QrsComplexDetect(bool reset)
{
	INTEG_PEAK peak_result;
	int mid_value = 0, temp = 0, integ_thresh = 0, i = 0, delay = 0, temp_amp = 0;
	float temp_hrv = 0;

	static int time_integ_init, integ_peak_num;
	static bool flag_thresh_init, flag_find_integ;
	
	if (reset)
	{
		// wCurQrsPeak初始化
		wCurQrsPeak.isPeakBefore = false;
		wCurQrsPeak.isPositive = true;
		wCurQrsPeak.isSolid = true;
		wCurQrsPeak.isNoisePeak = false;

		wCurQrsPeak.nQrsWidth = 0;
		wCurQrsPeak.nRRInterval = 0;
		wCurQrsPeak.nQrsType = 0;
		wCurQrsPeak.nQrsAmp = 0;
		wCurQrsPeak.nQrsHpAmp = 0;
		wCurQrsPeak.nStValue = 9999;

		wCurQrsPeak.nEndPos = 0;
		wCurQrsPeak.nPeakPos = 0;
		wCurQrsPeak.nPeakPosAll = 0;
		wCurQrsPeak.nStartPos = 0;

		wCurQrsPeak.nIsoPos = 0;
		wCurQrsPeak.nStPos = 0;

		wCurQrsPeak.nTpeakPos = 0;
		wCurQrsPeak.nPpeakPos = 0;
		wCurQrsPeak.nBpPeakPos = 0;

		// QRS波缓存区初始化
		for (i = 0; i < QRS_PEAK_BUFF_LEN; i++)
		{
			gQrsPeakBuff[i] = wCurQrsPeak;
		}
		gQrsPeakNum = 0;

		// 积分峰初始化
		for (i = 0; i < INTEG_PEAK_BUFF_LEN; i++)
		{
			wIntegPeakBuff[i].nPeakHight = 0;
			wIntegPeakBuff[i].nPeakPos = 0;
			wIntegPeakBuff[i].nStartHight = 0;
			wIntegPeakBuff[i].nStartPos = 0;

			wIntegPeakAmp[i] = 0;
			wIntegNoiseAmp[i] = 0;
		}
		wIntegPeakNum = 0;
		wPeakAmpNum = 0;
		wNoiseAmpNum = 0;
		wIntegAverageAmp = 0;

		// 其他参数初始化
		wIntegSigThresh = 0;
		wIntegNoiseThresh = 0;

		time_integ_init = 0;
		integ_peak_num = 0;
		flag_thresh_init = false;
		flag_find_integ = false;

		wTimeNoQrs = 0;

		IntegThreshInitalize(true);
		IntegPeakDetection(0, 0, &peak_result, true);

		// 实时HRV初始化
		CalculateOntimeHrv(true);
		
		return 0;
	}

	//=========================================================================
	//1. 积分峰搜波峰值初始化
	//=========================================================================
	if (!flag_thresh_init)
	{
		if (IntegThreshInitalize(false))
		{
			flag_thresh_init = true;
		}
		
		// 积分峰搜波阈值尚未初始化，直接退出
		return 0;
	}

	//=========================================================================
	//2. 在积分信号中进行峰值检测
	//=========================================================================
	if (IntegPeakDetection(gIntegDataBuff[gCurBuffPtr], gCurBuffPtr, &peak_result, false))
	{
		//1. 根据未检出时间，更新搜波阈值
		temp = wTimeNoQrs * 100 / ECG_SAMPLE_RATE;

		// 设置阶梯阈值
		if (temp <= 100)
		{
			// 1秒内，阈值保持不变
			integ_thresh = wIntegSigThresh;
		}
		else if (temp <= 250)
		{
			// 1秒 - 2.5秒之间，阈值按时间比例下降
			integ_thresh = wIntegSigThresh * 100 / temp;
		}
		else
		{
			// 若超过2.5秒未检测到新的脉搏波，则将阈值设置为最小积分峰阈值
			integ_thresh = INTEG_PEAK_MIN_THRESH;
		}

		//2. 判断是否超过搜波阈值
		if ((peak_result.nPeakHight - peak_result.nStartHight) > integ_thresh)
		{
			// 至此，说明搜到了合适的积分峰，输出积分峰调试信息
			gEcgDebugInfo.IntegPeak.nPeakPos = gTotalSampleBuff[peak_result.nPeakPos];
			gEcgDebugInfo.IntegPeak.nStartPos = gTotalSampleBuff[peak_result.nStartPos];
			gEcgDebugInfo.bIntegPeak = true;

			// 积分峰幅度大于阈值，填充积分峰缓存区
			if (wIntegPeakNum < INTEG_PEAK_BUFF_LEN)
			{
				//缓存区尚未填满，直接填充
				wIntegPeakBuff[wIntegPeakNum] = peak_result;
				wIntegPeakNum++;
			}
			else
			{
				//缓存区已填满，则逐个移位，将最新的积分峰填充到队尾
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegPeakBuff[i] = wIntegPeakBuff[i + 1];
				}
				wIntegPeakBuff[INTEG_PEAK_BUFF_LEN - 1] = peak_result;
			}

			//2. 进一步判断是否为有效的积分峰
			if (QrsComplexJudge())
			{
				// 至此，说明已经搜索到有效的QRS波
				// 维护QRS波队列，更新相关QRS波参数
				QrsComplexUpdate();
				
				// 更新测量结果参数
				gEcgAlgResult.isQrsDetected = true;
				gEcgAlgResult.nQrsAmp = wCurQrsPeak.nQrsAmp;
				gEcgAlgResult.nQrsHpAmp = wCurQrsPeak.nQrsHpAmp;
				gEcgAlgResult.nQrsInterval = wCurQrsPeak.nRRInterval;
				gEcgAlgResult.nQrsWidth = wCurQrsPeak.nQrsWidth;
				gEcgAlgResult.bIsSolid = wCurQrsPeak.isSolid;
				
				// 计算QRS波波峰的准确位置，提供给PWTT进行实时运算（与显示模式无关）
				delay = 0;
				gEcgAlgResult.nQrsPeakPos = wCurQrsPeak.nPeakPosAll * 4 - delay * 2;
				
				// 无QRS波计时清零
				wTimeNoQrs = 0;

				// 将停搏标识置位
				gEcgAlgResult.isAsystole = false;

				// 填充有效积分峰序列
				if (wPeakAmpNum < INTEG_PEAK_BUFF_LEN)
				{
					//缓存区尚未填满，直接填充
					wIntegPeakAmp[wPeakAmpNum] = peak_result.nPeakHight - peak_result.nStartHight;
					wPeakAmpNum++;
				}
				else
				{
					//缓存区已填满，则逐个移位，将最新的积分峰填充到队尾
					for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
					{
						wIntegPeakAmp[i] = wIntegPeakAmp[i + 1];
					}
					wIntegPeakAmp[INTEG_PEAK_BUFF_LEN - 1] = peak_result.nPeakHight - peak_result.nStartHight;
				}

				// 更新搜波阈值
				IntegThreshUpdate();

				return 1;
			}
		}
		else
		{

		}

		// 至此，说明该峰为噪声峰，填充噪声峰序列
		temp_amp = peak_result.nPeakHight - peak_result.nStartHight;
		if ((temp_amp * 2) < wIntegAverageAmp)
		{
			if (wNoiseAmpNum < INTEG_PEAK_BUFF_LEN)
			{
				//缓存区尚未填满，直接填充
				wIntegNoiseAmp[wNoiseAmpNum] = temp_amp;
				wNoiseAmpNum++;
			}
			else
			{
				//缓存区已填满，则逐个移位，将最新的积分峰填充到队尾
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegNoiseAmp[i] = wIntegNoiseAmp[i + 1];
				}
				wIntegNoiseAmp[INTEG_PEAK_BUFF_LEN - 1] = temp_amp;
			}
		}
	}
	else
	{
	}

	//=========================================================================
	//3. 若连续5秒没有搜到脉搏波，则判断出现停搏
	//=========================================================================
	if (ECG_SAMPLE_RATE * 5 <= wTimeNoQrs)
	{
		// 超过5秒，判断为停搏，初始化测量结果，并判断为停搏
		EcgAlgGetResult(0, true);

		gEcgAlgResult.isAsystole = true;
	}
	else
	{
		gEcgAlgResult.isAsystole = false;
	}

	//未检测到QRS波的时间自增
	wTimeNoQrs++;
	return 0;
}

//平方积分信号中波峰检测函数
int IntegPeakDetection(int cur_data, int cur_pos, INTEG_PEAK *result, bool reset)
{
	static int last_data, last_ptr, peak_amp, peak_ptr, start_amp, start_ptr, flag;
	int peak_width = 0;

	if (reset)
	{
		flag = 0;
		last_data = 0;
		last_ptr = 0;
		peak_amp = 0;
		peak_ptr = 0;
		start_amp = 0;
		start_ptr = 0;

		return 0;
	}

	if (0 == flag)
	{
		if (cur_data > last_data)
		{
			start_amp = last_data;
			start_ptr = last_ptr;
			peak_amp = cur_data;
			peak_ptr = cur_pos;
			flag = 1;
		}
	}
	else
	{
		if (cur_data > last_data)
		{
			if (cur_data > peak_amp)
			{
				peak_amp = cur_data;
				peak_ptr = cur_pos;
			}
		}
		else
		{
			if (cur_data < (peak_amp + start_amp) / 2)
			{
				last_data = cur_data;
				last_ptr = cur_pos;
				flag = 0;

				peak_width = gTotalSampleBuff[peak_ptr] - gTotalSampleBuff[start_ptr];

				//通常QRS宽度为60 - 200ms，前后延拓，此处将宽度阈值设置为20 - 500ms
				//if ((peak_width > INTEG_PEAK_MAX_WIDTH) || (peak_width < INTEG_PEAK_MIN_WIDTH))
				if (peak_width < INTEG_PEAK_MIN_WIDTH)
				{
					// 调试发现上限阈值设置不合适，因为积分峰的起点可能定位过早，导致宽度较大，仅保留下限阈值
					return 0;
				}
				else
				{
					result->nPeakHight = peak_amp;
					result->nPeakPos = peak_ptr;
					result->nStartHight = start_amp;
					result->nStartPos = start_ptr;
					return 1;
				}
			}
		}
	}

	last_data = cur_data;
	last_ptr = cur_pos;
	return 0;
}


// 积分峰搜波阈值初始化函数
int IntegThreshInitalize(bool reset)
{
	INTEG_PEAK peak_result;
	int mid_value = 0, sec_value = 0, i = 0, integ_sum = 0, temp_amp = 0;

	static bool flag_find_integ;
	static int time_integ_init, integ_peak_num;

	if (reset)
	{
		flag_find_integ = false;
		time_integ_init = 0;
		integ_peak_num = 0;

		return 0;
	}

	if (flag_find_integ)
	{
		//找到第一个积分峰后方开始计算时间，避免将初始的无效信号计入
		time_integ_init ++;
	}

	if (IntegPeakDetection(gIntegDataBuff[gCurBuffPtr], gCurBuffPtr, &peak_result, false))
	{
		temp_amp = peak_result.nPeakHight - peak_result.nStartHight;

		if (temp_amp < wIntegAverageAmp / 2)
		{
			return 0;
		}
		else
		{
			// 将搜波结果存入积分峰缓存区
			if (wIntegPeakNum < INTEG_PEAK_BUFF_LEN)
			{
				//缓存区尚未填满，直接填充
				wIntegPeakBuff[wIntegPeakNum] = peak_result;
				wIntegPeakNum++;
			}
			else
			{
				//缓存区已填满，则逐个移位，将最新的积分峰填充到队尾
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegPeakBuff[i] = wIntegPeakBuff[i + 1];
				}
				wIntegPeakBuff[INTEG_PEAK_BUFF_LEN - 1] = peak_result;
			}

			// 填充有效积分峰幅度序列
			if (wPeakAmpNum < INTEG_PEAK_BUFF_LEN)
			{
				//缓存区尚未填满，直接填充
				wIntegPeakAmp[wPeakAmpNum] = temp_amp;
				wPeakAmpNum++;
			}
			else
			{
				//缓存区已填满，则逐个移位，将最新的积分峰填充到队尾
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegPeakAmp[i] = wIntegPeakAmp[i + 1];
				}
				wIntegPeakAmp[INTEG_PEAK_BUFF_LEN - 1] = temp_amp;
			}

			integ_peak_num ++;

			flag_find_integ = true;
		}

		// 计算有效积分峰的平均幅度
		if (integ_peak_num >= INTEG_PEAK_BUFF_LEN)
		{
			for (i = 0; i < INTEG_PEAK_BUFF_LEN; i++)
			{
				integ_sum += wIntegPeakAmp[i];
			}
			wIntegAverageAmp = integ_sum / INTEG_PEAK_BUFF_LEN;
		}
	}

	if ((time_integ_init >= ECG_SAMPLE_RATE * 3) && (integ_peak_num >= INTEG_PEAK_BUFF_LEN))
	{
		// 满足两个条件方进行阈值初始化
		// 1. 从搜到第一个波开始至少三秒；2. 至少搜到5个积分峰
		
		// 获取中值及次大值
		mid_value = SortAndSearch(wIntegPeakAmp, INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN + 1) / 2);
		sec_value = SortAndSearch(wIntegPeakAmp, INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN - 1));

		// 信号峰阈值为中值与次大值的均值，因为该阈值会自动调整减小，可能会漏检信号峰，但便于排除噪声峰
		// 改为均值的一半，因为若最初5个波正确，那么该阈值过大，容易导致漏检，并且干扰后续的噪声峰填充
		wIntegSigThresh = (mid_value + sec_value) / 4;
		// 噪声峰阈值为信号峰阈值的1/8
		wIntegNoiseThresh = mid_value / 8;

		// 填充信号峰和噪声峰缓存区
		wPeakAmpNum = 0;
		wNoiseAmpNum = 0;
		for (i = 0; i < INTEG_PEAK_BUFF_LEN; i++)
		{
			//wIntegPeakAmp[wPeakAmpNum] = wIntegSigThresh;
			wIntegNoiseAmp[wNoiseAmpNum] = wIntegNoiseThresh;
			wPeakAmpNum++;
			wNoiseAmpNum++;
		}

		return 1;
	}

	return 0;
}

// 更新积分峰搜波阈值
void IntegThreshUpdate(void)
{
	int peak_mid = 0, noise_mid = 0, peak_pre = 0, peak_last = 0, temp = 0, integ_sum = 0, i = 0;

	//=========================================================================
	//1. 获取信号峰和噪声峰中值、获取最新的两个信号峰值，以备后续判断
	//=========================================================================
	peak_mid  = SortAndSearch(wIntegPeakAmp,  INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN + 1) / 2);				//信号峰中值
	noise_mid = SortAndSearch(wIntegNoiseAmp, INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN + 1) / 2);				//噪声峰中值

	if (2 > wPeakAmpNum)
	{
		return;
	}
	else
	{
		// 获取最新及次新信号峰幅度，以判断信号突变
		peak_last = wIntegPeakAmp[wPeakAmpNum - 1];
		peak_pre = wIntegPeakAmp[wPeakAmpNum- 2];
	}
	

	//=========================================================================
	//2. 判断信号是否存在突变
	//=========================================================================
	if (((peak_mid > peak_last * 4) && (peak_mid > peak_pre * 4)) || ((peak_mid * 4 < peak_last) && (peak_mid * 4 < peak_pre)))
	{
		// 信号突变，以最近的两个信号峰的均值更新阈值
		temp = (peak_last + peak_pre) / 2 - noise_mid;
	}
	else
	{
		// 信号未突变，以信号峰中值更新阈值
		temp = peak_mid - noise_mid;
	}

	//=========================================================================
	//3. 计算并更新阈值
	//=========================================================================
	// 计算信号、噪声峰差值的0.1875 （误检过多，更改阈值为差值的0.5）modified by mao 14.05.03
	if(temp >= 0)
	{
		//temp = (temp >> 3) + (temp >> 4);
		temp = temp>>1;
	}
	else
	{
		//temp = -(((-temp) >> 3) + ((-temp) >> 4));
		temp = -(temp>>1);
	}

	//=========================================================================
	//4. 计算有效积分峰的均值，避免将漏检的波峰归入噪声峰
	//=========================================================================
	// 计算阈值
	wIntegSigThresh = noise_mid + temp;

	if (wPeakAmpNum > 0)
	{
		for (i = 0; i < wPeakAmpNum; i++)
		{
			integ_sum += wIntegPeakAmp[i];
		}
		wIntegAverageAmp = integ_sum / wPeakAmpNum;
	}
}


// QRS波有效性判断函数
int QrsComplexJudge(void)
{
	BP_DIFF_PEAK diff_peak;
	QRS_PEAK_POS band_peak;
	QRS_PEAK_POS view_peak;
	int flag = 0, temp_value = 0;

	//=========================================================================
	//1. 首先在带通差分信号上搜索差分信号的最大值、最小值、过零点等，理论上过零点对应于带通信号的波峰位置
	//=========================================================================
	flag = BpDiffDataPeakDetect(&diff_peak);

	if (flag)
	{
		flag = BandPeakDetect(&diff_peak, &band_peak);
	}

	if (flag)
	{
		flag = ViewPeakDetect(&band_peak, &view_peak);
	}

	if (flag)
	{
		flag = QrsComplexExclusion(&view_peak, &band_peak);
	}

	if (flag)
	{
		// 至此，说明确定QRS波搜波成功

		// 差分峰幅度累加，为后续噪声检测做准备
		temp_value = gDiffDataBuff[diff_peak.nDiffMaxPos] - gDiffDataBuff[diff_peak.nDiffMinPos];
		if (0 <= temp_value)
		{
			wAveDiffPeak = wAveDiffPeak + temp_value;
		}
		else
		{
			// 理论上temp_value应该都是大于零的，到这里一般来说是有问题的
			wAveDiffPeak = wAveDiffPeak - temp_value;
		}
		wAveDiffPeakNum++;

		// 输出显示峰的搜波调试信息
		gEcgDebugInfo.ViewPeak.nStartPos = gTotalSampleBuff[view_peak.nStartPos];
		gEcgDebugInfo.ViewPeak.nPeakPos = gTotalSampleBuff[view_peak.nPeakPos];
		gEcgDebugInfo.ViewPeak.nEndPos  = gTotalSampleBuff[view_peak.nEndPos];
		
		return 1;
	}
	else
	{
		return 0;
	}

}

// 在带通差分信号上进行QRS波位置初步定位
int BpDiffDataPeakDetect(BP_DIFF_PEAK *diff_peak)
{
	int temp_pos = 0, max_pos = 0, min_pos = 0, max_value = 0, min_value = 0;
	int i = 0, pos_diff = 0, slop_thresh = 0, wind_begin = 0, wind_end = 0, wind_len = 0;
	int last_state = 0, cur_state = 0, last_value = 0, cur_value = 0, slop_cnt = 0, peak_cnt = 0;

	// 初始化输出信息
	diff_peak->nDiffStartPos = 0;
	diff_peak->nDiffEndPos = 0;
	diff_peak->nDiffZeroPos = 0;
	diff_peak->nDiffMaxPos = 0;
	diff_peak->nDiffMinPos = 0;
	
	wCurQrsPeak.isPeakBefore = 0;

	if (gQrsInfo.nAveInterval)
	{
		wind_len = gQrsInfo.nAveInterval * 2 / 3;
	}
	else
	{
		wind_len = SEARCH_WINDOW;
	}

	if (wind_len > SEARCH_WINDOW)
	{
		wind_len = SEARCH_WINDOW;
	}

	// 确定搜索的起始和中止位置
	wind_begin = mod((wIntegPeakBuff[wIntegPeakNum - 1].nPeakPos - wind_len), ECG_DATA_BUFF_LEN);
	wind_end = gCurBuffPtr;
	
	//=========================================================================
	//1. 搜索差分信号中最大值、次大值和最小值的位置
	//=========================================================================
	// 从搜索窗的起始点开始向后搜索，定位最大、最小值位置
	temp_pos = wind_begin;
	for(i = 0; i < wind_len; i++)
	{
		cur_value = gDiffDataBuff[temp_pos];

		if(cur_value > max_value)
		{
			//逐点对比，确定最大值位置
			if(last_value <= 0)
			{
				// 如果上一个极值是最小值，那么只有当当前值大于前面一个极大值的2倍时，方才进行更新
				// 这个策略的目的是防止QRS波的正负方向搜索错误，以正向QRS波为优先
				//if(cur_value > max_value * 6 / 5)
				if(cur_value > max_value * 2)
				{
					max_pos = temp_pos;
					max_value = cur_value;
					last_value = cur_value;
				}
			}
			else
			{
				max_pos = temp_pos;
				max_value = cur_value;
			}
		}
		else if(cur_value < min_value)
		{
			// 逐点对比，确定最小值位置
			if(last_value >= 0)
			{
				if(cur_value < min_value * 4 / 3)
				{
					min_pos = temp_pos;
					min_value = cur_value;
					last_value = cur_value;
				}
			}
			else
			{
				min_pos = temp_pos;
				min_value = cur_value;
			}
		}

		// 指针自增
		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	// 判断该峰是否有效
	if (gDiffDataBuff[max_pos] * gDiffDataBuff[min_pos] > 0)
	{
		// 最大、最小差分值同向，说明该峰无效
		return 0;
	}

	//=========================================================================
	//2. 至此，说明该差分峰有效，判断该Peak的类型，并存贮最大最小斜率的位置
	//=========================================================================
	pos_diff = gTotalSampleBuff[max_pos] - gTotalSampleBuff[min_pos];
	diff_peak->nDiffMaxPos = max_pos;
	diff_peak->nDiffMinPos = min_pos;

	if (0 < pos_diff)
	{
		// 最小值在前，最大值在后，说明是负向峰
		diff_peak->nDiffStartPos = min_pos;
		diff_peak->nDiffEndPos = max_pos;
		wCurQrsPeak.isPeakBefore = 0;
	}
	else if(0 > pos_diff)
	{
		// 最大值在前，最小值在后，说明是正向峰
		diff_peak->nDiffStartPos = max_pos;
		diff_peak->nDiffEndPos = min_pos;
		wCurQrsPeak.isPeakBefore = 1;
	}
	else
	{
		// 最大、最小值在同一位置，检测出错，直接返回
		return 0;
	}

	//=========================================================================
	//3. 确定过零点位置，对应带通信号的波峰位置
	//=========================================================================
	temp_pos = diff_peak->nDiffStartPos;
	last_value = gDiffDataBuff[temp_pos];
	while (temp_pos != diff_peak->nDiffEndPos)
	{
		cur_value = gDiffDataBuff[temp_pos];
		if (cur_value * last_value <= 0)
		{
			// 当前点与上一点不同号，说明找到过零点，记录过零点
			diff_peak->nDiffZeroPos = temp_pos;
			break;
		}
		else
		{
			temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
			last_value = cur_value;
		}
	}

	//=========================================================================
	//4. 确定起点位置（连续六点小于最大值1/5，或过零点）
	//=========================================================================
	// 设定阈值
	slop_thresh = Abs(gDiffDataBuff[diff_peak->nDiffStartPos]) / 5;

	// 确定搜索起始位置，此时nDiffStartPos是斜率最大/最小值位置
	temp_pos = diff_peak->nDiffStartPos;
	cur_value = gDiffDataBuff[temp_pos];
	last_value = cur_value;
	slop_cnt = 0;

	// 从起始位置开始往前搜索，确定起点
	while (temp_pos != wind_begin)
	{
		cur_value = gDiffDataBuff[temp_pos];

		if (0 >= (cur_value * last_value))
		{
			//遇到了斜率过零点，对应带通信号峰值点，认为找到了起点，跳出
			break;
		}
		else
		{
			// 如果当前值小于最大斜率的1/5，计数器自增
			if (Abs(cur_value) <= slop_thresh)
			{
				slop_cnt ++;
			}
			else
			{
				slop_cnt = 0;
			}

			if (6 <= slop_cnt)
			{
				// 连续6点小于阈值，认为找到了起点，跳出
				break;
			}

			// 维护序列及相关参数
			last_value = cur_value;
			temp_pos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}
	}

	// 若到搜索窗起始位置还没找到起点，说明波形有误，退出
	if ((temp_pos != diff_peak->nDiffStartPos) && (temp_pos != wind_begin))
	{
		// 此时nDiffStartPos是起点附近的极值点位置，temp_pos是起点位置
		max_value = Abs(gDiffDataBuff[diff_peak->nDiffStartPos] - gDiffDataBuff[temp_pos]);
		diff_peak->nDiffStartPos = temp_pos;
	}
	else
	{
		return 0;
	}

	//=========================================================================
	//5. 确定终点位置（连续六点小于最大值1/5，或过零点）
	//=========================================================================
	// 设定阈值
	slop_thresh = Abs(gDiffDataBuff[diff_peak->nDiffEndPos]) / 5;

	// 确定搜索起始位置，此时nDiffStartPos是斜率最大/最小值位置
	temp_pos = diff_peak->nDiffEndPos;
	cur_value = gDiffDataBuff[temp_pos];
	last_value = cur_value;
	slop_cnt = 0;

	// 从起始位置开始往后搜索，确定终点
	while (temp_pos != wind_end)
	{
		cur_value = gDiffDataBuff[temp_pos];

		if (0 >= (cur_value * last_value))
		{
			//遇到了斜率过零点，对应带通信号峰值点，认为找到了起点，跳出
			break;
		}
		else
		{
			// 如果当前值小于最大斜率的1/5，计数器自增
			if (Abs(cur_value) <= slop_thresh)
			{
				slop_cnt ++;
			}
			else
			{
				slop_cnt = 0;
			}

			if (6 <= slop_cnt)
			{
				// 连续6点小于阈值，认为找到了起点，跳出
				break;
			}

			// 维护序列及相关参数
			last_value = cur_value;
			temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}
	}

	// 若到搜索窗起始位置还没找到起点，说明波形有误，退出
	if ((temp_pos != diff_peak->nDiffEndPos) && (temp_pos != wind_end))
	{
		// 此时nDiffEndPos是终点附近的极值点位置，temp_pos是终点位置
		min_value = Abs(gDiffDataBuff[diff_peak->nDiffEndPos] - gDiffDataBuff[temp_pos]);
		diff_peak->nDiffEndPos = temp_pos;
	}
	else
	{
		return 0;
	}

	//=========================================================================
	//6. 形态判断，若最大、最小幅度相差太大，或起止点间峰值点过多，则判为误检
	//=========================================================================
	//6.1 通常来说，正常QRS波的上升段和下降段的斜率应该较接近，因此若最大、最小斜率幅度比值过大，则可能是梯形基漂干扰
	if ((max_value * 5 < min_value) || (max_value > min_value * 5))
	{
		//最大、最小斜率幅度比值相差超过5倍，判为干扰
		return 0;
	}

	//6.2 对带通差分来说，一组极值对有2个峰，通常应该不超过4个峰。若有5个峰及以上，则说明波形有可能是震荡干扰所致
	//i = mod((diff_peak->nDiffStartPos + 2), ECG_DATA_BUFF_LEN);
	//temp_pos   = mod((diff_peak->nDiffEndPos - 2), ECG_DATA_BUFF_LEN);

	i = mod((diff_peak->nDiffStartPos + 1), ECG_DATA_BUFF_LEN);
	cur_value = 0;
	cur_state = 0;
	last_value = 0;
	last_state = 0;
	peak_cnt = 0;

	// 统计峰谷个数
	while (i != diff_peak->nDiffEndPos)
	{
		// 获取中间点幅值
		cur_value = gDiffDataBuff[i];

		// 趋势检测，若是上升则为1，若是下降则为-1
		if (cur_value > last_value)
		{
			cur_state = 1;
		}
		else if (cur_value < last_value)
		{
			cur_state = -1;
		}
		else
		{
			cur_state = cur_state;
		}

		// 峰值检测，若趋势状态发生改变，说明有峰谷存在
		if (0 > (cur_state * last_state))
		{
			peak_cnt++;
		}

		last_state = cur_state;
		last_value = cur_value;

		/*
		// 获取中间点幅值
		cur_value = gDiffDataBuff[i];

		// 获取前一点幅值
		wind_begin = mod((i - 1), ECG_DATA_BUFF_LEN);
		min_value = gDiffDataBuff[wind_begin];

		// 获取后一点幅值
		wind_end = mod((i + 1), ECG_DATA_BUFF_LEN);
		max_value = gDiffDataBuff[wind_end];

		// 判断是否波峰
		if ((cur_value >= min_value) && (cur_value >= max_value))
		{
			// 中间点大于前后点，为波峰
			peak_cnt ++;
		}
		else if ((cur_value <= min_value) && (cur_value <= max_value))
		{
			// 中间点小于前后点，为波谷
			peak_cnt ++;
		}
		else
		{
			// 非峰谷
		}
		*/

		i = LoopInc(i, 1, ECG_DATA_BUFF_LEN);
	}
	
	// 判断是否干扰
	if (peak_cnt > 6)
	{
		return 0;
	}


	// 至此，说明差分峰搜索成功，输出调试信息
	gEcgDebugInfo.BpDiffPeak.nDiffStartPos = gTotalSampleBuff[diff_peak->nDiffStartPos];
	gEcgDebugInfo.BpDiffPeak.nDiffZeroPos = gTotalSampleBuff[diff_peak->nDiffZeroPos];
	gEcgDebugInfo.BpDiffPeak.nDiffEndPos = gTotalSampleBuff[diff_peak->nDiffEndPos];
	gEcgDebugInfo.BpDiffPeak.nDiffMaxPos = gTotalSampleBuff[diff_peak->nDiffMaxPos];
	gEcgDebugInfo.BpDiffPeak.nDiffMinPos = gTotalSampleBuff[diff_peak->nDiffMinPos];

	return 1;
}


// 在带通波形上，检测QRS波的位置
int BandPeakDetect(BP_DIFF_PEAK *diff_peak, QRS_PEAK_POS *band_peak)
{
	int temp_pos = 0, cur_value = 0, max_value = 0, min_value = 0, max_pos = 0, min_pos = 0;
	int last_value = 0, next_value = 0, search_end = 0, search_start = 0, i = 0;
	bool is_peak = false, is_valley = false;
	QRS_PEAK_POS high_peak;

	// 输出带通信号上的Peak位置，便于调试
	band_peak->nStartPos = mod((diff_peak->nDiffStartPos - DIFF_PASS_DELAY), ECG_DATA_BUFF_LEN);
	band_peak->nPeakPos = mod((diff_peak->nDiffZeroPos - DIFF_PASS_DELAY), ECG_DATA_BUFF_LEN);
	band_peak->nEndPos  = mod((diff_peak->nDiffEndPos   - DIFF_PASS_DELAY), ECG_DATA_BUFF_LEN);

	//=========================================================================
	//1. 根据波的形态，准确定位峰、谷点位置
	//=========================================================================
	if (wCurQrsPeak.isPeakBefore)
	{
		// 波峰在前
		// 查分信号的5个点，起点、最大点、过零点、       最小点、终点
		// 对应于带通5个点，起点、上升点、正向峰值点、下降点、谷点
		// 所以采用峰值点和终点最为搜索的参考点
		max_pos = band_peak->nPeakPos;
		min_pos = band_peak->nEndPos;	
	}
	else
	{
		// 波峰在后
		// 差分信号的5个点，起点、    最小点、过零点、       最大点、终点
		// 对应于带通5个点，最大点、下降点、反向峰值点、上升点、终点
		// 所以采用起点和峰值点最为搜索的参考点
		max_pos = band_peak->nStartPos;
		min_pos = band_peak->nPeakPos;
	}

	// 1.1 以max_pos为基准，前后搜索5个点，寻找最大值
	temp_pos = mod((max_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((max_pos + 5), ECG_DATA_BUFF_LEN);
	max_value = gBpDataBuff[temp_pos];

	// 搜索十个点，定位波谷位置
	while (temp_pos != search_end)
	{
		cur_value = gBpDataBuff[temp_pos];
		if (cur_value >= max_value)
		{
			max_value = cur_value;
			max_pos = temp_pos;
		}

		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	// 1.2 以min_pos为基准，前后搜索5个点，寻找谷值点
	temp_pos = mod((min_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((min_pos + 5), ECG_DATA_BUFF_LEN);
	min_value = gBpDataBuff[temp_pos];

	// 搜索十个点，定位波谷位置
	while (temp_pos != search_end)
	{
		cur_value = gBpDataBuff[temp_pos];
		if (cur_value <= min_value)
		{
			min_value = cur_value;
			min_pos = temp_pos;
		}

		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	if (wCurQrsPeak.isPeakBefore)
	{
		// 波峰在前
		 band_peak->nPeakPos = max_pos;
		 band_peak->nEndPos = min_pos;	
	}
	else
	{
		// 波谷在前
		 band_peak->nStartPos = max_pos;
		 band_peak->nPeakPos = min_pos;
	}


	//=========================================================================
	//2.判断，波峰和波谷是否存在（有可能不存在波谷，直接搜索到最小值，远离波峰，可作为终点搜索的依据）
	//     todo：这一部分的结果没有用上
	//=========================================================================
	last_value = gBpDataBuff[LoopDec(max_pos, 1, ECG_DATA_BUFF_LEN)];
	cur_value = gBpDataBuff[max_pos];
	next_value = gBpDataBuff[LoopInc(max_pos, 1, ECG_DATA_BUFF_LEN)];
	if ((cur_value >= last_value) && (cur_value >= next_value))
	{
		is_peak = true;
	}
	else
	{
		is_peak = false;
	}

	last_value = gViewDataBuff[LoopDec(min_pos, 1, ECG_DATA_BUFF_LEN)];
	cur_value = gViewDataBuff[min_pos];
	next_value = gViewDataBuff[LoopInc(min_pos, 1, ECG_DATA_BUFF_LEN)];
	if ((cur_value <= last_value) && (cur_value <= next_value))
	{
		is_valley = true;
	}
	else
	{
		is_valley = false;
	}

	//=========================================================================
	//3. 判断该QRS波的类型
	//=========================================================================
	if (wCurQrsPeak.isPeakBefore)
	{
		// 若波峰在前，直接判断为正向QRS波
		wCurQrsPeak.isPositive = true;
	}
	else
	{
		// 波峰在后，则需要考虑波峰波谷的幅度情况，修正QRS波的方向
		// 此时nStartPos是波峰，nPeakPos是波谷，nEndPos是终点
		max_value = Abs(gBpDataBuff[band_peak->nStartPos] - gBpDataBuff[band_peak->nEndPos]);
		min_value = Abs(gBpDataBuff[band_peak->nPeakPos] - gBpDataBuff[band_peak->nEndPos]);
		
		// 若波峰幅度大于波谷幅度的2/3，则认为是正向QRS波
		if (max_value * 2 >= min_value * 1)
		{
			wCurQrsPeak.isPositive = true;
		}
		else
		{
			wCurQrsPeak.isPositive = false;
		}
	}

	//=========================================================================
	// 4. 计算高通QRS波的幅度，这里还是计算峰峰值，因为区分正负向比较困难，容易造成较多的误检
	//=========================================================================
	// 确定搜索区域，从起点前2点开始，到峰值点后2点结束
	search_start = mod((band_peak->nStartPos - LOW_PASS_DELAY - 2), ECG_DATA_BUFF_LEN);
	search_end = mod((band_peak->nEndPos - LOW_PASS_DELAY + 2), ECG_DATA_BUFF_LEN);

	max_value = gHpDataBuff[search_start];
	min_value = gHpDataBuff[search_start];
	temp_pos = search_start;
	while (temp_pos != search_end)
	{
		if (gHpDataBuff[temp_pos] >= max_value)
		{
			max_value = gHpDataBuff[temp_pos];
			max_pos = temp_pos;
		}

		if (gHpDataBuff[temp_pos] <= min_value)
		{
			min_value = gHpDataBuff[temp_pos];
			min_pos = temp_pos;
		}

		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	// 起点和峰点位置赋值
	high_peak.nStartPos =min_pos;
	high_peak.nPeakPos = max_pos;

	wCurQrsPeak.nQrsHpAmp = Abs(gHpDataBuff[high_peak.nPeakPos] - gHpDataBuff[high_peak.nStartPos]);


	//=========================================================================
	// 5. 至此，说明带通峰搜索成功，输出调试信息
	//=========================================================================
	gEcgDebugInfo.BandPeak.nStartPos = gTotalSampleBuff[band_peak->nStartPos];
	gEcgDebugInfo.BandPeak.nPeakPos = gTotalSampleBuff[band_peak->nPeakPos];
	gEcgDebugInfo.BandPeak.nEndPos  = gTotalSampleBuff[band_peak->nEndPos];

	gEcgDebugInfo.HighPeak.nStartPos = gTotalSampleBuff[high_peak.nStartPos];
	gEcgDebugInfo.HighPeak.nPeakPos = gTotalSampleBuff[high_peak.nPeakPos];
	//gEcgDebugInfo.HighPeak.nEndPos  = gTotalSampleBuff[high_peak.nEndPos];
	gEcgDebugInfo.HighPeak.nEndPos  = 0;

	return 1;
}

// 在显示波形上进行最终的QRS波特征点定位
int ViewPeakDetect(QRS_PEAK_POS *band_peak, QRS_PEAK_POS *view_peak)
{
	int temp_pos = 0, search_end = 0, cur_value = 0, max_value = 0, min_value = 0, max_pos = 0, min_pos = 0;
	int last_value = 0, next_value = 0, cur_slope = 0, max_slope = 0, slope_cnt = 0;
	bool is_peak = false, is_valley = false;

	// 根据位置关系，首先粗略计算显示信号上的Qrs波的起点、终点和波峰位置
	view_peak->nStartPos = mod((band_peak->nStartPos - (HIGH_PASS_DELAY + LOW_PASS_DELAY)), ECG_DATA_BUFF_LEN);
	view_peak->nPeakPos = mod((band_peak->nPeakPos - (HIGH_PASS_DELAY + LOW_PASS_DELAY)), ECG_DATA_BUFF_LEN);
	view_peak->nEndPos  = mod((band_peak->nEndPos   - (HIGH_PASS_DELAY + LOW_PASS_DELAY)), ECG_DATA_BUFF_LEN);

	//=========================================================================
	//1. 根据波的形态，准确定位峰、谷点位置
	//=========================================================================
	if (wCurQrsPeak.isPeakBefore)
	{
		// 波峰在前
		max_pos = view_peak->nPeakPos;
		min_pos = view_peak->nEndPos;	
	}
	else
	{
		// 波谷在前
		max_pos = view_peak->nStartPos;
		min_pos = view_peak->nPeakPos;
	}

	// 以peakpos为基准，前后搜索5个点，寻找最大值
	temp_pos = mod((max_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((max_pos + 5), ECG_DATA_BUFF_LEN);
	max_value = gViewDataBuff[temp_pos];

	while (temp_pos != search_end)
	{
		cur_value = gViewDataBuff[temp_pos];
		if (cur_value >= max_value)
		{
			max_value = cur_value;
			max_pos = temp_pos;
		}

		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	// 以EndPos为基准，前后搜索5个点，寻找谷值点
	temp_pos = mod((min_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((min_pos + 5), ECG_DATA_BUFF_LEN);
	min_value = gBpDataBuff[temp_pos];

	// 搜索十个点，定位波峰、波谷位置
	while (temp_pos != search_end)
	{
		cur_value = gViewDataBuff[temp_pos];
		if (cur_value <= min_value)
		{
			min_value = cur_value;
			min_pos = temp_pos;
		}

		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	if (wCurQrsPeak.isPeakBefore)
	{
		// 波峰在前
		view_peak->nPeakPos = max_pos;
		view_peak->nEndPos = min_pos;	
	}
	else
	{
		// 波谷在前
		view_peak->nStartPos = max_pos;
		view_peak->nPeakPos = min_pos;
	}

	// 判断，波峰和波谷是否存在（有可能不存在波谷，直接搜索到最小值，远离波峰，可作为终点搜索的依据）
	last_value = gViewDataBuff[LoopDec(max_pos, 1, ECG_DATA_BUFF_LEN)];
	cur_value = gViewDataBuff[max_pos];
	next_value = gViewDataBuff[LoopInc(max_pos, 1, ECG_DATA_BUFF_LEN)];
	if ((cur_value >= last_value) && (cur_value >= next_value))
	{
		is_peak = true;
	}
	else
	{
		is_peak = false;
	}

	last_value = gViewDataBuff[LoopDec(min_pos, 1, ECG_DATA_BUFF_LEN)];
	cur_value = gViewDataBuff[min_pos];
	next_value = gViewDataBuff[LoopInc(min_pos, 1, ECG_DATA_BUFF_LEN)];
	if ((cur_value <= last_value) && (cur_value <= next_value))
	{
		is_valley = true;
	}
	else
	{
		is_valley = false;
	}

	//=========================================================================
	//2. 定位起点位置  todo：负向QRS波没有进行细致搜波，后续考虑
	//=========================================================================
	if (wCurQrsPeak.isPositive && !wCurQrsPeak.isPeakBefore)
	{
		// qrs波为正向波，且波谷在前，此时nStart实际上是nPeak，起点尚未检测到
		// 将nStart点，实际是R波峰值点，设定为搜波起点
		temp_pos = view_peak->nStartPos;

		// 定义搜波终点，暂定为QRS半波宽度15点		todo：需根据平均RR间期调整
		search_end = mod((temp_pos - 15), ECG_DATA_BUFF_LEN);
		max_slope = 0;
		slope_cnt = 0;

		while (temp_pos != search_end)
		{
			// 获取当前点，及其前后两点的幅度
			cur_value = gViewDataBuff[temp_pos];
			last_value = gViewDataBuff[mod((temp_pos - 1), ECG_DATA_BUFF_LEN)];
			next_value = gViewDataBuff[mod((temp_pos + 1), ECG_DATA_BUFF_LEN)];

			// 若遇到波谷，则认为找到了起点位置
			if ((cur_value <= last_value) && (cur_value <= next_value))
			{
				break;
			}

			// 连续三点，斜率小于最大斜率1/5
			cur_slope = next_value - last_value;
			if (cur_slope >= max_slope)
			{
				// 更新最大斜率值
				max_slope = cur_slope;
				slope_cnt = 0;
			}
			else if (cur_slope * 5 <= max_slope)
			{
				// 若小于最大斜率的1/5，则slope计数自增
				slope_cnt ++;

				// 若有连续三个以上的点，则定位为起点
				if (3 <= slope_cnt)
				{
					break;
				}
			}
			else
			{
				// 斜率小于最大斜率，但大于最大斜率的1/5，计数置零
				slope_cnt = 0;
			}

			temp_pos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}

		// 重新定义起点和峰值点
		view_peak->nPeakPos = view_peak->nStartPos;
		view_peak->nStartPos = temp_pos;
	}
	else if (wCurQrsPeak.isPositive && wCurQrsPeak.isPeakBefore)
	{
		// 此时nPeak即是波峰位置，nStart为起点位置
		// 该nStart为带通起点位置，需要细化搜索

		// 从nPeak开始前向搜索，终点为nStart
		temp_pos = view_peak->nPeakPos;
		search_end = view_peak->nStartPos;
		max_slope = 0;
		slope_cnt = 0;

		while (temp_pos != search_end)
		{
			// 获取当前点，及其前后两点的幅度
			cur_value = gViewDataBuff[temp_pos];
			last_value = gViewDataBuff[mod((temp_pos - 1), ECG_DATA_BUFF_LEN)];
			next_value = gViewDataBuff[mod((temp_pos + 1), ECG_DATA_BUFF_LEN)];

			// 连续三点，斜率小于最大斜率1/5
			cur_slope = next_value - last_value;
			if (cur_slope >= max_slope)
			{
				// 更新最大斜率值
				max_slope = cur_slope;
				slope_cnt = 0;
			}
			else if (cur_slope * 5 <= max_slope)
			{
				// 若小于最大斜率的1/5，则slope计数自增
				slope_cnt ++;

				// 若有连续三个以上的点，则定位为起点
				if (3 <= slope_cnt)
				{
					break;
				}
			}
			else
			{
				// 斜率小于最大斜率，但大于最大斜率的1/5，计数置零
				slope_cnt = 0;
			}

			temp_pos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}

		// 重新定义起点
		if (view_peak->nStartPos == temp_pos)
		{
			// 说明一直搜索到起点都没找到连续下降点，则不更新，以当前起点为起点
		}
		else
		{
			// 在起点之前搜到了斜率连续下降点，则更新起点
			// 因为是连续三个小于最大斜率的点，因此要取第一个
			view_peak->nStartPos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
			//view_peak->nStartPos = temp_pos;
		}
	}
	else
	{
		;
	}

	//=========================================================================
	//3. 定位终点位置   todo：负向QRS波没有进行细致搜波，后续考虑
	//=========================================================================
	if (wCurQrsPeak.isPositive)
	{
		// qrs波为正向波，精确定位终点位置
		// 设定nStart为搜波起点位置，nEnd为搜波终点位置
		temp_pos = view_peak->nPeakPos;

		// 定义搜波终点，暂定为QRS半波宽度15点		todo：需根据平均RR间期调整
		search_end = view_peak->nEndPos;
		max_slope = 0;
		slope_cnt = 0;

		while (temp_pos != search_end)
		{
			// 获取当前点，及其前后两点的幅度
			cur_value = gViewDataBuff[temp_pos];
			last_value = gViewDataBuff[mod((temp_pos - 1), ECG_DATA_BUFF_LEN)];
			next_value = gViewDataBuff[mod((temp_pos + 1), ECG_DATA_BUFF_LEN)];

			// 连续三点，斜率小于最大斜率1/5
			cur_slope = last_value - next_value;
			if (cur_slope >= max_slope)
			{
				// 更新最大斜率值
				max_slope = cur_slope;
				slope_cnt = 0;
			}
			else if (cur_slope * 5 <= max_slope)
			{
				// 若小于最大斜率的1/5，则slope计数自增
				slope_cnt ++;

				// 若有连续三个以上的点，则定位为起点
				if (3 <= slope_cnt)
				{
					break;
				}
			}
			else
			{
				// 斜率小于最大斜率，但大于最大斜率的1/5，计数置零
				slope_cnt = 0;
			}

			temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}

		// 重新定义终点
		if (view_peak->nEndPos == temp_pos)
		{
			// 说明一直搜索到终点都没找到连续下降点，则不更新，以当前终点为终点
		}
		else
		{
			// 在起点之前搜到了斜率连续下降点，则更新终点
			// 因为是连续三个小于最大斜率的点，因此要取第一个
			view_peak->nEndPos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
			//view_peak->nEndPos = temp_pos;
		}
	}

	return 1;
}

// 误检QRS波排除
int QrsComplexExclusion(QRS_PEAK_POS *view_peak, QRS_PEAK_POS *band_peak)
{
	int min_thresh = 0, max_thresh = 0, i = 0, cur_value = 0, last_ptr = 0, last_rr = 0;

	//=========================================================================
	//1. 计算QRS波的相关参数
	//=========================================================================
	// Qrs波特征值赋值
	wCurQrsPeak.nStartPos = view_peak->nStartPos;
	wCurQrsPeak.nPeakPos = view_peak->nPeakPos;
	wCurQrsPeak.nEndPos  = view_peak->nEndPos;

	wCurQrsPeak.nPeakPosAll = gTotalSampleBuff[wCurQrsPeak.nPeakPos];

	// 将带通信号的波峰位置保存下来，方便后续的分类
	wCurQrsPeak.nBpPeakPos = band_peak->nPeakPos;

	// 计算幅度
	// wCurQrsPeak.nQrsAmp = Abs(gViewDataBuff[wCurQrsPeak.nPeakPos] - gViewDataBuff[wCurQrsPeak.nStartPos]);
	// 用PeakPos - StartPos的方法可能有问题，如果定位不准，幅度会非常小，产生漏检
	// 因此此处搜索起点和终点的最大最小值，并以该差值作为幅度值
	i = wCurQrsPeak.nStartPos;
	min_thresh = gViewDataBuff[wCurQrsPeak.nStartPos];
	max_thresh = gViewDataBuff[wCurQrsPeak.nStartPos];
	while (i != wCurQrsPeak.nEndPos)
	{
		cur_value = gViewDataBuff[i];
		if (cur_value < min_thresh)
		{
			min_thresh = cur_value;
		}

		if (cur_value > max_thresh)
		{
			max_thresh = cur_value;
		}

		i = LoopInc(i, 1, ECG_DATA_BUFF_LEN);
	}
	wCurQrsPeak.nQrsAmp = Abs(max_thresh - min_thresh);

	// 计算宽度
	wCurQrsPeak.nQrsWidth = gTotalSampleBuff[wCurQrsPeak.nEndPos] - gTotalSampleBuff[wCurQrsPeak.nStartPos];
	if (0 >= wCurQrsPeak.nQrsWidth)
	{
		//Qrs波宽度小于等于0，说明该波有问题，删掉
		return 0;
	}

	// 计算与前一QRS波的RR间期
	if (0 != gQrsPeakNum)
	{
		wCurQrsPeak.nRRInterval = wCurQrsPeak.nPeakPosAll - gQrsPeakBuff[gQrsPeakNum - 1].nPeakPosAll;

		if (0 >= wCurQrsPeak.nRRInterval)
		{
			//RR间期小于等于0，有问题，删掉
			return 0;
		}
	}
	else
	{
		// 暂时还没有QRS波检出，间期设置为0
		wCurQrsPeak.nRRInterval = 0;
	}

	//=========================================================================
	//2. 幅度判断，排除一些过高或过低的干扰
	//=========================================================================
	// 如果幅度小于最小、最大幅度阈值（100LSB，约0.25mV / 4000LSB，10mV），则排除掉
	if ((QRS_PEAK_MIN_THRESH > wCurQrsPeak.nQrsAmp) || (QRS_PEAK_MAX_THRESH < wCurQrsPeak.nQrsAmp))
	{
		return 0;
	}

	// 如果平均幅度和平均间期均不为零，则与平均幅度进行比较
	if ((0 < gQrsInfo.nAveQrsAmp) && (0 < gQrsInfo.nAveInterval))
	{
		min_thresh = 0;
		max_thresh = 0;

		// 设置历史幅度阈值
		if (0 == gQrsPeakNum)
		{
			// 这个地方是这样设计的：
			// 如果平均宽度和平均间期都已经存在，但是QRS个数为0，说明是重算
			// 重算的初始阶段，不能因为nRRInterval为0，而令判断条件过严
			// 这样有可能漏检初始阶段的PVC，因此当作正常RR间期处理
			min_thresh = gQrsInfo.nAveQrsAmp / 3;
			max_thresh = gQrsInfo.nAveQrsAmp * 3;
		}
		else if (wCurQrsPeak.nRRInterval * 3 < gQrsInfo.nAveInterval * 2)
		{
			//当rr间期比平均rr间期的2/3还小时，阈值设置更紧一点，要求在平均幅度的1/2与2倍之间
			min_thresh = gQrsInfo.nAveQrsAmp / 2;
			max_thresh = gQrsInfo.nAveQrsAmp * 2;
		}
		else if (wCurQrsPeak.nRRInterval < gQrsInfo.nAveInterval)
		{
			//rr间期小于平均间期，则放宽阈值，1/3与3倍之间都可以
			min_thresh = gQrsInfo.nAveQrsAmp / 3;
			max_thresh = gQrsInfo.nAveQrsAmp * 3;
		}
		else
		{
			//若此时RR间期已经大于平均间期，则不设置阈值限制，避免长时间搜波失败
			min_thresh = QRS_PEAK_MIN_THRESH;
			max_thresh = QRS_PEAK_MAX_THRESH;
		}

		if ((wCurQrsPeak.nQrsAmp < min_thresh) || (wCurQrsPeak.nQrsAmp > max_thresh))
		{
			// 或超出平均幅度阈值，则排除掉
			return 0;
		}
	}

	//=========================================================================
	//3. 宽度判断，主要是用来排除T波误检
	//=========================================================================
	// 如果平均间期与平均宽度均不为零，则进行宽度的判断，以剔除T波
	if ((0 < gQrsInfo.nAveWidth) && (0 < gQrsInfo.nAveInterval))
	{
		min_thresh = 0;
		max_thresh = 0;

		// 设置宽度阈值
		if (0 == gQrsPeakNum)
		{
			// 这个地方是这样设计的：
			// 如果平均宽度和平均间期都已经存在，但是QRS个数为0，说明是重算
			// 重算的初始阶段，不能因为nRRInterval为0，而令判断条件过严
			// 这样有可能漏检初始阶段的PVC，因此当作正常RR间期处理
			min_thresh = gQrsInfo.nAveWidth / 3;
			max_thresh = gQrsInfo.nAveWidth * 3;
		}
		else if (wCurQrsPeak.nRRInterval * 2 < gQrsInfo.nAveInterval)
		{
			//当rr间期比平均rr间期的1/2还小时，阈值设置更紧一点，要求在平均幅度的1/2与1.5倍之间
			min_thresh = gQrsInfo.nAveWidth / 2;
			max_thresh = gQrsInfo.nAveWidth * 3 / 2;
		}
		else if (wCurQrsPeak.nRRInterval < gQrsInfo.nAveInterval)
		{
			//rr间期小于平均间期，则放宽阈值，1/3与3倍之间都可以
			min_thresh = gQrsInfo.nAveWidth / 3;
			max_thresh = gQrsInfo.nAveWidth * 3;
		}
		else
		{
			//若此时RR间期已经大于平均间期，则不设置阈值限制，避免长时间搜波失败
			min_thresh = 1;
			max_thresh = 100;
		}

		if ((wCurQrsPeak.nQrsWidth < min_thresh) || (wCurQrsPeak.nQrsWidth > max_thresh))
		{
			// 若超出宽度阈值，则排除掉
			return 0;
		}
	}


	//=========================================================================
	// 4. RR间期判断，仅用以输出，对于一些过大的RR间期不予采信
	//=========================================================================
	// 设置默认为false
	wCurQrsPeak.isSolid = false;

	// 满足两个条件判断为可靠
	// 1. 在平均RR间期的0.5 - 1.5之间
	// 2. 在上一个RR间期的0.8 - 1.2之间
	if (0 != gQrsInfo.nAveInterval)
	{
		if ((gQrsInfo.nAveInterval <= wCurQrsPeak.nRRInterval * 2) && (gQrsInfo.nAveInterval * 3 >= wCurQrsPeak.nRRInterval * 2))
		{
			// 获取上一个QRS波的rr间期，这时候应该不会得到无效值
			last_rr = gQrsPeakBuff[gQrsPeakNum - 1].nRRInterval;
			
			if ((wCurQrsPeak.nRRInterval * 10 >= last_rr * 8) && (wCurQrsPeak.nRRInterval * 10 <= last_rr * 12))
			{
				wCurQrsPeak.isSolid = true;
			}
		}
	}

	//=========================================================================
	// 斜率排除，新检出QRS波的斜率不应小于历史平均斜率的1/5    todo
	//=========================================================================

	return 1;
}

// 维护QRS波队列，更新相关QRS波参数
void QrsComplexUpdate(void)
{
	int i = 0, sum = 0, cunt = 0, temp = 0;

	//=========================================================================
	//1. 填充QRS波缓存区
	//=========================================================================
	if (gQrsPeakNum < QRS_PEAK_BUFF_LEN)
	{
		//缓存区尚未填满，直接填充
		gQrsPeakBuff[gQrsPeakNum] = wCurQrsPeak;
		gQrsPeakNum++;
	}
	else
	{
		//缓存区已填满，则逐个移位，将最新的QRS波填充到队尾
		for (i = 0; i < QRS_PEAK_BUFF_LEN - 1; i++)
		{
			gQrsPeakBuff[i] = gQrsPeakBuff[i + 1];
		}
		gQrsPeakBuff[QRS_PEAK_BUFF_LEN - 1] = wCurQrsPeak;
	}

	// 将当前的QRS波填充进算法实时结果信息
	gEcgAlgResult.EcgOntimeQrs = wCurQrsPeak;

	//=========================================================================
	//2. 更新QRS波的全局特征参数  todo：应该采用排序、取大值得方法计算
	//=========================================================================
	if (4 >= gQrsPeakNum)
	{
		// QRS波太少的情况下，不进行此计算
		return;
	}

	// 更新平均宽度
	sum = 0;
	cunt = 0;
	gQrsInfo.nAveWidth = 0;
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nQrsWidth)
		{
			sum += gQrsPeakBuff[i].nQrsWidth;
			cunt++;
		}
	}
	gQrsInfo.nAveWidth = sum / cunt;

	// 更新平均幅度
	sum = 0;
	cunt = 0;
	gQrsInfo.nAveQrsAmp = 0;
	for (i = gQrsPeakNum > 5 ? gQrsPeakNum - 5:0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nQrsAmp && gQrsPeakBuff[i].isNoisePeak != true)
		{
			sum += gQrsPeakBuff[i].nQrsAmp;
			cunt++;
		}
	}
	gQrsInfo.nAveQrsAmp = sum / cunt;

	// 更新平均rr间期
	sum = 0;
	cunt = 0;
	gQrsInfo.nAveInterval = 0;
	for (i = gQrsPeakNum > 5 ? gQrsPeakNum - 5:0; i < gQrsPeakNum; i++)
	{
		temp = gQrsPeakBuff[i].nRRInterval;
		// 如果当前的RR间期在2s（30BPM）和200ms（300BPM）之间，则可以计算
		if ((ECG_SAMPLE_RATE * 2 >= temp) && (ECG_SAMPLE_RATE / 5 <= temp))
		{
			if (0 != temp && gQrsPeakBuff[i].isNoisePeak != true)
			{
				sum += gQrsPeakBuff[i].nRRInterval;
				cunt++;
			}
		}
	}
	if (0 != cunt)
	{
		gQrsInfo.nAveInterval = sum / cunt;
	}
	
}


// 噪声检测主函数
int EcgNoiseEstimate(bool reset)
{
	int temp_pos = 0, noise_type = 0, i = 0, diff_max = 0, peak_num = 0;
	int diff_min = 0, diff_cur = 0, std_value = 0, peak_ratio = 0;
	int final_pos = 0, cur_amp = 0, is_PVC = 0, noise_thr = 0;

	if (reset)
	{
		gEcgNoiseBuffNum = 0;
		
		for (i = 0; i < ECG_NOISE_BUFF_LEN; i ++)
		{
			gEcgNoiseBuff[i].nNoiseStartPosAll = 0;
			gEcgNoiseBuff[i].nNoiseType = 0;
			gEcgNoiseBuff[i].nNoiseStd = 0;
			gEcgNoiseBuff[i].nPeakRatio = 0;
			gEcgNoiseBuff[i].nNoiseThr = 0;

			wDiffNoisePeakBuff[i] = 0;
		}

		gEcgNoiseTimeCnt = 0;
		gNoiseOrgSum = 0;
		wAveDiffPeak = 0;
		wAveDiffPeakNum = 0;
		wDiffNoisePeakNum = 0;

		return 0;
	}

	//=========================================================================
	// 1. 计算原始信号的标准差
	//=========================================================================
	// 直接用一个绝对值进行判断太粗暴了一点，要考虑QRS波的高度
	if (gQrsInfo.nAveQrsAmp)
	{
		// 如果存在平均QRS波幅度，就以其 1/4 作为阈值
		noise_thr = gQrsInfo.nAveQrsAmp / 4;
	}

	// 对过小的阈值进行限制，否则当QRS波幅度很小的时候，容易误报噪声
	if (noise_thr < ECG_NOISE_STD_THR)
	{
		noise_thr = ECG_NOISE_STD_THR;
	}

	std_value = EcgNoiseCalStd();

	if (std_value > noise_thr)
	{
		noise_type = 1;
	}
	else
	{
		noise_type = 0;
	}

	//=========================================================================
	//  2. 为了避免将室颤、室速的信号误判为噪声，此处判断3秒内所有QRS波的幅度变化情况
	//		如果最大幅度在最小幅度的1.1倍以内，说明是室速、室颤波形，不是噪声
	//		同时在此判断PVC，如果存在PVC，则不报噪声
	//=========================================================================
	if (1 == noise_type)
	{
		final_pos = gQrsPeakBuff[gQrsPeakNum-1].nPeakPosAll;
		diff_max = gQrsPeakBuff[gQrsPeakNum-1].nQrsAmp;
		diff_min = gQrsPeakBuff[gQrsPeakNum-1].nQrsAmp;

		for (i = 0; i < gQrsPeakNum - 1; i++)
		{
			// 仅统计三秒内的QRS波，获得最大、最小的QRS波的幅度
			if ((ECG_SAMPLE_RATE * 3) >= (final_pos - gQrsPeakBuff[i].nPeakPosAll))
			{
				// 范围内的QRS波个数计数
				peak_num ++;

				cur_amp = gQrsPeakBuff[i].nQrsAmp;
				if (cur_amp >= diff_max)
				{
					diff_max = cur_amp;
				}

				if (cur_amp <= diff_min)
				{
					diff_min = cur_amp;
				}

				// 判断是否是PVC
				if (100 == gQrsPeakBuff[i].nQrsType)
				{
					is_PVC = 1;
				}
			}
		}

		// 如果最大幅度在最小幅度的1.1倍以内，说明是室速、室颤波形，不是噪声
		// 上面的方法也有一个问题，在噪声中，如果只检测到一个或两个脉搏波，二者的差值恰恰很小，此时可能漏检噪声
		// 往往这种时候信号噪声是很大的
		// 如果存在PVC，则不报噪声
		if (((diff_max * 10 <= diff_min * 11) && (peak_num > 2)) || (1 == is_PVC))
		{
			noise_type = 0;
		}
	}

	//=========================================================================
	// 3. 计算带通差分信号中较大的波峰的数量
	//		todo：暂时保留差分峰的计算结果，但是目前不以该准则判断噪声
	//=========================================================================
	// 若平均差分峰存在，进行噪声检测
	if (wAveDiffPeakNum)
	{
		//检测
		 EcgNoisePeakDetect();

		//判断条件1，差分峰个数大于QRS数量的1.5倍
		if (((wDiffNoisePeakNum * 2) <= (wAveDiffPeakNum * 3)) || (0 >= wDiffNoisePeakNum))
		{
			// 噪声峰数量大于QRS波数量，但小于1.5倍，认为有噪声，但问题不大
			// 或者噪声峰小于等于0，认为没有噪声
			peak_ratio = 0;
		}
		else
		{
			// todo：这里是这样设计的，暂时保留差分峰的判断准则，但是目前不以该准则判断噪声
			peak_ratio = 1;

			// 噪声峰数量大于QRS波数量的1.5倍，认为噪声非常大
			// 判断条件2，噪声查分峰的最大最小值差异超过1.5倍
			diff_max = wDiffNoisePeakBuff[0];
			diff_min = wDiffNoisePeakBuff[0];

			for (i = 0; i < wDiffNoisePeakNum; i++)
			{
				diff_cur = wDiffNoisePeakBuff[i];
				if (diff_max < diff_cur)
				{
					diff_max = diff_cur;
				}

				if (diff_min >= diff_cur)
				{
					diff_min = diff_cur;
				}
			}

			/* todo: 暂时干掉，不以该标准评判
			if ((diff_min * 3) <= (diff_max * 2))
			{
				temp_type = 1;
			}
			else
			{
				temp_type = 0;
			}*/
		}
	}
	else
	{
		// 差分峰不存在，说明还没有检测到QRS波，不做噪声检测
		peak_ratio = 0;
	}

	//=========================================================================
	// 3. 将上述结果以及是否噪声填充到噪声缓冲区里面
	//=========================================================================
	//填充
	if (gEcgNoiseBuffNum < ECG_NOISE_BUFF_LEN)
	{
		// 若未满，直接填充
		temp_pos = mod((gCurBuffPtr + 1), ECG_DATA_BUFF_LEN);
		gEcgNoiseBuff[gEcgNoiseBuffNum].nNoiseStartPosAll = gTotalSampleBuff[temp_pos];
		gEcgNoiseBuff[gEcgNoiseBuffNum].nNoiseType = noise_type;
		gEcgNoiseBuff[gEcgNoiseBuffNum].nNoiseStd = std_value;
		gEcgNoiseBuff[gEcgNoiseBuffNum].nPeakRatio = peak_ratio;
		gEcgNoiseBuff[gEcgNoiseBuffNum].nNoiseThr = noise_thr;

		gEcgNoiseBuffNum++;
	}
	else
	{
		//若满，则移位填充，将最新的补到最后
		for (i = 0; i < (gEcgNoiseBuffNum - 2); i++)
		{
			gEcgNoiseBuff[i] = gEcgNoiseBuff[i+1];
		}

		temp_pos = mod((gCurBuffPtr + 1), ECG_DATA_BUFF_LEN);
		gEcgNoiseBuff[gEcgNoiseBuffNum - 1].nNoiseStartPosAll = gTotalSampleBuff[temp_pos];
		gEcgNoiseBuff[gEcgNoiseBuffNum - 1].nNoiseType = noise_type;
		gEcgNoiseBuff[gEcgNoiseBuffNum - 1].nNoiseStd = std_value;
		gEcgNoiseBuff[gEcgNoiseBuffNum - 1].nPeakRatio = peak_ratio;
	}

	//=========================================================================
	// 4. 设置噪声段QRS波的标志
	//=========================================================================
	if (noise_type)
	{
		for (i = 0; i < gQrsPeakNum - 1; i++)
		{
			// 仅统计三秒内的QRS波，获得最大、最小的QRS波的幅度
			if ((ECG_SAMPLE_RATE * 3) >= (final_pos - gQrsPeakBuff[i].nPeakPosAll))
			{
				gQrsPeakBuff[i].isNoisePeak = true;
			}
		}
	}

	//=========================================================================
	// 5. 重置，返回
	//=========================================================================
	gNoiseOrgSum = 0;
	wAveDiffPeak = 0;
	wAveDiffPeakNum = 0;
	wDiffNoisePeakNum = 0;

	return 1;
}

// 计算3秒钟的标准差
int EcgNoiseCalStd(void)
{
	int i = 0, temp = 0,  org_ave = 0;
	double square_sum = 0, std_result = 0;

	// 计算均值
	org_ave = gNoiseOrgSum / gEcgNoiseTimeCnt;

	for (i = 0; i < gEcgNoiseTimeCnt; i++)
	{
		temp = gViewDataBuff[i] - org_ave;
		square_sum += temp * temp;
	}

	std_result = sqrt(square_sum / gEcgNoiseTimeCnt);

	return std_result;
}


// 噪声峰检测函数
int EcgNoisePeakDetect(void)
{
	int noise_thr = 0, i = 0, start_pos = 0, end_pos = 0, temp_pos = 0, noise_peak_cnt = 0;
	int pre_value = 0, cur_value = 0, max_value = 0, min_value = 0, temp_value = 0;
	bool go_up = false, is_max = false, if_min = false;

	//=========================================================================
	//1. 计算噪声差分峰的幅度阈值，平均QRS差分峰的1/3
	//=========================================================================
	noise_thr = wAveDiffPeak / wAveDiffPeakNum;
	noise_thr = noise_thr / 2;
	if (0 >= noise_thr)
	{
		// 阈值小于等于0，直接返回
		return 0;
	}

	//=========================================================================
	//2. 搜索3秒内的差分极值对
	//=========================================================================
	// 计算Buff的起始位置
	start_pos = mod((gCurBuffPtr + 1), ECG_DATA_BUFF_LEN);
	end_pos = mod((gCurBuffPtr - 1), ECG_DATA_BUFF_LEN);
	i = start_pos;

	//初始化搜波相关参数
	pre_value = gDiffDataBuff[start_pos];
	max_value = pre_value;
	min_value = max_value;
	if (0 <= pre_value)
	{
		go_up = true;
	}
	else
	{
		go_up = false;
	}

	// 开始搜索极值对
	while (i != end_pos)
	{
		//获取前、中、后三点的幅度值
		cur_value = gDiffDataBuff[i];

		if (go_up)
		{
			// 目前处于波峰状态
			if (cur_value >= pre_value)
			{
				//仍然处于上升段，更新最大值
				if (cur_value >= max_value)
				{
					max_value = cur_value;
				}
			}
			else
			{
				// 进入下降状态了，判断是否跌破0值线
				if (0 >= (cur_value * pre_value))
				{
					// 跌破零值线，标记已找到极大值，并转入波谷状态
					is_max = true;
					go_up = false;
					min_value = cur_value;
				}
				else
				{
					// 在下降，但是还没跌破0值线，什么都不做，继续
				}
			}
		}
		else
		{
			// 处于波谷状态
			if (cur_value <= pre_value)
			{
				// 下降中，判断并更新最小值
				if (cur_value <= min_value)
				{
					min_value = cur_value;
				}
			}
			else
			{
				// 上升过程中，判断是否超过零值线
				if (0 >= (cur_value * pre_value))
				{
					// 突破零值线，这时认为一个极值对搜波周期结束，进行噪声峰计算和判断
					if (is_max)
					{
						// 极大值存在，计算极值对的幅度
						temp_value = max_value - min_value;

						if (temp_value >= noise_thr)
						{
							// 极值对的幅度大于阈值幅度，则噪声峰计数值+1
							// 如果噪声峰的个数小于缓存区长度，则直接填充
							if (wDiffNoisePeakNum < ECG_NOISE_BUFF_LEN - 1)
							{
								wDiffNoisePeakBuff[wDiffNoisePeakNum] = temp_value;
								wDiffNoisePeakNum ++;
							}
							else
							{
								// 噪声峰个数已经超过了最大长度，不用填充了，已经很大了
							}
						}

						// 清空极大值标志
						is_max = false;
					}

					// 进入波峰状态
					go_up = true;
					max_value = cur_value;
				}
				else
				{
					// 未突破零值线，继续
				}
			}
		}

		pre_value = cur_value;
		i = LoopInc(i, 1, ECG_DATA_BUFF_LEN);
	}

	return 1;
}


// 计算HRV值
float CalculateOntimeHrv(bool reset)
{
	int i = 0, cunt = 0, cur_rri = 0, next_rri = 0, rrd_ave = 0;
	int noise_cunt = 0, hrv_cunt = 0, start_noise = 0;
	float temp_value = 0, std_dev = 0, ontime_hrv = 0;

	if (reset)
	{
		gEcgHrvTimeCnt = 0;
		gTotalHrvSqr = 0;
		gTotalHrvNum = 0;
		return 0;
	}

	if (4 >= gQrsPeakNum)
	{
		// QRS波的个数小于5，不计算HRV
		return 0;
	}

	//=========================================================================
	// 1. 判断噪声状态，查找过去30s（10个噪声检测结果）中的噪声数量，并计算用于HRV的QRS波范围
	//=========================================================================
	start_noise = gEcgNoiseBuffNum - 10;
	if (start_noise < 0)
	{
		start_noise = 0;
	}

	for (i = start_noise; i <gEcgNoiseBuffNum; i++)
	{
		if (gEcgNoiseBuff[i].nNoiseType)
		{
			noise_cunt++;
		}
	}

	// 计算时间间隔的长度（这里用start_noise代替，减少参数），30s+剔除的噪声长度
	start_noise = ECG_HRV_COUNT + ECG_SAMPLE_RATE * noise_cunt * 3;

	//=========================================================================
	// 2. 若非噪声信号长度不够，则不计算HRV，否则计算相邻RRI的差值的平方和
	//=========================================================================
	if (gEcgTotalSampleNum > start_noise)
	{
		if (0 == gEcgAlgResult.nOntimeHrv)
		{
			// 如果是第一次计算HRV，则从第6个QRS波开始检测，避免初始阶段的误检影响
			rrd_ave = 6;
		}
		else
		{
			rrd_ave = 0;
		}

		for (i = rrd_ave; i < (gQrsPeakNum - 1); i++)
		{
			// 1. 30s内的QRS波;  2. 只采用可信的QRS波;  3. 不是噪声期间的QRS波
			if ((gQrsPeakBuff[i].nPeakPosAll + start_noise) >= gQrsPeakBuff[gQrsPeakNum-1].nPeakPosAll)
			{
				if ((0 != gQrsPeakBuff[i].nRRInterval) && (!gQrsPeakBuff[i].isNoisePeak))
				{
					//=========================================================================
					// 这一部分用于实时和最终的HRV计算
					//=========================================================================
					// 1. 计算前后QRS波间期的差值；2. 单位转换为ms；3. 确保精度，放大100倍；4. 计算平方和
					temp_value = Abs(gQrsPeakBuff[i+1].nRRInterval - gQrsPeakBuff[i].nRRInterval);
					temp_value = temp_value * 100000 / ECG_SAMPLE_RATE;

					std_dev += temp_value * temp_value;
					hrv_cunt ++;
				}
			}
		}

		//=========================================================================
		// 3. 若可信的RR间期个数符合要求，则计算RRI平方和的均方根
		//=========================================================================
		if (hrv_cunt >= 5)
		{
			// 首先将平方和以及参与计算的QRS波的个数存储下来
			gTotalHrvSqr += std_dev;
			gTotalHrvNum += hrv_cunt;

			// 计算均方根
			ontime_hrv = std_dev / hrv_cunt;
			ontime_hrv = sqrt(ontime_hrv);
		}
	}
	else
	{
		// 无噪声的信号不足30s，HRV无效
		ontime_hrv = 0;
	}

	
	//=========================================================================
	// 4. 输出结果
	//=========================================================================
	return ontime_hrv;
}

