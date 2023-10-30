//*************************************************************************
//文件名称：EcgResult.cpp
//
//文件说明：Ecg参数计算函数
//
//更改历史：Created by Chenmaomao [5 / 15 / 2014]
//*************************************************************************

//#include "stdafx.h"
#include "EcgResult.h"
#include <math.h>


// 初始化函数
void EcgResultInitialize(void)
{
	CalculateHeartRate(true);
	CalculateStValue(true);

	// 最终分析结果初始化
	EcgAlgGetFinalResult(0, true);

	// Holter结果初始化
	EcgAlgGetHolterOneMinResult(0, true);
	EcgAlgGetHolterFinalResult(0, true);
}


// 计算心率
int CalculateHeartRate(bool reset)
{
	int heart_rate = 0, i = 0, j = 0, interval = 0, cur_pos = 0, final_pos = 0, dist_pos = 0, temp = 0;
	int max_value = 0, min_value = 0, sum_value = 0, ave_value = 0, left = 0, right = 0, ave_rri = 0;

	unsigned short total_rri[50];						// 支持最高300BPM，10秒内有50个QRS波
	unsigned short rri_num = 0;
	unsigned short rri_dist = ECG_SAMPLE_RATE * 10;		// 定义HR计算的QRS波范围
	unsigned char noise_num = 0;				//噪声QRS个数

	static int real_output = 0;		//实际输出
	static int pre_heart_rate = 0;		//前一个有效输出hr
	static int pre_rri_num = 0;			//前一个有效窗口检测到R波数
	static unsigned char invaid_cnt = 0;	//无效计数

	if (5 > gQrsPeakNum)  // 5 -> 3
	{
		return 0;
	}

	//=========================================================================
	// 1.首先将10s内的QRS波的RR间期，填充到rri里面来
	//=========================================================================
	// 最近的一个QRS波的波峰位置
	final_pos = gQrsPeakBuff[gQrsPeakNum - 1].nPeakPosAll;

	// 填充10秒内的QRS波
	for (i = gQrsPeakNum - 1; i >= 0; i--)
	{
		cur_pos = gQrsPeakBuff[i].nPeakPosAll;
		dist_pos = final_pos - cur_pos;

		if ((dist_pos > 0) && (dist_pos <= rri_dist) && (rri_num < 50))
		{
			total_rri[rri_num] = gQrsPeakBuff[i].nRRInterval;
			rri_num ++;
			if (gQrsPeakBuff[i].isNoisePeak == true)
			{
				noise_num ++;
			}
		}
	}

	if (5 > rri_num)   // 5 -> 3 
	{
		return 0;
	}

	//=========================================================================
	// 2.冒泡排序，将间期按照升序排列
	//=========================================================================
	for (j = 0; j <= rri_num - 1; j++)
	{
		for(i = 0; i < rri_num - 1 - j; i++) 
		{
			if(total_rri[i] > total_rri[i+1])	
			{
				temp = total_rri[i];
				total_rri[i] = total_rri[i + 1];
				total_rri[i + 1] = temp;
			}	
		}
	}

	/*
	if (gQrsPeakNum >= 5)
	{
		
		//=========================================================================
		// 原来的代码
		//=========================================================================
		// 若rr间期的数量大于5，则开始计算心率，目的是避免初期的干扰导致心率计算错误
		max_value = 0;
		min_value= 10000;

		for (i = 0; i < gQrsPeakNum; i++)
		{
			max_value = gQrsPeakBuff[i].nRRInterval > max_value ? gQrsPeakBuff[i].nRRInterval : max_value;
			min_value = gQrsPeakBuff[i].nRRInterval < min_value ? gQrsPeakBuff[i].nRRInterval : min_value;
			sum_value += gQrsPeakBuff[i].nRRInterval;
		}

		// 去掉最大、最小值，并计算均值
		sum_value -= max_value;
		sum_value -= min_value;
		ave_value = sum_value / (gQrsPeakNum - 2);
	}
	else
	{
		// rr间期的个数小于5，不计算平均rr间期，直接退出
		return 0;
	}*/

	//=========================================================================
	//3. 计算心率
	//=========================================================================
	//3.1 计算平均间期，采用中间的1/3部分计算
	left = rri_num / 3;
	right = rri_num * 2 / 3;
	for (i = left; i <= right; i++)
	{
		ave_rri += total_rri[i];
	}
	ave_rri = ave_rri / (right - left + 1);

	//3.2 换算为BPM
	heart_rate = ECG_SAMPLE_RATE * 60 / ave_rri;

	if (pre_heart_rate && ((pre_heart_rate - heart_rate) > (pre_heart_rate / 10) \
		|| (heart_rate - pre_heart_rate) > pre_heart_rate / 5))
	{
		//10内R波小于8个
		if (rri_num - noise_num < 8)   // 8 -> 3
		{
			invaid_cnt = 0;
			return 0;
		}
		else if (noise_num)			//存在噪声QRS波视为无效条件
		{
			if (++invaid_cnt < 3)	//无效次数未满3个
			{
				return 0;
			}			
		}
	}

	invaid_cnt = 0;

	//3.3 判断是否在计算范围内
	if ((heart_rate >= ECG_HR_MIN_THRESH) && (heart_rate <= ECG_HR_MAX_THRESH))
	{
		pre_heart_rate = heart_rate;
		pre_rri_num = rri_num;

		if (real_output)
		{
			if ((real_output - heart_rate) > 4)
			{
				real_output = real_output - 4;
			}
			else if ((heart_rate - real_output) > 4)
			{
				real_output = real_output + 4;
			}
			else
			{
				real_output = heart_rate;
			}
		}
		else
		{
			real_output = heart_rate;
		}
		return real_output;
	}
	else
	{
		return 9999;
	}
}


// 搜索当前QRS波的IOS点，计算上一个QRS波的ST值
int CalculateStValue(bool reset)
{
	QRS_PEAK curQRS, preQRS;
	LOCAL_PEAK newPeak, maxPeak, secPeak;		//局部搜波得到的峰
	int wind_start = 0, wind_end = 0;							// 搜索窗的起止点位置
	int interval = 0, i = 0, flag = 0, max_dist = 0, sec_dist = 0, slop_cnt = 0, cur_slop = 0;
	int cur_data = 0, last_data = 0, next_data = 0, cur_ptr = 0, last_ptr = 0, next_ptr = 0;
	int qt_dist = 0, t_pos_all = 0, p_pos_all = 0, st_pos_all = 0, ios_pos_all = 0;
	int max_data = 0, max_pos = 0;
	
	if (reset)
	{
		return 1;
	}

	// 刚开始检测到的QRS波可能不准确，不计算ST值
	if (gQrsPeakNum < 4)
	{
		return 0;
	}

	//=========================================================================
	//0. 初始化和出事参数计算
	//=========================================================================
	// Local Peak初始化
	newPeak.nPeakHight = 0;
	newPeak.nPeakPos = 0;
	newPeak.nPeakValue = 0;
	newPeak.nStartPos = 0;
	newPeak.nStartValue = 0;
	maxPeak = newPeak;
	secPeak = newPeak;

	// 获取当前和前一个QRS波
	curQRS  = gQrsPeakBuff[gQrsPeakNum - 1];
	preQRS = gQrsPeakBuff[gQrsPeakNum - 2];

	// 计算两个QRS波的间距，用以确定搜索区域
	interval = curQRS.nPeakPosAll - preQRS.nPeakPosAll;
	if ((interval <= 0) || (interval >= ECG_DATA_BUFF_LEN))
	{
		// 两个QRS的距离超过了数据缓存区的距离，或者位置不对，说明有误，返回
		return 0;
	}

	//=========================================================================
	//1. 定位上一个QRS波的ST点
	//		基本思路
	//		1、在搜索窗内搜波，找出最大、次大两个峰
	//		2、确定T波，如果次大峰与QRS波距离更近，且幅度大于最大峰的2/3，则认为次大峰为T波，否则以最大峰作为T波
	//=========================================================================
	//1.1 确定T波搜索窗的起止点，以QRS波终点为起点，以1/2interval处为终点
	wind_start = mod((preQRS.nEndPos + LOW_PASS_DELAY), ECG_DATA_BUFF_LEN);
	wind_end  = mod((wind_start + interval/2), ECG_DATA_BUFF_LEN);
	wind_end = LoopInc(wind_start,  interval/2, ECG_DATA_BUFF_LEN);

	//1.2 设定初始值，开始搜波
	cur_ptr = wind_start;
	flag = 0;
	if (preQRS.isPositive)
	{
		// 这里主要是为了方便后续搜波，如果是负向QRS波，则对所有数据取负值
		cur_data = gLpDataBuff[wind_start];
	}
	else
	{
		cur_data = -gLpDataBuff[wind_start];
	}
	last_data = cur_data;

	//1.3 开始搜波
	while (cur_ptr != wind_end)
	{
		if (preQRS.isPositive)
		{
			// 这里主要是为了方便后续搜波，如果是负向QRS波，则对所有数据取负值
			cur_data = gLpDataBuff[cur_ptr];
		}
		else
		{
			cur_data = -gLpDataBuff[cur_ptr];
		}

		if (cur_data >= max_data)
		{
			max_data = cur_data;
			max_pos = cur_ptr;
		}

		if (0 == flag)
		{
			if (cur_data > last_data)
			{
				// 如果开始上升，则保存peak起点
				newPeak.nStartValue = last_data;
				newPeak.nStartPos = last_ptr;
				newPeak.nPeakValue = cur_data;
				newPeak.nPeakPos = cur_ptr;
				flag = 1;
			}
			else
			{
				//仍然在下降，啥都不做，继续搜波
			}
		}
		else
		{
			if (cur_data > last_data)
			{
				// 仍然在上升，保持当前状态，根据情况更新peak幅度
				if (cur_data > newPeak.nPeakValue)
				{
					newPeak.nPeakValue = cur_data;
					newPeak.nPeakPos = cur_ptr;
				}
			}
			else
			{
				// 开始下降了，判断是否下降到peak幅度的一半，如果是则确定搜到peak，如果不是就继续搜波
				// -0.5mV ST 模拟器数据测试结果显示，T波没有下降到1/2就结束。为了改善这类情况，将阈值下调为1/4
				if (cur_data < (newPeak.nPeakValue + newPeak.nStartValue) / 4)
				{
					// 已经下降到peak幅度的一半，确定搜波成功，计算peak幅度
					newPeak.nPeakHight = newPeak.nPeakValue - newPeak.nStartValue;

					// 判断这个峰的高度，40LSB对应0.05mV，若小于这个值不可能是T波
					if (newPeak.nPeakHight >= T_PEAK_THRESH)
					{
						// 大于0.05mV，更新最大、次大peak
						if (newPeak.nPeakHight >= maxPeak.nPeakHight)
						{
							// 当前peak幅度大于最大peak幅度，则更新maxPeak和secPeak
							secPeak = maxPeak;
							maxPeak = newPeak;
						}
						else if (newPeak.nPeakHight >= secPeak.nPeakHight)
						{
							// 当前peak幅度大于secPeak，小于maxPeak，则更新secPeak即可
							secPeak = newPeak;
						}
					}
					
					// 清空newPeak，进入下降段
					newPeak.nPeakHight = 0;
					newPeak.nPeakPos = 0;
					newPeak.nPeakValue = 0;
					newPeak.nStartPos = 0;
					newPeak.nStartValue = 0;
					flag = 0;
				}
				else
				{
					//尚未下降到peak幅度的一半，不认为搜到波，继续
				}
			}
		}

		last_data = cur_data;
		last_ptr = cur_ptr;
		cur_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.4 确定T波
	if (0 != (maxPeak.nPeakHight + secPeak.nPeakHight))
	{
		// max和sec中至少有一个不为0，则从maxPeak和secPeak中选定一个作为T波
		// 计算maxPeak和secPeak与QRS波的距离
		if (0 != maxPeak.nPeakHight)
		{
			max_dist = mod((maxPeak.nPeakPos - preQRS.nPeakPos), ECG_DATA_BUFF_LEN);
		}
		else
		{
			max_dist = 0;
		}
		if (0 != secPeak.nPeakHight)
		{
			sec_dist = mod((secPeak.nPeakPos - preQRS.nPeakPos), ECG_DATA_BUFF_LEN);
		}
		else
		{
			sec_dist = 0;
		}

		// 若secPeak距离小于maoPeak，且幅度大于maxPeak的2/3，就以secPeak的波峰为T波波峰
		if ((sec_dist < max_dist) && (secPeak.nPeakHight * 3 > maxPeak.nPeakHight * 2))
		{
			preQRS.nTpeakPos = secPeak.nPeakPos;
		}
		else
		{
			preQRS.nTpeakPos = maxPeak.nPeakPos;
		}
	}
	else
	{
		// 如果没有搜到T波，说明可能是大幅度的S波导致，这时用最大幅度的位置作为T波的位置
		preQRS.nTpeakPos = max_pos;
	}
	t_pos_all = gTotalSampleBuff[preQRS.nTpeakPos] - LOW_PASS_DELAY;

	//1.5  正向查找，以连续三个斜率为0的点作为ST点
	// 更新搜索窗终点
	if (0 != t_pos_all)
	{
		wind_end = preQRS.nTpeakPos;
	}

	cur_ptr = wind_start;
	slop_cnt = 0;
	while ((cur_ptr != wind_end) && (5 > slop_cnt))
	{
		//计算当前点的斜率
		last_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		next_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		cur_slop = Abs(gLpDataBuff[next_ptr] - gLpDataBuff[last_ptr]);

		if (cur_slop <= ST_HORIZON_THRESH)
		{
			//小于0.01mV，认为无波动
			slop_cnt++;
		}
		else
		{
			// 确保连续三点
			slop_cnt = 0;
		}
		cur_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.6 如果找到，赋值。如果没找到，则以T波和终点间1/3的位置作为ST点位置
	if (cur_ptr != wind_end)
	{
		//说明找到，赋值
		preQRS.nStPos = cur_ptr;
	}
	else if (0 != t_pos_all)
	{
		qt_dist = t_pos_all - preQRS.nPeakPosAll;
		if (15 < qt_dist)
		{
			preQRS.nStPos = mod((preQRS.nEndPos + qt_dist/3), ECG_DATA_BUFF_LEN);
		}
		else
		{
			preQRS.nStPos = mod((preQRS.nEndPos + 5), ECG_DATA_BUFF_LEN);
		}
	}
	else
	{
		//没找到平直线，也没找到T波，直接以终点后面第5点作为ST点
		preQRS.nStPos = mod((preQRS.nEndPos + 5), ECG_DATA_BUFF_LEN);
	}
	st_pos_all = gTotalSampleBuff[preQRS.nStPos] - LOW_PASS_DELAY;


	//=========================================================================
	//2. 定位当前QRS波的IOS点
	//=========================================================================
	// Local Peak初始化
	newPeak.nPeakHight = 0;
	newPeak.nPeakPos = 0;
	newPeak.nPeakValue = 0;
	newPeak.nStartPos = 0;
	newPeak.nStartValue = 0;
	maxPeak = newPeak;
	secPeak = newPeak;

	//1.1 确定T波搜索窗的起止点，以QRS波起点为起点，以1/3interval处为终点
	wind_start = mod((curQRS.nStartPos + LOW_PASS_DELAY), ECG_DATA_BUFF_LEN);
	wind_end  = mod((wind_start - interval/2), ECG_DATA_BUFF_LEN);

	//1.2 设定初始值，开始搜波
	cur_ptr = wind_start;
	flag = 0;
	if (curQRS.isPositive)
	{
		// 这里主要是为了方便后续搜波，如果是负向QRS波，则对所有数据取负值
		cur_data = gLpDataBuff[wind_start];
	}
	else
	{
		cur_data = -gLpDataBuff[wind_start];
	}
	last_data = cur_data;

	//1.3 开始搜波
	while (cur_ptr != wind_end)
	{
		if (curQRS.isPositive)
		{
			// 这里主要是为了方便后续搜波，如果是负向QRS波，则对所有数据取负值
			cur_data = gLpDataBuff[cur_ptr];
		}
		else
		{
			cur_data = -gLpDataBuff[cur_ptr];
		}

		if (0 == flag)
		{
			if (cur_data > last_data)
			{
				// 如果开始上升，则保存peak起点
				newPeak.nStartValue = last_data;
				newPeak.nStartPos = last_ptr;
				newPeak.nPeakValue = cur_data;
				newPeak.nPeakPos = cur_ptr;
				flag = 1;
			}
			else
			{
				//仍然在下降，啥都不做，继续搜波
			}
		}
		else
		{
			if (cur_data > last_data)
			{
				// 仍然在上升，保持当前状态，根据情况更新peak幅度
				if (cur_data > newPeak.nPeakValue)
				{
					newPeak.nPeakValue = cur_data;
					newPeak.nPeakPos = cur_ptr;
				}
			}
			else
			{
				// 开始下降了，判断是否下降到peak幅度的一半，如果是则确定搜到peak，如果不是就继续搜波
				if (cur_data < (newPeak.nPeakValue + newPeak.nStartValue) / 4)
				{
					// 已经下降到peak幅度的一半，确定搜波成功，计算peak幅度
					newPeak.nPeakHight = newPeak.nPeakValue - newPeak.nStartValue;

					// 判断这个峰的高度，20LSB对应0.05mV，若小于这个值不可能是T波
					if (newPeak.nPeakHight >= T_PEAK_THRESH)
					{
						// 大于0.05mV，更新最大、次大peak
						if (newPeak.nPeakHight >= maxPeak.nPeakHight)
						{
							// 当前peak幅度大于最大peak幅度，则更新maxPeak和secPeak
							secPeak = maxPeak;
							maxPeak = newPeak;
						}
						else if (newPeak.nPeakHight >= secPeak.nPeakHight)
						{
							// 当前peak幅度大于secPeak，小于maxPeak，则更新secPeak即可
							secPeak = newPeak;
						}
					}

					// 清空newPeak，进入下降段
					newPeak.nPeakHight = 0;
					newPeak.nPeakPos = 0;
					newPeak.nPeakValue = 0;
					newPeak.nStartPos = 0;
					newPeak.nStartValue = 0;
					flag = 0;
				}
				else
				{
					//尚未下降到peak幅度的一半，不认为搜到波，继续
				}
			}
		}

		last_data = cur_data;
		last_ptr = cur_ptr;
		cur_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.4 确定P波
	if (0 != (maxPeak.nPeakHight + secPeak.nPeakHight))
	{
		// max和sec中至少有一个不为0，则从maxPeak和secPeak中选定一个作为P波
		// 计算maxPeak和secPeak与QRS波的距离
		if (0 != maxPeak.nPeakHight)
		{
			max_dist = mod((curQRS.nPeakPos - maxPeak.nPeakPos), ECG_DATA_BUFF_LEN);
		}
		else
		{
			max_dist = 0;
		}
		if (0 != secPeak.nPeakHight)
		{
			sec_dist = mod((curQRS.nPeakPos - secPeak.nPeakPos), ECG_DATA_BUFF_LEN);
		}
		else
		{
			sec_dist = 0;
		}

		// 若secPeak距离小于maoPeak，且幅度大于maxPeak的2/3，就以secPeak的波峰为P波波峰
		if ((sec_dist < max_dist) && (secPeak.nPeakHight * 3 > maxPeak.nPeakHight * 2))
		{
			curQRS.nPpeakPos = secPeak.nPeakPos;
		}
		else
		{
			curQRS.nPpeakPos = maxPeak.nPeakPos;
		}
		p_pos_all = gTotalSampleBuff[curQRS.nPpeakPos] - LOW_PASS_DELAY;
	}
	else
	{
		curQRS.nPpeakPos = 0;
		p_pos_all= 0;
	}

	//1.5  正向查找，以连续5个斜率为0的点作为IOS点
	// 更新搜索窗终点
	if (0 != p_pos_all)
	{
		wind_end = curQRS.nPpeakPos;
	}

	cur_ptr = wind_start;
	slop_cnt = 0;
	while ((cur_ptr != wind_end) && (5 > slop_cnt))
	{
		//计算当前点的斜率
		last_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		next_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		cur_slop = gLpDataBuff[next_ptr] - gLpDataBuff[last_ptr];

		if (cur_slop <= ST_HORIZON_THRESH)
		{
			//小于0.01mV，认为无波动
			slop_cnt++;
		}
		else
		{
			// 确保连续三点
			slop_cnt = 0;
		}
		cur_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.6 如果找到，赋值。如果没找到，则以T波和终点间1/3的位置作为ISO点位置
	if (cur_ptr != wind_end)
	{
		//说明找到，赋值
		curQRS.nIsoPos = cur_ptr;
	}
	else if (0 != p_pos_all)
	{
		qt_dist = curQRS.nPeakPosAll - p_pos_all;

		if (10 < qt_dist)
		{
			curQRS.nIsoPos = mod((curQRS.nStartPos - qt_dist/3), ECG_DATA_BUFF_LEN);
		}
		else
		{
			curQRS.nIsoPos = mod((curQRS.nStartPos - 3), ECG_DATA_BUFF_LEN);
		}
	}
	else
	{
		//没找到平直线，也没找到T波，直接以终点后面第三点作为ST点
		curQRS.nIsoPos = mod((curQRS.nStartPos - 3), ECG_DATA_BUFF_LEN);
	}
	ios_pos_all = gTotalSampleBuff[curQRS.nIsoPos] - LOW_PASS_DELAY;

	//=========================================================================
	//3、计算前一个QRS波的ST值
	//=========================================================================
	if ((0 != ios_pos_all) && (0 != st_pos_all))
	{
		preQRS.nStValue = gLpDataBuff[preQRS.nStPos] - gLpDataBuff[preQRS.nIsoPos];
	}
	else
	{
		preQRS.nStValue = 9999;
	}

	// 这时候的nStPos和nIsoPos都没有减去 LOW_PASS_DELEY，与ViewWave对不上，因此要在这里减去
	preQRS.nStPos = LoopDec(preQRS.nStPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);

	if (9999 == preQRS.nTpeakPos)
	{
		preQRS.nTpeakPos = 9999;
	}
	else
	{
		preQRS.nTpeakPos = LoopDec(preQRS.nTpeakPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);
	}

	curQRS.nIsoPos = LoopDec(curQRS.nIsoPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);

	//=========================================================================
	//4、最后重新填充QRS波缓存区
	//=========================================================================
	gQrsPeakBuff[gQrsPeakNum - 1] = curQRS;
	gQrsPeakBuff[gQrsPeakNum - 2] = preQRS;

	return 1;
}


int CalculateQTValue(bool reset)
{
	int amp_cur = 0, amp_last = 0, amp_next = 0, interval = 0, wind_start = 0, wind_end = 0;
	int cur_ptr =0, last_ptr = 0, next_ptr = 0, cur_slop = 0, max_slop = 0, slop_cnt = 0, tEnd_ptr = 0;
	int max_ptr = 0, min_ptr = 0, max_amp = 0, min_amp = 0;

	QRS_PEAK curQRS, preQRS;

	if (reset)
	{
		return 1;
	}

	// 刚开始检测到的QRS波可能不准确，不计算ST值
	if (gQrsPeakNum < 4)
	{
		return 0;
	}

	// 获取上一个QRS波，并且获取其IOS点，ST点的幅度
	preQRS = gQrsPeakBuff[gQrsPeakNum - 2];
	curQRS  = gQrsPeakBuff[gQrsPeakNum - 1];

	// 设定搜波的起点和终点。以上一个脉搏波的T波峰值点为起点，1/2 RR间期为重点
	// 计算两个QRS波的间距，用以确定搜索区域
	interval = curQRS.nPeakPosAll - preQRS.nPeakPosAll;
	if ((interval <= 0) || (interval >= ECG_DATA_BUFF_LEN))
	{
		// 两个QRS的距离超过了数据缓存区的距离，或者位置不对，说明有误，返回
		preQRS.nTendPos = 9999;
		preQRS.nQtValue = 0;
		return 0;
	}

	//1.1 确定T波搜索窗的起止点，以QRS波终点为起点，以1/2interval处为终点
	if (9999 == preQRS.nTpeakPos)
	{
		// 若没有找到T波，则不计算QT值
		preQRS.nTendPos = 9999;
		preQRS.nQtValue = 0;
		return 0;
	}

	wind_start = LoopInc(preQRS.nTpeakPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);
	wind_end  = LoopInc(preQRS.nPeakPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);

	wind_end  = LoopInc(wind_end, interval*2/3, ECG_DATA_BUFF_LEN);

	//=========================================================================
	// 1. 从T波波峰开始向后索搜，满足其一则结束搜索，在带通波形上进行检测
	//		幅度与ISO和ST的平均幅度相等的点
	//		斜率小于最大斜率的 1/5
	//=========================================================================
	cur_ptr = wind_start;
	amp_cur = gLpDataBuff[cur_ptr];
	max_amp = amp_cur;
	max_ptr = cur_ptr;
	min_amp = amp_cur;
	min_ptr = cur_ptr;

	while (cur_ptr != wind_end)
	{
		// 获取当前点，及前后点的幅度
		last_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		next_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);

		amp_cur = gLpDataBuff[cur_ptr];
		amp_last = gLpDataBuff[last_ptr];
		amp_next = gLpDataBuff[next_ptr];

		//=========================================================================
		// 第一种方法，根据斜率进行判断，连续三点小于最大斜率1/5，则判定为T波终止
		//		这种方法对于心率较低（120及以下）的判断比较准确
		//=========================================================================
		//计算当前点的斜率
		cur_slop = Abs(gLpDataBuff[next_ptr] - gLpDataBuff[last_ptr]);

		// 若有连续三点斜率小于最大斜率的1/5，就判断为终止点
		if (cur_slop > max_slop)
		{
			// 更新最大斜率
			max_slop = cur_slop;
			slop_cnt = 0;
		}
		else if (cur_slop < max_slop/5)
		{
			// 当前斜率小于最大斜率的1/5
			slop_cnt++;
			if (3 <= slop_cnt)
			{
				//tEnd_ptr = LoopDec(cur_ptr, 2, ECG_DATA_BUFF_LEN);
				tEnd_ptr = cur_ptr;
				break;
			}
		}
		else
		{
			// 将斜率计数清零
			slop_cnt = 0;
		}

		//=========================================================================
		// 第二种方法，搜索幅度上升为下降沿1/3的最低点
		//		这种方法适用于心率较高（140及以上）的波形
		//=========================================================================
		if (amp_next <= amp_cur)
		{
			if (amp_next <= min_amp)
			{
				min_amp = amp_next;
				min_ptr = next_ptr;
			}
		}
		else
		{
			// 上升阶段
			if ((amp_next - min_amp) > (max_amp - min_amp) / 3)
			{
				// 上升段的幅度大于下降段幅度的1/3，则取最小值点为T波终点
				tEnd_ptr = min_ptr;
				break;
			}
		}

		cur_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//=========================================================================
	// 如果得到了tEnd，计算QT，并将tEnd填充到preQRS里面
	//=========================================================================
	if (0 != tEnd_ptr)
	{
		//preQRS.nTendPos = LoopDec(tEnd_ptr, BAND_PASS_DELAY, ECG_DATA_BUFF_LEN);

		preQRS.nTendPos = LoopDec(tEnd_ptr, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);
		preQRS.nQtValue = mod((preQRS.nTendPos - preQRS.nStartPos), ECG_DATA_BUFF_LEN);
	}
	else
	{
		preQRS.nTendPos = 9999;
		preQRS.nQtValue = 0;
	}
	
	gQrsPeakBuff[gQrsPeakNum - 2] = preQRS;

	return 1;
}


//=========================================================================
// 计算HRV相关参数
// 这个函数里面计算了 Mean、SDNN、rMSSD，暂时不需要这么多参数，注释掉不用
//=========================================================================

// char CalculateFinalHrv(int rri_num, int *rri_value)
// {
// 	int i = 0, temp = 0, local_sum = 0, pnn_thr = 0;
// 
// 	// 条件判断，平均RR为0，不计算；RR个数小于30，不计算
// 	if ((0 >= gEcgFinalResult.HrvResult.nHrvMean) || (10 >= rri_num))
// 	{
// 		gEcgFinalResult.cHRV = 0;
// 		return 0;
// 	}
// 
// 	//=========================================================================
// 	//1. 计算SDNN，总体标准差
// 	//=========================================================================
// 	// 计算均差平方和
// 	local_sum = 0;
// 	for (i = 0; i < rri_num; i++)
// 	{
// 		temp = rri_value[i] - gEcgFinalResult.HrvResult.nHrvMean;
// 		local_sum += temp * temp;
// 	}
// 
// 	// 求均差平方和的均值及平方根
// 	local_sum = local_sum / rri_num;
// 	gEcgFinalResult.HrvResult.nHrvSdnn = sqrt(float(local_sum));
// 
// 
// 	//=========================================================================
// 	//2. 计算PNN50，相邻间期>50ms的百分比
// 	//=========================================================================
// 	pnn_thr = ECG_SAMPLE_RATE / 50;
// 	local_sum = 0;
// 
// 	// 计算个数
// 	for (i = 0; i < rri_num-1; i++)
// 	{
// 		temp = rri_value[i+1] - rri_value[i];
// 		if (temp > pnn_thr)
// 		{
// 			local_sum++;
// 		}
// 	}
// 
// 	// 计算百分比
// 	gEcgFinalResult.HrvResult.nHrvPnn50 = local_sum * 100 / (rri_num - 1);
// 
// 	//=========================================================================
// 	//3. 条件判断
// 	//=========================================================================
// 	gEcgFinalResult.HrvResult.nHrvSdnn = gEcgFinalResult.HrvResult.nHrvSdnn * 1000 / ECG_SAMPLE_RATE;
// 	gEcgFinalResult.HrvResult.nHrvMean = gEcgFinalResult.HrvResult.nHrvMean * 1000 / ECG_SAMPLE_RATE;
// 	gEcgFinalResult.HrvResult.nHrvRmssd = gEcgFinalResult.HrvResult.nHrvRmssd * 1000 / ECG_SAMPLE_RATE;
// 
// 	return 0;
// }


int CalculateFinalAmp(int *total_amp)
{
	int ave_amp = 0, ave_num = 0, temp = 0, final_amp = 0;
	int i = 0, j = 0, left = 0, right = 0;

	// 计算平均QRS幅度
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nQrsAmp)
		{
			// 初始阶段的amp为0，不参与计算
			total_amp[ave_num] = gQrsPeakBuff[i].nQrsAmp;
			ave_num ++;
		}
	}

	// 对total_rri排序
	if (0 != ave_num)
	{
		for (j = 0; j <= ave_num - 1; j++)
		{
			for(i = 0; i < ave_num - 1 - j; i++) 
			{
				if(total_amp[i] > total_amp[i+1])	
				{
					temp = total_amp[i];
					total_amp[i] = total_amp[i + 1];
					total_amp[i + 1] = temp;
				}	
			}
		}

		// 计算中间1/3的平均RRI
		ave_amp = 0;
		left = ave_num / 3;
		right = ave_num * 2 / 3;
		for (i = left; i <= right; i++)
		{
			ave_amp += total_amp[i];
		}
		ave_amp = ave_amp / (right - left + 1);

		final_amp = ave_amp;
	}
	else
	{
		// 如果参与计算的amp个数为0，说明出错
		final_amp = 0;
	}

	return final_amp;
}

//int CalculateFinalHr0(int rri_num, int *total_rri)
//{
//	int ave_rri = 0, temp = 0, heart_rate = 0, final_hr = 0;
//	int i = 0, j = 0, left = 0, right = 0;

//	// 对total_rri排序
//	if (0 != rri_num)
//	{
//		for (j = 0; j <= rri_num - 1; j++)
//		{
//			for(i = 0; i < rri_num - 1 - j; i++) 
//			{
//				if(total_rri[i] > total_rri[i+1])	
//				{
//					temp = total_rri[i];
//					total_rri[i] = total_rri[i + 1];
//					total_rri[i + 1] = temp;
//				}	
//			}
//		}

//		// 计算中间1/3的平均RRI
//		ave_rri = 0;
//		left = rri_num / 3;
//		right = rri_num * 2 / 3;
//		for (i = left; i <= right; i++)
//		{
//			ave_rri += total_rri[i];
//		}
//		ave_rri = ave_rri / (right - left + 1);

//		// 计算心率
//		heart_rate = ECG_SAMPLE_RATE * 60 / ave_rri;

//		//3.3 判断是否在计算范围内
//		if ((heart_rate >= ECG_HR_MIN_THRESH) && (heart_rate <= ECG_HR_MAX_THRESH))
//		{
//			final_hr = heart_rate;
//		}
//		else
//		{
//			final_hr = 9999;
//		}
//	}
//	else
//	{
//		// 如果参与计算的RRI个数为0，说明出错
//		final_hr = 9999;
//	}

//	// 与初次计算的结果进行比对，判断是否更正
//	if (gbFirstExist)
//	{
//		// 若重算的结果小于初次心率的80%，则以初次计算为准
//		if ((final_hr * 5) < (gEcgFinalFirst.nEcgHeartRate * 4))
//		{
//			final_hr = gEcgFinalFirst.nEcgHeartRate;
//		}

//		// 若重算之后心率无效，则用初次的结果
//		if (9999 == final_hr)
//		{
//			final_hr = gEcgFinalFirst.nEcgHeartRate;
//		}
//	}

//	return final_hr;
//}

int CalculateFinalHr(int rri_num, int *total_rri)
{
	int ave_rri = 0, temp = 0, heart_rate = 0, final_hr = 0;
	int i = 0, j = 0, left = 0, right = 0;

	// 对total_rri排序
	if (0 != rri_num)
	{
		for (j = 0; j <= rri_num - 1; j++)
		{
			for(i = 0; i < rri_num - 1 - j; i++) 
			{
				if(total_rri[i] > total_rri[i+1])	
				{
					temp = total_rri[i];
					total_rri[i] = total_rri[i + 1];
					total_rri[i + 1] = temp;
				}	
			}
		}

		// 计算中间1/3的平均RRI
		ave_rri = 0;
		left = rri_num / 3;
		right = rri_num * 2 / 3;
		for (i = left; i <= right; i++)
		{
			ave_rri += total_rri[i];
		}
		ave_rri = ave_rri / (right - left + 1);

		// 计算心率
		heart_rate = ECG_SAMPLE_RATE * 60 / ave_rri;

		//3.3 判断是否在计算范围内
		if ((heart_rate >= ECG_HR_MIN_THRESH) && (heart_rate <= ECG_HR_MAX_THRESH))
		{
			final_hr = heart_rate;
		}
		else
		{
			final_hr = 9999;
		}
	}
	else
	{
		// 如果参与计算的RRI个数为0，说明出错
		final_hr = 9999;
	}

	// 与初次计算的结果进行比对，判断是否更正
	if (gbFirstExist)
	{
		// 若重算的结果小于初次心率的80%，则以初次计算为准
		if ((final_hr * 5) < (gEcgFinalFirst.nEcgHeartRate * 4))
		{
			final_hr = gEcgFinalFirst.nEcgHeartRate;
		}

		// 若重算之后心率无效，则用初次的结果
		if (9999 == final_hr)
		{
			final_hr = gEcgFinalFirst.nEcgHeartRate;
		}
	}

	return final_hr;
}


// 判断不规则节律，采用归一化 r-MSSD进行计算
int JudgeIrregRhythm(int rri_num, int *rri_value, int *rri_sort)
{
	int i = 0, ave_rri = 0, std_dev = 0, temp_value = 0, irreg_thr = 0, num_thr = 0;
	int cur_rr = 0, sum_rr = 0, cur_num = 0, pre_rr = 0, min_rr = 0, max_rr = 0;
	bool r_mssd = false, max_min_diff = false;

	if (1 >= rri_num)
	{
		return 0;
	}

	//=========================================================================
	//1、 方法1，计算r-MSSD
	//=========================================================================
	// 计算相邻RRI的差值的平方和
	for (i = 0; i < (rri_num - 1); i++)
	{
		temp_value = rri_value[i+1] - rri_value[i];
		std_dev += temp_value * temp_value;

		ave_rri += rri_value[i];
	}

	// 计算平均RRI
	ave_rri = (ave_rri + rri_value[rri_num - 1]) / rri_num;

	// 计算RRI平方和的均方根
	std_dev = std_dev / (rri_num - 1);
	std_dev = sqrt((float)(std_dev));

	// 计算归一化 r-MSSD
	gEcgFinalResult.nIrregCoef = std_dev * 100 / ave_rri;

	// 根据类型制定不同的阈值
	if (gEcgAlgConfig.bJapanVersion)
	{
		irreg_thr = IRREG_THR_JAP;
	}
	else
	{
		irreg_thr = IRREG_THR_NRM;
	}

	// 判断是否满足IRREG条件，大于等于不规则系数阈值
	if (gEcgFinalResult.nIrregCoef >= irreg_thr)
	{
		r_mssd = true;
	}
	else
	{
		r_mssd = false;
	}

	//=========================================================================
	//2、 方法2，计算最大最小的RR间期的差值
	//=========================================================================
	if (gEcgAlgConfig.bJapanVersion)
	{
		// 日本版本比较灵敏，2个相似RR间期即纳入统计
		num_thr = 2;
	}
	else
	{
		num_thr = 3;
	}

	cur_rr = rri_sort[0];
	sum_rr = cur_rr;
	cur_num = 1;
	pre_rr = rri_sort[0] * 115 / 100;		// 设置间期区间阈值为115%

	for (i = 1; i < gQrsPeakNum; i++)
	{
		cur_rr = rri_sort[i];

		if (0 == cur_rr)
		{
			continue;
		}

		if ((cur_rr <= pre_rr) && (pre_rr != 0))
		{
			// 当前的rr不大于前一个rr+10，认为是同一档rr间期，进行累加并计数
			sum_rr += cur_rr;
			cur_num++;
		}
		else
		{
			// 首先计算上一档的平均值
			if (cur_num >= num_thr)
			{
				// 如果这一档的rr个数大于等于3个，就开始更新最大、最小平均间期
				if (0 == min_rr)
				{
					// min只更新一次，确保取得最小的rr档位
					min_rr = sum_rr / cur_num;
				}
				else
				{
					if ((sum_rr / cur_num) >= max_rr)
					{
						max_rr = sum_rr / cur_num;
					}
				}
			}

			// 新的rr分档，更新相关阈值，计数器清零
			pre_rr = cur_rr * 115 / 100;
			sum_rr = cur_rr;
			cur_num = 1;
		}
	}

	// 最后还要再计算一次平均值，因为可能最后一档的数据还未计算
	if (cur_num >= num_thr)
	{
		// 如果这一档的rr个数大于等于3个，就开始更新最大、最小平均间期
		if (0 == min_rr)
		{
			// min只更新一次，确保取得最小的rr档位
			min_rr = sum_rr / cur_num;
		}
		else
		{
			if ((sum_rr / cur_num) >= max_rr)
			{
				max_rr = sum_rr / cur_num;
			}
		}
	}

	// 分析最大最小差异，最小的小于最大的3/4，判断为不规则节律
	if (min_rr < max_rr * 3 / 4)
	{
		max_min_diff = true;
	}
	else
	{
		max_min_diff = false;
	}


	//=========================================================================
	//3、 方法1，方法2，幅度，三者都达到要求，就判断为不规则节律
	//=========================================================================
	if ((gEcgFinalResult.nEcgQrsAmp >= QRS_LOW_AMP_NO_IRREG) && r_mssd && max_min_diff)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//=========================================================================
// 计算HRV相关参数
// 这个函数里面计算了 Mean、SDNN、rMSSD，暂时不需要这么多参数，注释掉不用
//=========================================================================
int CalculateFinalHrv(void)
{
	float temp_hrv = 0, temp = 0;
	int final_hrv = 0, i = 0, pnn_num = 0;
	int mean = 0, sdnn = 0, rmssd = 0, pnn50 = 0;

	//=========================================================================
	// 计算FinalHrv，此处不考虑噪声状况
	//=========================================================================
	if ((10 >= gTotalHrvNum))
	{
		// 如果总体HRV过小，或QRS个数小于等于10个，则不计算HRV
		final_hrv = -1;
	}
	else
	{
		// 计算均方根（注意，该结果单位为ms，放大100倍）
		temp_hrv = gTotalHrvSqr / gTotalHrvNum;
		final_hrv = sqrt(temp_hrv);
	}

	return final_hrv;
}


float CalculateFinalSt(int *st_value, int time_dist)
{
	float ave_st = 0, final_st = 0;
	int ave_num = 0, left = 0, right = 0, temp = 0;
	int i = 0, j = 0;

	if (0 >= gQrsPeakNum)
	{
		// 如果总Peak数为0，说明出错，另外也防止除零
		return 0;
	}
	else
	{
		// 取各个QRS波的ST值，用于排序
		ave_num = 0;
		for (i = 0; i < gQrsPeakNum; i++)
		{
			if ((9999 != gQrsPeakBuff[i].nStValue) && (1 == gQrsPeakBuff[i].nQrsType) && ((gQrsPeakBuff[i].nPeakPosAll + time_dist) >= gEcgTotalSampleNum))
			{
				// 错误及无法计算的ST值定义为9999，不参与计算
				// 只计算正常QRS的ST值，因为PVC一般ST值很高
				// 只统计规定时间内（holter为1分钟，手持机为10分钟）
				st_value[ave_num] = gQrsPeakBuff[i].nStValue;
				ave_num ++;
			}
		}

		// 对total_rri排序
		if (0 != ave_num)
		{
			for (j = 0; j <= ave_num - 1; j++)
			{
				for(i = 0; i < ave_num - 1 - j; i++) 
				{
					if(st_value[i] > st_value[i+1])	
					{
						temp = st_value[i];
						st_value[i] = st_value[i + 1];
						st_value[i + 1] = temp;
					}	
				}
			}

			// 计算中间1/3的平均st值
			ave_st = 0;
			left = ave_num / 3;
			right = ave_num * 2 / 3;
			for (i = left; i <= right; i++)
			{
				ave_st += st_value[i];
			}
			ave_st = ave_st / (right - left + 1);

			// 计算最终的ST值，1LSB=1.25uV，然后换算为mV
			// gEcgFinalResult.nEcgStValue = ave_rri * 10 / 4 / 1000;
			final_st = ave_st * uV_PER_LSB / 1000;
		}
		else
		{
			// 如果参与计算的RRI个数为0，说明出错
			final_st = 9999;
		}
	}

	return final_st;
}


int CalculateFinalQt(int *qt_value, int time_dist)
{
	float ave_qt = 0;
	int ave_num = 0, left = 0, right = 0, temp = 0, final_qt = 0;
	int i = 0, j = 0;

	if (0 >= gQrsPeakNum)
	{
		// 如果总Peak数为0，说明出错，另外也防止除零
		return 0;
	}
	else
	{
		// 取各个QRS波的QT值，用于排序
		ave_num = 0;
		for (i = 0; i < gQrsPeakNum; i++)
		{
			if ((0 != gQrsPeakBuff[i].nQtValue) && (1 == gQrsPeakBuff[i].nQrsType) && (((gQrsPeakBuff[i].nPeakPosAll + time_dist) >= gEcgTotalSampleNum)))
			{
				// 错误及无法计算的QT值定义为0，不参与计算
				// 只计算正常QRS的QT值，因为PVC一般ST值很高
				qt_value[ave_num] = gQrsPeakBuff[i].nQtValue;
				ave_num ++;
			}
		}

		// 对total_rri排序
		if (0 != ave_num)
		{
			for (j = 0; j <= ave_num - 1; j++)
			{
				for(i = 0; i < ave_num - 1 - j; i++) 
				{
					if(qt_value[i] > qt_value[i+1])	
					{
						temp = qt_value[i];
						qt_value[i] = qt_value[i + 1];
						qt_value[i + 1] = temp;
					}	
				}
			}

			// 计算中间1/3的平均QT值
			ave_qt = 0;
			left = ave_num / 3;
			right = ave_num * 2 / 3;
			for (i = left; i <= right; i++)
			{
				ave_qt += qt_value[i];
			}
			ave_qt = ave_qt / (right - left + 1);

			// 计算最终的QT值，单位为ms
			final_qt = ave_qt * 1000 / ECG_SAMPLE_RATE;
		}
		else
		{
			// 如果参与计算的RRI个数为0，说明出错
			final_qt = 9999;
		}
	}

	return final_qt;
}


// 计算最终的QRS宽度
int CalculateFinalWidth(int *total_width, int time_dist)
{
	int wid_num = 0, temp = 0, i = 0, j = 0, left = 0, right = 0;
	float wid_time = 0, ave_wid = 0, final_width = 0;

	if (0 >= gQrsPeakNum)
	{
		// 如果总Peak数为0，说明出错，另外也防止除零
		return 0;
	}
	else
	{
		// 计算平均Qrs宽度
		ave_wid = 0;
		wid_num = 0;
		for (i = 0; i < gQrsPeakNum; i++)
		{
			if (0 != gQrsPeakBuff[i].nQrsWidth && ((gQrsPeakBuff[i].nPeakPosAll + time_dist) >= gEcgTotalSampleNum))
			{
				// 初始阶段的RRI为0，不参与计算
				total_width[wid_num] = gQrsPeakBuff[i].nQrsWidth;
				wid_num ++;
			}
		}

		// 对total_rri排序
		if (0 != wid_num)
		{
			for (j = 0; j <= wid_num - 1; j++)
			{
				for(i = 0; i < wid_num - 1 - j; i++) 
				{
					if(total_width[i] > total_width[i+1])	
					{
						temp = total_width[i];
						total_width[i] = total_width[i + 1];
						total_width[i + 1] = temp;
					}	
				}
			}

			// 计算中间1/3的平均RRI
			ave_wid = 0;
			left = wid_num / 3;
			right = wid_num * 2 / 3;
			for (i = left; i <= right; i++)
			{
				ave_wid += total_width[i];
			}
			ave_wid = ave_wid / (right - left + 1);

			// 计算平均QRS宽度，单位为ms
			final_width = 1000 * ave_wid / ECG_SAMPLE_RATE;

			// 宽度修正，显示滤波的40Hz低通滤波器（SR=500Hz）引入8点展宽（即16ms），需要减去
			final_width = final_width - 16;

			// 宽度修正，根据当前宽度乘一定倍数，实现一定的修正   todo：临时方法，后续优化
			wid_time = 1;
			if (final_width < 40)
			{
				wid_time = 1.8;
			}
			else if (final_width < 60)
			{
				wid_time = 1.7;
			}
			else if (final_width < 80)
			{
				wid_time = 1.6;
			}
			else if (final_width < 100)
			{
				wid_time = 1.5;
			}
			else if (final_width < 120)
			{
				wid_time = 1.4;
			}
			else
			{
				wid_time = 1.3;
			}

			final_width = final_width * wid_time;
		}
		else
		{
			// 如果参与计算的RRI个数为0，说明出错
			final_width = 9999;
		}
	}

	return final_width;
}


// 20秒结束后计算最终的总体结果
int EcgAlgGetFinalResult(ECG_FINAL_RESULT *final_result, bool reset)
{
	float ave_rri = 0;
	int i = 0, j = 0, left = 0, right = 0, temp = 0, ave_amp = 0, sum_rr = 0, heart_rate = 0;
	int min_thresh = 0, max_thresh = 0, rri_num = 0, noise_sig_num = 0;
	int min_rr = 0, max_rr = 0, cur_rr = 0, pre_rr = 0, cur_num = 0, amp_num = 0;
	float wid_time = 0;		// 临时变量，用于修正宽度

	int total_amp[QRS_PEAK_BUFF_LEN], total_rri[QRS_PEAK_BUFF_LEN];
	QRS_PEAK curPeak;

	if (reset)
	{
		gEcgFinalResult.nEcgHeartRate = 9999;
		gEcgFinalResult.nEcgStValue = 9999;
		gEcgFinalResult.nEcgQtValue = 9999;
		gEcgFinalResult.nEcgQTc = 9999;
		gEcgFinalResult.nQTcStatus = 1;
		gEcgFinalResult.nEcgQrsWidth = 9999;
		gEcgFinalResult.nEcgQrsAmp = 0;
		gEcgFinalResult.nPvcNum = 0;
		gEcgFinalResult.nIrregCoef = 0;

		gEcgFinalResult.Bradycardia = false;
		gEcgFinalResult.Tachycardia = false;
		gEcgFinalResult.Irregular = true;
		gEcgFinalResult.HighStValue = false;
		gEcgFinalResult.LowStValue = false;
		gEcgFinalResult.LowQrsAmp = false;
		gEcgFinalResult.WideQrs = false;
		gEcgFinalResult.PvcBeat = false;
		gEcgFinalResult.SignalNoise = 0;

		gEcgFinalResult.nEcgFinalHrv = -1;

		if (!gbFirstExist)
		{
			gEcgFinalFirst.nEcgHeartRate = 9999;
			gEcgFinalFirst.nEcgStValue = 9999;
			gEcgFinalFirst.nEcgQrsWidth = 9999;
			gEcgFinalFirst.nEcgQrsAmp = 0;
			gEcgFinalFirst.nPvcNum = 0;
			gEcgFinalFirst.nIrregCoef = 0;

			gEcgFinalFirst.Bradycardia = false;
			gEcgFinalFirst.Tachycardia = false;
			gEcgFinalFirst.Irregular = true;
			gEcgFinalFirst.HighStValue = false;
			gEcgFinalFirst.LowStValue = false;
			gEcgFinalFirst.LowQrsAmp = false;
			gEcgFinalFirst.WideQrs = false;
			gEcgFinalFirst.PvcBeat = false;
			gEcgFinalFirst.SignalNoise = 0;
			gEcgFinalFirst.nEcgFinalHrv = -1;
		}

		return 1;
	}

	// 最小支持30BPM，20s则有10个QRS波，如果小于5个，就不进行后面的计算
	if (10 >= gQrsPeakNum)
	{
		if (gbFirstExist)
		{
			// 重算没有得到结果，但是初次结果存在，则直接将初次计算的结果赋值
			//*final_result = gEcgFinalFirst;
			//return 1;
			EcgAlgGetFinalResult(0, true);
		}
		else
		{
			// 没有搜索到QRS波，且初次结果也不存在，出错，直接退出
		}
		return 0;
	}

	//=========================================================================
	//0. 判断噪声状态，首先判断噪声，可能对后续的状态设定有帮助
	//=========================================================================
	for (i = 0; i < gEcgNoiseBuffNum; i ++)
	{
		if (gEcgNoiseBuff[i].nNoiseType)
		{
			noise_sig_num++;
		}
	}

	// 噪声数据段，超过总信号的一半，就提示信号差
	if (0 == noise_sig_num)
	{
		gEcgFinalResult.SignalNoise = 0;
	}
	else if (noise_sig_num * 2 >= gEcgNoiseBuffNum)
	{
		gEcgFinalResult.SignalNoise = 2;
	}
	else if (noise_sig_num * 4 >= gEcgNoiseBuffNum)
	{
		gEcgFinalResult.SignalNoise = 1;
	}
	else
	{
		gEcgFinalResult.SignalNoise = 0;
	}

	//=========================================================================
	//1. 计算全部QRS的平均幅度和平均RR间期，然后进行一次排除
	//=========================================================================
	//1.1 将所有幅度和间期全部填入缓存区，准备后续的计算
	for (i = 0; i < gQrsPeakNum; i++)
	{
		total_amp[i] = gQrsPeakBuff[i].nQrsAmp;
		total_rri[i] = gQrsPeakBuff[i].nRRInterval;
	}

	//1.2 冒泡排序，将幅度和间期均按照升序排列
	for (j = 0; j <= gQrsPeakNum - 1; j++)
	{
		for(i = 0; i < gQrsPeakNum - 1 - j; i++) 
		{
			if(total_amp[i] > total_amp[i+1])	
			{
				temp = total_amp[i];
				total_amp[i] = total_amp[i + 1];
				total_amp[i + 1] = temp;
			}	
		}
	}
	for (j = 0; j <= gQrsPeakNum - 1; j++)
	{
		for(i = 0; i < gQrsPeakNum - 1 - j; i++) 
		{
			if(total_rri[i] > total_rri[i+1])	
			{
				temp = total_rri[i];
				total_rri[i] = total_rri[i + 1];
				total_rri[i + 1] = temp;
			}	
		}
	}

	//1.3 计算平均幅度和间期，采用中间向后的1/3部分计算
	// 此处的思想是取较大的1/3来算均值，主要是为了排除一些低幅度的干扰（已更改）
	// 上诉策略可能有大幅度QRS干扰，所以还是取中间1/3
	left = gQrsPeakNum / 3;
	right = gQrsPeakNum * 2 / 3;
	for (i = left; i <= right; i++)
	{
		ave_amp += total_amp[i];
		ave_rri += total_rri[i];
	}
	ave_amp = ave_amp / (right - left + 1);
	ave_rri = ave_rri / (right - left + 1);

	//1.4 根据幅度排除有问题的QRS波
	if ((0 < ave_amp) && (0 < ave_rri))
	{
		for (i = 0; i < gQrsPeakNum; i++)
		{
			curPeak = gQrsPeakBuff[i];
			min_thresh = 0;
			max_thresh = 0;

			// 计算幅度排除阈值
			if (curPeak.nRRInterval * 3 < ave_rri * 2)
			{
				//当rr间期比平均rr间期的2/3还小时，阈值设置更紧一点，要求在平均幅度的1/2与2倍之间
				min_thresh = ave_amp / 2;
				max_thresh = ave_amp * 2;
			}
			else if (curPeak.nRRInterval < ave_rri)
			{
				//rr间期小于平均间期，则放宽阈值，1/3与3倍之间都可以
				min_thresh = ave_amp / 3;
				max_thresh = ave_amp * 3;
			}
			else
			{
				//若此时RR间期已经大于平均间期，则不设置阈值限制，避免长时间搜波失败
				min_thresh = QRS_PEAK_MIN_THRESH;
				max_thresh = QRS_PEAK_MAX_THRESH;
			}

			// 判断幅度是否超出阈值范围，若超出则排除掉，并更新后一个QRS波的RR间期
			if ((curPeak.nQrsAmp < min_thresh) || (curPeak.nQrsAmp > max_thresh))
			{
				// 将排除掉的QRS波的后面一个QRS波的rr间期更新
				if (gQrsPeakNum > (i + 1) && (0 <= (i - 1)))
				{
					// 用后一个Peak的波峰位置减去前一个Peak的波峰位置，更新RR间期
					gQrsPeakBuff[i + 1].nRRInterval = gQrsPeakBuff[i + 1].nPeakPosAll - gQrsPeakBuff[i - 1].nPeakPosAll;
				}

				// 排除当前的QRS波，逐个填充
				for (j = i; j < gQrsPeakNum - 1; j++)
				{
					if (gQrsPeakNum > (j + 1))
					{
						gQrsPeakBuff[j] = gQrsPeakBuff[j + 1];
					}
				}
				gQrsPeakNum--;
				i--;
			}
		}
	}

	// 如果总Peak数为0，说明出错，另外也防止除零
	if (0 >= gQrsPeakNum)
	{
		return 0;
	}
//	int k = 0;
//	for (i = 0; i < gQrsPeakNum; i++)
//	{
//		if (gQrsPeakBuff[i].nPeakPosAll > 10 * 250)
//		{
//			total_rri0[k++] = gQrsPeakBuff[i].nRRInterval;
//		}
//	}
//    gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(k, total_rri0);
	//=========================================================================
	//2. 用更新后的QRS波重新计算平均幅度
	//=========================================================================
	gEcgFinalResult.nEcgQrsAmp = CalculateFinalAmp(total_amp);

	// 判断是否幅度过低
	if (QRS_LOW_AMP_THRESH > gEcgFinalResult.nEcgQrsAmp)
	{
		// 平均幅度小于最小幅度阈值，则报幅度过低
		gEcgFinalResult.LowQrsAmp = true;
	}
	else
	{
		gEcgFinalResult.LowQrsAmp = false;
	}

	//=========================================================================
	// 提取全部的RR间期，以备后用
	//=========================================================================
	ave_rri = 0;
	rri_num = 0;
	amp_num = 0;
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nRRInterval)
		{
			// 初始阶段的RRI为0，不参与计算
			total_rri[rri_num] = gQrsPeakBuff[i].nRRInterval;
			rri_num ++;

			total_amp[amp_num] = gQrsPeakBuff[i].nRRInterval;
			amp_num++;
		}
	}

	//=========================================================================
	//2. 用更新后的RR间期重新计算心率
	//=========================================================================
//	if(rri_num > 15)
//	{
//		gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(rri_num - 15, total_rri);
//	}
//	else 
//	{
		gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(rri_num, total_rri);
//	}
	
	
	

	// 判断心动过速、心动过缓等
	if (9999 == gEcgFinalResult.nEcgHeartRate)
	{
		// 心率为无效值
		gEcgFinalResult.Bradycardia = false;
		gEcgFinalResult.Tachycardia = false;
	}
	else
	{
		if (50 > gEcgFinalResult.nEcgHeartRate)
		{
			gEcgFinalResult.Bradycardia = true;
		}
		else if(100 < gEcgFinalResult.nEcgHeartRate)
		{
			gEcgFinalResult.Tachycardia = true;
		}
		else
		{
			gEcgFinalResult.Bradycardia = false;
			gEcgFinalResult.Tachycardia = false;
		}
	}
	
	//=========================================================================
	//4. 判断不规则节律，若最小的1/2平均RR间期，小于最大的1/2的平均RR间期的一半，则判断为不规则节律
	//		此处，total_amp存储的时没有排序的RRI，total_rri里面存储的是排序后的RRI
	//		该排序在 CalculateFinalHr 里面实现
	//=========================================================================
	if (JudgeIrregRhythm(amp_num, total_amp, total_rri))
	{
		gEcgFinalResult.Irregular = true;
	}
	else
	{
		gEcgFinalResult.Irregular = false;
	}

	//=========================================================================
	//10. HRV计算，Mean和rMSSD是在 JudgeIrregRhythm 中计算的
	//=========================================================================
	gEcgFinalResult.nEcgFinalHrv = CalculateFinalHrv();
	
	//=========================================================================
	//5. 计算ST值，输出单位为uV
	//		这里 total_rri 将被重新填充为 ST 值
	//=========================================================================
	gEcgFinalResult.nEcgStValue = CalculateFinalSt(total_rri, 600*ECG_SAMPLE_RATE);

	// 判断ST过高、过低
	if (0.2 <= gEcgFinalResult.nEcgStValue)
	{
		gEcgFinalResult.HighStValue = true;
	}
	else if (-0.2 >= gEcgFinalResult.nEcgStValue)
	{
		gEcgFinalResult.LowStValue = true;
	}
	else
	{
		gEcgFinalResult.HighStValue = false;
		gEcgFinalResult.LowStValue = false;
	}

	//=========================================================================
	//6. 计算QT值，输出单位为ms
	//		这里 total_rri 将被重新填充为 QT 值
	//=========================================================================
	gEcgFinalResult.nEcgQtValue = CalculateFinalQt(total_rri, 600*ECG_SAMPLE_RATE);

	// 超出范围则无效
	if ((gEcgFinalResult.nEcgHeartRate > 200) || (gEcgFinalResult.nEcgHeartRate < 30))
	{
		gEcgFinalResult.nEcgQtValue = 9999;
	}

	// 计算QTc
	if (9999 == gEcgFinalResult.nEcgQtValue)
	{
		gEcgFinalResult.nEcgQTc = 9999;
	}
	else
	{
		gEcgFinalResult.nEcgQTc = (float)(gEcgFinalResult.nEcgQtValue) / sqrt((60 / (float)(gEcgFinalResult.nEcgHeartRate)));
	}

	// 判断延长或缩短
	if (9999 == gEcgFinalResult.nEcgQTc)
	{
		gEcgFinalResult.nQTcStatus = 1;
	}
	else
	{
		if (SHORT_QTC_THRESH > gEcgFinalResult.nEcgQTc)
		{
			gEcgFinalResult.nQTcStatus = 0;
		}
		else if (LONG_QTC_THRESH < gEcgFinalResult.nEcgQTc)
		{
			gEcgFinalResult.nQTcStatus = 2;
		}
		else
		{
			gEcgFinalResult.nQTcStatus = 1;
		}
	}


	//=========================================================================
	//7. 计算平均QRS宽度，并判断QRS过宽
	//		这里 total_rri 将被重新填充为 QRS width
	//=========================================================================
	gEcgFinalResult.nEcgQrsWidth = CalculateFinalWidth(total_rri, 600*ECG_SAMPLE_RATE);

	// 判断宽度过宽
	if (120 < gEcgFinalResult.nEcgQrsWidth)
	{
		gEcgFinalResult.WideQrs = true;
	}
	else
	{
		gEcgFinalResult.WideQrs = false;
	}

	//=========================================================================
	//8. 输出PVC的个数，以及判断结果
	//=========================================================================
	gEcgFinalResult.nPvcNum = gPvcNumAll;

	if ((0 < gEcgFinalResult.nPvcNum) && (QRS_LOW_AMP_NO_PVC < gEcgFinalResult.nEcgQrsAmp))
	{
		gEcgFinalResult.PvcBeat = true;
	}
	else
	{
		gEcgFinalResult.PvcBeat = false;
		gEcgFinalResult.nPvcNum = 0;
	}
	
//	// 噪声判断
//	if (gEcgAlgResult.isAsystole != 1) {
//		if (9999 == gEcgFinalResult.nEcgQTc || -1 == gEcgFinalResult.nEcgFinalHrv)
//		{
//			gEcgFinalResult.SignalNoise = 2;
//		}
//	}

	//=========================================================================
	// 判断初次结果是否存在，若不存在则进行存储
	//=========================================================================
	if (!gbFirstExist)
	{
		// 不存在，给初次结果赋值
		gEcgFinalFirst = gEcgFinalResult;
		gbFirstExist = true;
	}

	//=========================================================================
	// 输出最后的结果
	//=========================================================================
	*final_result = gEcgFinalResult;
	return 1;
}


//=========================================================================
// 获取所有QRS波波峰位置，提供给PWTT，ECG单参数测量时不需要调用该函数
//=========================================================================
int EcgAlgGetQrsPeakPos(int *qrs_pos_buff)
{
	int i = 0, delay = 0;

	/*
	if (1 == gEcgAlgConfig.cViewMode)
	{
		// 模式一，0.05 - 40Hz
		delay = DELAY_VIEW_LP40;
	}
	else if (2 == gEcgAlgConfig.cViewMode)
	{
		// 模式二， 0.5 - 20Hz
		delay = DELAY_VIEW_HP05 + DELAY_VIEW_LP40;
	}
	else
	{
		// 默认模式二， 0.5 - 20Hz
		delay = DELAY_VIEW_HP05 + DELAY_VIEW_LP40;
	}
	*/

	for (i = 0; i < gQrsPeakNum; i++)
	{
		// 换算为1000Hz采样率
		qrs_pos_buff[i] = gQrsPeakBuff[i].nPeakPosAll * 4 - delay * 2;
	}

	return gQrsPeakNum;
}


int EcgAlgCalculateHolterHRV(unsigned short curRRI)
{
	static unsigned short last_rri = 0;
	float temp = 0;
	int ratio = 0;


	// 计算采样倍数
	ratio = 1000 / ECG_SAMPLE_RATE;

	if ((0 == curRRI) || (curRRI * 10 < gEcgHolterResult.nAveRRinterval) || (curRRI > gEcgHolterResult.nAveRRinterval * 5))
	{
		// 剔除条件：RRI为0，RRI大于平均RRI的5倍，RRI小于平均RRI的1/10
	}
	else
	{
		// 2. 计算总体标准差SDNN，单位为采样点
		temp = curRRI - gEcgHolterResult.nAveRRinterval;
		gEcgHolterResult.nHolterSdnn += temp * temp;

		// 3. 计算差值均方根rMSSD，单位为采样点
		if (0 != last_rri)
		{
			temp = curRRI - last_rri;
			gEcgHolterResult.nHolterRmssd += temp * temp;

			// 3. 计算大于50ms的比例
			if (temp * ratio > 50)
			{
				gEcgHolterResult.nPnn50Num++;
			}
		}

		gEcgHolterResult.nRRintervalNum++;
		last_rri = curRRI;
	}

	return 1;
}



// Holter 1分钟结果
int EcgAlgGetHolterOneMinResult(ECG_FINAL_RESULT *final_result, bool reset)
{
	float ave_rri = 0;
	int i = 0, j = 0, left = 0, right = 0, temp = 0, ave_amp = 0, sum_rr = 0, heart_rate = 0;
	int min_thresh = 0, max_thresh = 0, rri_num = 0, noise_sig_num = 0, peak_num = 0;
	int min_rr = 0, max_rr = 0, cur_rr = 0, pre_rr = 0, cur_num = 0, amp_num = 0;
	float wid_time = 0;		// 临时变量，用于修正宽度

	int total_amp[QRS_PEAK_BUFF_LEN], total_rri[QRS_PEAK_BUFF_LEN];
	QRS_PEAK curPeak;

	if (reset)
	{
		gEcgFinalResult.nEcgHeartRate = 9999;
		gEcgFinalResult.nEcgStValue = 9999;
		gEcgFinalResult.nEcgQtValue = 9999;
		gEcgFinalResult.nEcgQTc = 9999;
		gEcgFinalResult.nQTcStatus = 1;
		gEcgFinalResult.nEcgQrsWidth = 9999;
		gEcgFinalResult.nEcgQrsAmp = 0;
		gEcgFinalResult.nPvcNum = 0;
		gEcgFinalResult.nIrregCoef = 0;

		gEcgFinalResult.Bradycardia = false;
		gEcgFinalResult.Tachycardia = false;
		gEcgFinalResult.Irregular = true;
		gEcgFinalResult.HighStValue = false;
		gEcgFinalResult.LowStValue = false;
		gEcgFinalResult.LowQrsAmp = false;
		gEcgFinalResult.WideQrs = false;
		gEcgFinalResult.PvcBeat = false;
		gEcgFinalResult.PauseBeat = false;
		gEcgFinalResult.nPausePerMin = 0;
		gEcgFinalResult.SignalNoise = 0;

		gEcgFinalResult.nEcgFinalHrv = -1;
		return 1;
	}

	EcgAlgGetHolterOneMinResult(0, true);

	// 最小支持30BPM，20s则有10个QRS波，如果小于5个，就不进行后面的计算
	if (5 >= gQrsPeakNum)
	{
		// 没有搜索到QRS波，且初次结果也不存在，出错，直接退出
		*final_result = gEcgFinalResult;
		return 0;
	}

	//=========================================================================
	//0. 判断噪声状态，首先判断噪声，可能对后续的状态设定有帮助
	//=========================================================================
	for (i = 0; i < gEcgNoiseBuffNum; i ++)
	{
		if (gEcgNoiseBuff[i].nNoiseType)
		{
			noise_sig_num++;
		}
	}

	// 噪声数据段，超过总信号的一半，就提示信号差
	if (0 == noise_sig_num)
	{
		gEcgFinalResult.SignalNoise = 0;
	}
	else if (noise_sig_num * 2 >= gEcgNoiseBuffNum)
	{
		gEcgFinalResult.SignalNoise = 2;
	}
	else if (noise_sig_num * 4 >= gEcgNoiseBuffNum)
	{
		gEcgFinalResult.SignalNoise = 1;
	}
	else
	{
		gEcgFinalResult.SignalNoise = 0;
	}

	// 计算下一分钟的噪声情况，重新计数
	gEcgNoiseBuffNum = 0;

	//=========================================================================
	//1. 计算全部QRS的平均幅度和平均RR间期，然后进行一次排除
	//=========================================================================
	//1.1 将所有幅度和间期全部填入缓存区，准备后续的计算
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if ((gQrsPeakBuff[i].nPeakPosAll + ECG_SAMPLE_RATE * 60) > gEcgTotalSampleNum)
		{
			// 只统计1分钟内的QRS波的相关情况
			total_amp[peak_num] = gQrsPeakBuff[peak_num].nQrsAmp;
			total_rri[peak_num] = gQrsPeakBuff[peak_num].nRRInterval;
			peak_num++;
		}
	}

	if (5 >= peak_num)
	{
		// 如果1分钟内的QRS波个数少于5个，则报错
		*final_result = gEcgFinalResult;
		return 0;
	}

	//1.2 冒泡排序，将幅度和间期均按照升序排列
	for (j = 0; j <= peak_num - 1; j++)
	{
		for(i = 0; i < peak_num - 1 - j; i++) 
		{
			if(total_amp[i] > total_amp[i+1])	
			{
				temp = total_amp[i];
				total_amp[i] = total_amp[i + 1];
				total_amp[i + 1] = temp;
			}	
		}
	}
	for (j = 0; j <= peak_num - 1; j++)
	{
		for(i = 0; i < peak_num - 1 - j; i++) 
		{
			if(total_rri[i] > total_rri[i+1])	
			{
				temp = total_rri[i];
				total_rri[i] = total_rri[i + 1];
				total_rri[i + 1] = temp;
			}	
		}
	}

	//1.3 计算平均幅度和间期，采用中间向后的1/3部分计算
	// 此处的思想是取较大的1/3来算均值，主要是为了排除一些低幅度的干扰（已更改）
	// 上诉策略可能有大幅度QRS干扰，所以还是取中间1/3
	left = peak_num / 3;
	right = peak_num * 2 / 3;
	for (i = left; i <= right; i++)
	{
		ave_amp += total_amp[i];
		ave_rri += total_rri[i];
	}
	ave_amp = ave_amp / (right - left + 1);
	ave_rri = ave_rri / (right - left + 1);

	//=========================================================================
	//2. 用更新后的QRS波重新计算平均幅度
	//=========================================================================
	gEcgFinalResult.nEcgQrsAmp = ave_amp;

	// 判断是否幅度过低
	if (QRS_LOW_AMP_THRESH > gEcgFinalResult.nEcgQrsAmp)
	{
		// 平均幅度小于最小幅度阈值，则报幅度过低
		gEcgFinalResult.LowQrsAmp = true;
	}
	else
	{
		gEcgFinalResult.LowQrsAmp = false;
	}

	//=========================================================================
	// 提取全部的RR间期，以备后用
	//=========================================================================
	ave_rri = 0;
	rri_num = 0;
	amp_num = 0;
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nRRInterval && ((gQrsPeakBuff[i].nPeakPosAll + ECG_SAMPLE_RATE * 60) > gEcgTotalSampleNum))
		{
			// 只计算1分钟以内的RRI
			// 初始阶段的RRI为0，不参与计算
			total_rri[rri_num] = gQrsPeakBuff[i].nRRInterval;
			rri_num ++;

			total_amp[amp_num] = gQrsPeakBuff[i].nRRInterval;
			amp_num++;
		}
	}

	//=========================================================================
	//2. 用更新后的RR间期重新计算心率
	//=========================================================================
	gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(rri_num, total_rri);

	// 判断心动过速、心动过缓等
	gEcgFinalResult.Bradycardia = false;
	gEcgFinalResult.Tachycardia = false;
	if (9999 == gEcgFinalResult.nEcgHeartRate)
	{
		// 心率为无效值
		gEcgFinalResult.Bradycardia = false;
		gEcgFinalResult.Tachycardia = false;
	}
	else if (50 > gEcgFinalResult.nEcgHeartRate)
	{
		gEcgFinalResult.Bradycardia = true;
	}
	else if(100 < gEcgFinalResult.nEcgHeartRate)
	{
		gEcgFinalResult.Tachycardia = true;
	}

	//=========================================================================
	//4. 判断不规则节律，若最小的1/2平均RR间期，小于最大的1/2的平均RR间期的一半，则判断为不规则节律
	//		此处，total_amp存储的时没有排序的RRI，total_rri里面存储的是排序后的RRI
	//		该排序在 CalculateFinalHr 里面实现
	//=========================================================================
	if (JudgeIrregRhythm(amp_num, total_amp, total_rri))
	{
		gEcgFinalResult.Irregular = true;
	}
	else
	{
		gEcgFinalResult.Irregular = false;
	}

	//=========================================================================
	//10. HRV计算，Mean和rMSSD是在 JudgeIrregRhythm 中计算的
	//=========================================================================
	gEcgFinalResult.nEcgFinalHrv = CalculateFinalHrv();

	//=========================================================================
	//5. 计算ST值，输出单位为uV
	//		这里 total_rri 将被重新填充为 ST 值
	//=========================================================================
	gEcgFinalResult.nEcgStValue = CalculateFinalSt(total_rri, 60 * ECG_SAMPLE_RATE);

	// 判断ST过高、过低
	gEcgFinalResult.HighStValue = false;
	gEcgFinalResult.LowStValue = false;
	if (0.2 <= gEcgFinalResult.nEcgStValue)
	{
		gEcgFinalResult.HighStValue = true;
	}
	else if (-0.2 >= gEcgFinalResult.nEcgStValue)
	{
		gEcgFinalResult.LowStValue = true;
	}

	//=========================================================================
	//6. 计算QT值，输出单位为ms
	//		这里 total_rri 将被重新填充为 QT 值
	//=========================================================================
	gEcgFinalResult.nEcgQtValue = CalculateFinalQt(total_rri, 60 * ECG_SAMPLE_RATE);

	// 超出范围则无效
	if ((gEcgFinalResult.nEcgHeartRate > 200) || (gEcgFinalResult.nEcgHeartRate < 30))
	{
		gEcgFinalResult.nEcgQtValue = 9999;
	}

	// 计算QTc
	if (9999 == gEcgFinalResult.nEcgQtValue)
	{
		gEcgFinalResult.nEcgQTc = 9999;
	}
	else
	{
		gEcgFinalResult.nEcgQTc = (float)(gEcgFinalResult.nEcgQtValue) / sqrt((60 / (float)(gEcgFinalResult.nEcgHeartRate)));
	}

	// 判断延长或缩短
	if (9999 == gEcgFinalResult.nEcgQTc)
	{
		gEcgFinalResult.nQTcStatus = 1;
	}
	else
	{
		if (SHORT_QTC_THRESH > gEcgFinalResult.nEcgQTc)
		{
			gEcgFinalResult.nQTcStatus = 0;
		}
		else if (LONG_QTC_THRESH < gEcgFinalResult.nEcgQTc)
		{
			gEcgFinalResult.nQTcStatus = 2;
		}
		else
		{
			gEcgFinalResult.nQTcStatus = 1;
		}
	}


	//=========================================================================
	//7. 计算平均QRS宽度，并判断QRS过宽
	//		这里 total_rri 将被重新填充为 QRS width
	//=========================================================================
	gEcgFinalResult.nEcgQrsWidth = CalculateFinalWidth(total_rri, 60 * ECG_SAMPLE_RATE);

	// 判断宽度过宽
	if (120 < gEcgFinalResult.nEcgQrsWidth)
	{
		gEcgFinalResult.WideQrs = true;
	}
	else
	{
		gEcgFinalResult.WideQrs = false;
	}

	//=========================================================================
	//8. 输出PVC的个数，以及判断结果
	//=========================================================================
	if (gPvcNumAll >= gPvcNumAllLast)
	{
		gEcgFinalResult.nPvcNum = gPvcNumAll - gPvcNumAllLast;
		gPvcNumAllLast = gPvcNumAll;
	}
	else
	{
		gEcgFinalResult.nPvcNum = 0;
	}

	if ((0 < gEcgFinalResult.nPvcNum) && (QRS_LOW_AMP_NO_PVC < gEcgFinalResult.nEcgQrsAmp))
	{
		gEcgFinalResult.PvcBeat = true;
	}
	else
	{
		gEcgFinalResult.PvcBeat = false;
		gEcgFinalResult.nPvcNum = 0;
	}

	//=========================================================================
	// 判断是否存在心跳暂停
	//=========================================================================
	if (gEcgPauseNum > 0)
	{
		gEcgFinalResult.PauseBeat = true;
		gEcgFinalResult.nPausePerMin = gEcgPauseNum;
	}
	else
	{
		gEcgFinalResult.PauseBeat = false;
		gEcgFinalResult.nPausePerMin = 0;
	}
	gEcgHolterResult.nPauseNum = gEcgHolterResult.nPauseNum + gEcgPauseNum;
	gEcgPauseNum = 0;

	//=========================================================================
	//9. 计算holter结果，每1分钟计算一次
	//=========================================================================
	// 1. 计算心电数据的片段
	if (9999 == gEcgFinalResult.nEcgHeartRate)
	{
		// 9999为无效心率，不纳入计算
	}
	else if (2 == gEcgFinalResult.SignalNoise)
	{
		// 大噪声情况下心率无效，不纳入计算
	}
	else
	{
		gEcgHolterResult.nTotalRhythmNum++;

		// 2. 更新最大、最小心率
		if (0 == gEcgHolterResult.nMaxHeartRate)
		{
			gEcgHolterResult.nMaxHeartRate = gEcgFinalResult.nEcgHeartRate;
			gEcgHolterResult.nMinHeartRate = gEcgFinalResult.nEcgHeartRate;
		}
		else if (gEcgFinalResult.nEcgHeartRate > gEcgHolterResult.nMaxHeartRate)
		{
			gEcgHolterResult.nMaxHeartRate = gEcgFinalResult.nEcgHeartRate;
		}
		else if (gEcgFinalResult.nEcgHeartRate < gEcgHolterResult.nMinHeartRate)
		{
			gEcgHolterResult.nMinHeartRate = gEcgFinalResult.nEcgHeartRate;
		}

		// 3. 计算平均心率
		gEcgHolterResult.nSumHeartRate += gEcgFinalResult.nEcgHeartRate;
		gEcgHolterResult.nAveHeartRate = gEcgHolterResult.nSumHeartRate / gEcgHolterResult.nTotalRhythmNum;

		// 计算平均RRI
		gEcgHolterResult.nAveRRinterval = ECG_SAMPLE_RATE * 60 / gEcgHolterResult.nAveHeartRate;
	}
	

	//=========================================================================
	// 输出最后的结果
	//=========================================================================
	*final_result = gEcgFinalResult;
	return 1;
}



int EcgAlgGetHolterFinalResult(ECG_HOLTER_RESULT *final_result, bool reset)
{
	float temp = 0;
	int ratio = 0;

	if (reset)
	{
		gEcgHolterResult.nAveHeartRate = 0;
		gEcgHolterResult.nSumHeartRate = 0;
		gEcgHolterResult.nAveRRinterval = 0;
		gEcgHolterResult.nMaxHeartRate = 0;
		gEcgHolterResult.nMinHeartRate = 0;
		gEcgHolterResult.nTotalRhythmNum = 0;
		gEcgHolterResult.nHolterSdnn = 0;
		gEcgHolterResult.nHolterRmssd = 0;
		gEcgHolterResult.nHolterPnn50 = 0;
		gEcgHolterResult.nRRintervalNum = 0;
		gEcgHolterResult.nPnn50Num = 0;
		gEcgHolterResult.nPauseNum = 0;

		return 0;
	}

	// 首先判断数据是否有效，QRS个数小于5个，则说明数据有问题
	if (5 >= gQrsPeakNum)
	{
		*final_result = gEcgHolterResult;
		return 0;
	}

	ratio = 1000 / ECG_SAMPLE_RATE;

	// 计算相关的HRV参数，防止除零
	if (2 < gEcgHolterResult.nRRintervalNum)
	{
		// 2. 计算总体标准差SDNN
		temp = gEcgHolterResult.nHolterSdnn * ratio * ratio / gEcgHolterResult.nRRintervalNum;
		gEcgHolterResult.nHolterSdnn = sqrt(temp);

		// 3. 计算差值均方根rMSSD
		temp = gEcgHolterResult.nHolterRmssd * ratio * ratio / (gEcgHolterResult.nRRintervalNum - 1);
		gEcgHolterResult.nHolterRmssd = sqrt(temp);

		// 4. 计算相邻间期差值>50ms的百分比 PNN50
		gEcgHolterResult.nHolterPnn50 = gEcgHolterResult.nPnn50Num * 100 / (gEcgHolterResult.nRRintervalNum - 1);
	}
	else
	{
		gEcgHolterResult.nHolterSdnn = 0;
		gEcgHolterResult.nHolterRmssd = 0;
		gEcgHolterResult.nHolterPnn50 = 0;
	}
	
	*final_result = gEcgHolterResult;

	return 1;
}
