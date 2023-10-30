//*************************************************************************
//�ļ����ƣ�EcgResult.cpp
//
//�ļ�˵����Ecg�������㺯��
//
//������ʷ��Created by Chenmaomao [5 / 15 / 2014]
//*************************************************************************

//#include "stdafx.h"
#include "EcgResult.h"
#include <math.h>


// ��ʼ������
void EcgResultInitialize(void)
{
	CalculateHeartRate(true);
	CalculateStValue(true);

	// ���շ��������ʼ��
	EcgAlgGetFinalResult(0, true);

	// Holter�����ʼ��
	EcgAlgGetHolterOneMinResult(0, true);
	EcgAlgGetHolterFinalResult(0, true);
}


// ��������
int CalculateHeartRate(bool reset)
{
	int heart_rate = 0, i = 0, j = 0, interval = 0, cur_pos = 0, final_pos = 0, dist_pos = 0, temp = 0;
	int max_value = 0, min_value = 0, sum_value = 0, ave_value = 0, left = 0, right = 0, ave_rri = 0;

	unsigned short total_rri[50];						// ֧�����300BPM��10������50��QRS��
	unsigned short rri_num = 0;
	unsigned short rri_dist = ECG_SAMPLE_RATE * 10;		// ����HR�����QRS����Χ
	unsigned char noise_num = 0;				//����QRS����

	static int real_output = 0;		//ʵ�����
	static int pre_heart_rate = 0;		//ǰһ����Ч���hr
	static int pre_rri_num = 0;			//ǰһ����Ч���ڼ�⵽R����
	static unsigned char invaid_cnt = 0;	//��Ч����

	if (5 > gQrsPeakNum)  // 5 -> 3
	{
		return 0;
	}

	//=========================================================================
	// 1.���Ƚ�10s�ڵ�QRS����RR���ڣ���䵽rri������
	//=========================================================================
	// �����һ��QRS���Ĳ���λ��
	final_pos = gQrsPeakBuff[gQrsPeakNum - 1].nPeakPosAll;

	// ���10���ڵ�QRS��
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
	// 2.ð�����򣬽����ڰ�����������
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
		// ԭ���Ĵ���
		//=========================================================================
		// ��rr���ڵ���������5����ʼ�������ʣ�Ŀ���Ǳ�����ڵĸ��ŵ������ʼ������
		max_value = 0;
		min_value= 10000;

		for (i = 0; i < gQrsPeakNum; i++)
		{
			max_value = gQrsPeakBuff[i].nRRInterval > max_value ? gQrsPeakBuff[i].nRRInterval : max_value;
			min_value = gQrsPeakBuff[i].nRRInterval < min_value ? gQrsPeakBuff[i].nRRInterval : min_value;
			sum_value += gQrsPeakBuff[i].nRRInterval;
		}

		// ȥ�������Сֵ���������ֵ
		sum_value -= max_value;
		sum_value -= min_value;
		ave_value = sum_value / (gQrsPeakNum - 2);
	}
	else
	{
		// rr���ڵĸ���С��5��������ƽ��rr���ڣ�ֱ���˳�
		return 0;
	}*/

	//=========================================================================
	//3. ��������
	//=========================================================================
	//3.1 ����ƽ�����ڣ������м��1/3���ּ���
	left = rri_num / 3;
	right = rri_num * 2 / 3;
	for (i = left; i <= right; i++)
	{
		ave_rri += total_rri[i];
	}
	ave_rri = ave_rri / (right - left + 1);

	//3.2 ����ΪBPM
	heart_rate = ECG_SAMPLE_RATE * 60 / ave_rri;

	if (pre_heart_rate && ((pre_heart_rate - heart_rate) > (pre_heart_rate / 10) \
		|| (heart_rate - pre_heart_rate) > pre_heart_rate / 5))
	{
		//10��R��С��8��
		if (rri_num - noise_num < 8)   // 8 -> 3
		{
			invaid_cnt = 0;
			return 0;
		}
		else if (noise_num)			//��������QRS����Ϊ��Ч����
		{
			if (++invaid_cnt < 3)	//��Ч����δ��3��
			{
				return 0;
			}			
		}
	}

	invaid_cnt = 0;

	//3.3 �ж��Ƿ��ڼ��㷶Χ��
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


// ������ǰQRS����IOS�㣬������һ��QRS����STֵ
int CalculateStValue(bool reset)
{
	QRS_PEAK curQRS, preQRS;
	LOCAL_PEAK newPeak, maxPeak, secPeak;		//�ֲ��Ѳ��õ��ķ�
	int wind_start = 0, wind_end = 0;							// ����������ֹ��λ��
	int interval = 0, i = 0, flag = 0, max_dist = 0, sec_dist = 0, slop_cnt = 0, cur_slop = 0;
	int cur_data = 0, last_data = 0, next_data = 0, cur_ptr = 0, last_ptr = 0, next_ptr = 0;
	int qt_dist = 0, t_pos_all = 0, p_pos_all = 0, st_pos_all = 0, ios_pos_all = 0;
	int max_data = 0, max_pos = 0;
	
	if (reset)
	{
		return 1;
	}

	// �տ�ʼ��⵽��QRS�����ܲ�׼ȷ��������STֵ
	if (gQrsPeakNum < 4)
	{
		return 0;
	}

	//=========================================================================
	//0. ��ʼ���ͳ��²�������
	//=========================================================================
	// Local Peak��ʼ��
	newPeak.nPeakHight = 0;
	newPeak.nPeakPos = 0;
	newPeak.nPeakValue = 0;
	newPeak.nStartPos = 0;
	newPeak.nStartValue = 0;
	maxPeak = newPeak;
	secPeak = newPeak;

	// ��ȡ��ǰ��ǰһ��QRS��
	curQRS  = gQrsPeakBuff[gQrsPeakNum - 1];
	preQRS = gQrsPeakBuff[gQrsPeakNum - 2];

	// ��������QRS���ļ�࣬����ȷ����������
	interval = curQRS.nPeakPosAll - preQRS.nPeakPosAll;
	if ((interval <= 0) || (interval >= ECG_DATA_BUFF_LEN))
	{
		// ����QRS�ľ��볬�������ݻ������ľ��룬����λ�ò��ԣ�˵�����󣬷���
		return 0;
	}

	//=========================================================================
	//1. ��λ��һ��QRS����ST��
	//		����˼·
	//		1�������������Ѳ����ҳ���󡢴δ�������
	//		2��ȷ��T��������δ����QRS������������ҷ��ȴ��������2/3������Ϊ�δ��ΪT����������������ΪT��
	//=========================================================================
	//1.1 ȷ��T������������ֹ�㣬��QRS���յ�Ϊ��㣬��1/2interval��Ϊ�յ�
	wind_start = mod((preQRS.nEndPos + LOW_PASS_DELAY), ECG_DATA_BUFF_LEN);
	wind_end  = mod((wind_start + interval/2), ECG_DATA_BUFF_LEN);
	wind_end = LoopInc(wind_start,  interval/2, ECG_DATA_BUFF_LEN);

	//1.2 �趨��ʼֵ����ʼ�Ѳ�
	cur_ptr = wind_start;
	flag = 0;
	if (preQRS.isPositive)
	{
		// ������Ҫ��Ϊ�˷�������Ѳ�������Ǹ���QRS���������������ȡ��ֵ
		cur_data = gLpDataBuff[wind_start];
	}
	else
	{
		cur_data = -gLpDataBuff[wind_start];
	}
	last_data = cur_data;

	//1.3 ��ʼ�Ѳ�
	while (cur_ptr != wind_end)
	{
		if (preQRS.isPositive)
		{
			// ������Ҫ��Ϊ�˷�������Ѳ�������Ǹ���QRS���������������ȡ��ֵ
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
				// �����ʼ�������򱣴�peak���
				newPeak.nStartValue = last_data;
				newPeak.nStartPos = last_ptr;
				newPeak.nPeakValue = cur_data;
				newPeak.nPeakPos = cur_ptr;
				flag = 1;
			}
			else
			{
				//��Ȼ���½���ɶ�������������Ѳ�
			}
		}
		else
		{
			if (cur_data > last_data)
			{
				// ��Ȼ�����������ֵ�ǰ״̬�������������peak����
				if (cur_data > newPeak.nPeakValue)
				{
					newPeak.nPeakValue = cur_data;
					newPeak.nPeakPos = cur_ptr;
				}
			}
			else
			{
				// ��ʼ�½��ˣ��ж��Ƿ��½���peak���ȵ�һ�룬�������ȷ���ѵ�peak��������Ǿͼ����Ѳ�
				// -0.5mV ST ģ�������ݲ��Խ����ʾ��T��û���½���1/2�ͽ�����Ϊ�˸����������������ֵ�µ�Ϊ1/4
				if (cur_data < (newPeak.nPeakValue + newPeak.nStartValue) / 4)
				{
					// �Ѿ��½���peak���ȵ�һ�룬ȷ���Ѳ��ɹ�������peak����
					newPeak.nPeakHight = newPeak.nPeakValue - newPeak.nStartValue;

					// �ж������ĸ߶ȣ�40LSB��Ӧ0.05mV����С�����ֵ��������T��
					if (newPeak.nPeakHight >= T_PEAK_THRESH)
					{
						// ����0.05mV��������󡢴δ�peak
						if (newPeak.nPeakHight >= maxPeak.nPeakHight)
						{
							// ��ǰpeak���ȴ������peak���ȣ������maxPeak��secPeak
							secPeak = maxPeak;
							maxPeak = newPeak;
						}
						else if (newPeak.nPeakHight >= secPeak.nPeakHight)
						{
							// ��ǰpeak���ȴ���secPeak��С��maxPeak�������secPeak����
							secPeak = newPeak;
						}
					}
					
					// ���newPeak�������½���
					newPeak.nPeakHight = 0;
					newPeak.nPeakPos = 0;
					newPeak.nPeakValue = 0;
					newPeak.nStartPos = 0;
					newPeak.nStartValue = 0;
					flag = 0;
				}
				else
				{
					//��δ�½���peak���ȵ�һ�룬����Ϊ�ѵ���������
				}
			}
		}

		last_data = cur_data;
		last_ptr = cur_ptr;
		cur_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.4 ȷ��T��
	if (0 != (maxPeak.nPeakHight + secPeak.nPeakHight))
	{
		// max��sec��������һ����Ϊ0�����maxPeak��secPeak��ѡ��һ����ΪT��
		// ����maxPeak��secPeak��QRS���ľ���
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

		// ��secPeak����С��maoPeak���ҷ��ȴ���maxPeak��2/3������secPeak�Ĳ���ΪT������
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
		// ���û���ѵ�T����˵�������Ǵ���ȵ�S�����£���ʱ�������ȵ�λ����ΪT����λ��
		preQRS.nTpeakPos = max_pos;
	}
	t_pos_all = gTotalSampleBuff[preQRS.nTpeakPos] - LOW_PASS_DELAY;

	//1.5  ������ң�����������б��Ϊ0�ĵ���ΪST��
	// �����������յ�
	if (0 != t_pos_all)
	{
		wind_end = preQRS.nTpeakPos;
	}

	cur_ptr = wind_start;
	slop_cnt = 0;
	while ((cur_ptr != wind_end) && (5 > slop_cnt))
	{
		//���㵱ǰ���б��
		last_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		next_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		cur_slop = Abs(gLpDataBuff[next_ptr] - gLpDataBuff[last_ptr]);

		if (cur_slop <= ST_HORIZON_THRESH)
		{
			//С��0.01mV����Ϊ�޲���
			slop_cnt++;
		}
		else
		{
			// ȷ����������
			slop_cnt = 0;
		}
		cur_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.6 ����ҵ�����ֵ�����û�ҵ�������T�����յ��1/3��λ����ΪST��λ��
	if (cur_ptr != wind_end)
	{
		//˵���ҵ�����ֵ
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
		//û�ҵ�ƽֱ�ߣ�Ҳû�ҵ�T����ֱ�����յ�����5����ΪST��
		preQRS.nStPos = mod((preQRS.nEndPos + 5), ECG_DATA_BUFF_LEN);
	}
	st_pos_all = gTotalSampleBuff[preQRS.nStPos] - LOW_PASS_DELAY;


	//=========================================================================
	//2. ��λ��ǰQRS����IOS��
	//=========================================================================
	// Local Peak��ʼ��
	newPeak.nPeakHight = 0;
	newPeak.nPeakPos = 0;
	newPeak.nPeakValue = 0;
	newPeak.nStartPos = 0;
	newPeak.nStartValue = 0;
	maxPeak = newPeak;
	secPeak = newPeak;

	//1.1 ȷ��T������������ֹ�㣬��QRS�����Ϊ��㣬��1/3interval��Ϊ�յ�
	wind_start = mod((curQRS.nStartPos + LOW_PASS_DELAY), ECG_DATA_BUFF_LEN);
	wind_end  = mod((wind_start - interval/2), ECG_DATA_BUFF_LEN);

	//1.2 �趨��ʼֵ����ʼ�Ѳ�
	cur_ptr = wind_start;
	flag = 0;
	if (curQRS.isPositive)
	{
		// ������Ҫ��Ϊ�˷�������Ѳ�������Ǹ���QRS���������������ȡ��ֵ
		cur_data = gLpDataBuff[wind_start];
	}
	else
	{
		cur_data = -gLpDataBuff[wind_start];
	}
	last_data = cur_data;

	//1.3 ��ʼ�Ѳ�
	while (cur_ptr != wind_end)
	{
		if (curQRS.isPositive)
		{
			// ������Ҫ��Ϊ�˷�������Ѳ�������Ǹ���QRS���������������ȡ��ֵ
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
				// �����ʼ�������򱣴�peak���
				newPeak.nStartValue = last_data;
				newPeak.nStartPos = last_ptr;
				newPeak.nPeakValue = cur_data;
				newPeak.nPeakPos = cur_ptr;
				flag = 1;
			}
			else
			{
				//��Ȼ���½���ɶ�������������Ѳ�
			}
		}
		else
		{
			if (cur_data > last_data)
			{
				// ��Ȼ�����������ֵ�ǰ״̬�������������peak����
				if (cur_data > newPeak.nPeakValue)
				{
					newPeak.nPeakValue = cur_data;
					newPeak.nPeakPos = cur_ptr;
				}
			}
			else
			{
				// ��ʼ�½��ˣ��ж��Ƿ��½���peak���ȵ�һ�룬�������ȷ���ѵ�peak��������Ǿͼ����Ѳ�
				if (cur_data < (newPeak.nPeakValue + newPeak.nStartValue) / 4)
				{
					// �Ѿ��½���peak���ȵ�һ�룬ȷ���Ѳ��ɹ�������peak����
					newPeak.nPeakHight = newPeak.nPeakValue - newPeak.nStartValue;

					// �ж������ĸ߶ȣ�20LSB��Ӧ0.05mV����С�����ֵ��������T��
					if (newPeak.nPeakHight >= T_PEAK_THRESH)
					{
						// ����0.05mV��������󡢴δ�peak
						if (newPeak.nPeakHight >= maxPeak.nPeakHight)
						{
							// ��ǰpeak���ȴ������peak���ȣ������maxPeak��secPeak
							secPeak = maxPeak;
							maxPeak = newPeak;
						}
						else if (newPeak.nPeakHight >= secPeak.nPeakHight)
						{
							// ��ǰpeak���ȴ���secPeak��С��maxPeak�������secPeak����
							secPeak = newPeak;
						}
					}

					// ���newPeak�������½���
					newPeak.nPeakHight = 0;
					newPeak.nPeakPos = 0;
					newPeak.nPeakValue = 0;
					newPeak.nStartPos = 0;
					newPeak.nStartValue = 0;
					flag = 0;
				}
				else
				{
					//��δ�½���peak���ȵ�һ�룬����Ϊ�ѵ���������
				}
			}
		}

		last_data = cur_data;
		last_ptr = cur_ptr;
		cur_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.4 ȷ��P��
	if (0 != (maxPeak.nPeakHight + secPeak.nPeakHight))
	{
		// max��sec��������һ����Ϊ0�����maxPeak��secPeak��ѡ��һ����ΪP��
		// ����maxPeak��secPeak��QRS���ľ���
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

		// ��secPeak����С��maoPeak���ҷ��ȴ���maxPeak��2/3������secPeak�Ĳ���ΪP������
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

	//1.5  ������ң�������5��б��Ϊ0�ĵ���ΪIOS��
	// �����������յ�
	if (0 != p_pos_all)
	{
		wind_end = curQRS.nPpeakPos;
	}

	cur_ptr = wind_start;
	slop_cnt = 0;
	while ((cur_ptr != wind_end) && (5 > slop_cnt))
	{
		//���㵱ǰ���б��
		last_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		next_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		cur_slop = gLpDataBuff[next_ptr] - gLpDataBuff[last_ptr];

		if (cur_slop <= ST_HORIZON_THRESH)
		{
			//С��0.01mV����Ϊ�޲���
			slop_cnt++;
		}
		else
		{
			// ȷ����������
			slop_cnt = 0;
		}
		cur_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//1.6 ����ҵ�����ֵ�����û�ҵ�������T�����յ��1/3��λ����ΪISO��λ��
	if (cur_ptr != wind_end)
	{
		//˵���ҵ�����ֵ
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
		//û�ҵ�ƽֱ�ߣ�Ҳû�ҵ�T����ֱ�����յ�����������ΪST��
		curQRS.nIsoPos = mod((curQRS.nStartPos - 3), ECG_DATA_BUFF_LEN);
	}
	ios_pos_all = gTotalSampleBuff[curQRS.nIsoPos] - LOW_PASS_DELAY;

	//=========================================================================
	//3������ǰһ��QRS����STֵ
	//=========================================================================
	if ((0 != ios_pos_all) && (0 != st_pos_all))
	{
		preQRS.nStValue = gLpDataBuff[preQRS.nStPos] - gLpDataBuff[preQRS.nIsoPos];
	}
	else
	{
		preQRS.nStValue = 9999;
	}

	// ��ʱ���nStPos��nIsoPos��û�м�ȥ LOW_PASS_DELEY����ViewWave�Բ��ϣ����Ҫ�������ȥ
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
	//4������������QRS��������
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

	// �տ�ʼ��⵽��QRS�����ܲ�׼ȷ��������STֵ
	if (gQrsPeakNum < 4)
	{
		return 0;
	}

	// ��ȡ��һ��QRS�������һ�ȡ��IOS�㣬ST��ķ���
	preQRS = gQrsPeakBuff[gQrsPeakNum - 2];
	curQRS  = gQrsPeakBuff[gQrsPeakNum - 1];

	// �趨�Ѳ��������յ㡣����һ����������T����ֵ��Ϊ��㣬1/2 RR����Ϊ�ص�
	// ��������QRS���ļ�࣬����ȷ����������
	interval = curQRS.nPeakPosAll - preQRS.nPeakPosAll;
	if ((interval <= 0) || (interval >= ECG_DATA_BUFF_LEN))
	{
		// ����QRS�ľ��볬�������ݻ������ľ��룬����λ�ò��ԣ�˵�����󣬷���
		preQRS.nTendPos = 9999;
		preQRS.nQtValue = 0;
		return 0;
	}

	//1.1 ȷ��T������������ֹ�㣬��QRS���յ�Ϊ��㣬��1/2interval��Ϊ�յ�
	if (9999 == preQRS.nTpeakPos)
	{
		// ��û���ҵ�T�����򲻼���QTֵ
		preQRS.nTendPos = 9999;
		preQRS.nQtValue = 0;
		return 0;
	}

	wind_start = LoopInc(preQRS.nTpeakPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);
	wind_end  = LoopInc(preQRS.nPeakPos, LOW_PASS_DELAY, ECG_DATA_BUFF_LEN);

	wind_end  = LoopInc(wind_end, interval*2/3, ECG_DATA_BUFF_LEN);

	//=========================================================================
	// 1. ��T�����忪ʼ������ѣ�������һ������������ڴ�ͨ�����Ͻ��м��
	//		������ISO��ST��ƽ��������ȵĵ�
	//		б��С�����б�ʵ� 1/5
	//=========================================================================
	cur_ptr = wind_start;
	amp_cur = gLpDataBuff[cur_ptr];
	max_amp = amp_cur;
	max_ptr = cur_ptr;
	min_amp = amp_cur;
	min_ptr = cur_ptr;

	while (cur_ptr != wind_end)
	{
		// ��ȡ��ǰ�㣬��ǰ���ķ���
		last_ptr = LoopDec(cur_ptr, 1, ECG_DATA_BUFF_LEN);
		next_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);

		amp_cur = gLpDataBuff[cur_ptr];
		amp_last = gLpDataBuff[last_ptr];
		amp_next = gLpDataBuff[next_ptr];

		//=========================================================================
		// ��һ�ַ���������б�ʽ����жϣ���������С�����б��1/5�����ж�ΪT����ֹ
		//		���ַ����������ʽϵͣ�120�����£����жϱȽ�׼ȷ
		//=========================================================================
		//���㵱ǰ���б��
		cur_slop = Abs(gLpDataBuff[next_ptr] - gLpDataBuff[last_ptr]);

		// ������������б��С�����б�ʵ�1/5�����ж�Ϊ��ֹ��
		if (cur_slop > max_slop)
		{
			// �������б��
			max_slop = cur_slop;
			slop_cnt = 0;
		}
		else if (cur_slop < max_slop/5)
		{
			// ��ǰб��С�����б�ʵ�1/5
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
			// ��б�ʼ�������
			slop_cnt = 0;
		}

		//=========================================================================
		// �ڶ��ַ�����������������Ϊ�½���1/3����͵�
		//		���ַ������������ʽϸߣ�140�����ϣ��Ĳ���
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
			// �����׶�
			if ((amp_next - min_amp) > (max_amp - min_amp) / 3)
			{
				// �����εķ��ȴ����½��η��ȵ�1/3����ȡ��Сֵ��ΪT���յ�
				tEnd_ptr = min_ptr;
				break;
			}
		}

		cur_ptr = LoopInc(cur_ptr, 1, ECG_DATA_BUFF_LEN);
	}

	//=========================================================================
	// ����õ���tEnd������QT������tEnd��䵽preQRS����
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
// ����HRV��ز���
// ���������������� Mean��SDNN��rMSSD����ʱ����Ҫ��ô�������ע�͵�����
//=========================================================================

// char CalculateFinalHrv(int rri_num, int *rri_value)
// {
// 	int i = 0, temp = 0, local_sum = 0, pnn_thr = 0;
// 
// 	// �����жϣ�ƽ��RRΪ0�������㣻RR����С��30��������
// 	if ((0 >= gEcgFinalResult.HrvResult.nHrvMean) || (10 >= rri_num))
// 	{
// 		gEcgFinalResult.cHRV = 0;
// 		return 0;
// 	}
// 
// 	//=========================================================================
// 	//1. ����SDNN�������׼��
// 	//=========================================================================
// 	// �������ƽ����
// 	local_sum = 0;
// 	for (i = 0; i < rri_num; i++)
// 	{
// 		temp = rri_value[i] - gEcgFinalResult.HrvResult.nHrvMean;
// 		local_sum += temp * temp;
// 	}
// 
// 	// �����ƽ���͵ľ�ֵ��ƽ����
// 	local_sum = local_sum / rri_num;
// 	gEcgFinalResult.HrvResult.nHrvSdnn = sqrt(float(local_sum));
// 
// 
// 	//=========================================================================
// 	//2. ����PNN50�����ڼ���>50ms�İٷֱ�
// 	//=========================================================================
// 	pnn_thr = ECG_SAMPLE_RATE / 50;
// 	local_sum = 0;
// 
// 	// �������
// 	for (i = 0; i < rri_num-1; i++)
// 	{
// 		temp = rri_value[i+1] - rri_value[i];
// 		if (temp > pnn_thr)
// 		{
// 			local_sum++;
// 		}
// 	}
// 
// 	// ����ٷֱ�
// 	gEcgFinalResult.HrvResult.nHrvPnn50 = local_sum * 100 / (rri_num - 1);
// 
// 	//=========================================================================
// 	//3. �����ж�
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

	// ����ƽ��QRS����
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nQrsAmp)
		{
			// ��ʼ�׶ε�ampΪ0�����������
			total_amp[ave_num] = gQrsPeakBuff[i].nQrsAmp;
			ave_num ++;
		}
	}

	// ��total_rri����
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

		// �����м�1/3��ƽ��RRI
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
		// �����������amp����Ϊ0��˵������
		final_amp = 0;
	}

	return final_amp;
}

//int CalculateFinalHr0(int rri_num, int *total_rri)
//{
//	int ave_rri = 0, temp = 0, heart_rate = 0, final_hr = 0;
//	int i = 0, j = 0, left = 0, right = 0;

//	// ��total_rri����
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

//		// �����м�1/3��ƽ��RRI
//		ave_rri = 0;
//		left = rri_num / 3;
//		right = rri_num * 2 / 3;
//		for (i = left; i <= right; i++)
//		{
//			ave_rri += total_rri[i];
//		}
//		ave_rri = ave_rri / (right - left + 1);

//		// ��������
//		heart_rate = ECG_SAMPLE_RATE * 60 / ave_rri;

//		//3.3 �ж��Ƿ��ڼ��㷶Χ��
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
//		// �����������RRI����Ϊ0��˵������
//		final_hr = 9999;
//	}

//	// ����μ���Ľ�����бȶԣ��ж��Ƿ����
//	if (gbFirstExist)
//	{
//		// ������Ľ��С�ڳ������ʵ�80%�����Գ��μ���Ϊ׼
//		if ((final_hr * 5) < (gEcgFinalFirst.nEcgHeartRate * 4))
//		{
//			final_hr = gEcgFinalFirst.nEcgHeartRate;
//		}

//		// ������֮��������Ч�����ó��εĽ��
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

	// ��total_rri����
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

		// �����м�1/3��ƽ��RRI
		ave_rri = 0;
		left = rri_num / 3;
		right = rri_num * 2 / 3;
		for (i = left; i <= right; i++)
		{
			ave_rri += total_rri[i];
		}
		ave_rri = ave_rri / (right - left + 1);

		// ��������
		heart_rate = ECG_SAMPLE_RATE * 60 / ave_rri;

		//3.3 �ж��Ƿ��ڼ��㷶Χ��
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
		// �����������RRI����Ϊ0��˵������
		final_hr = 9999;
	}

	// ����μ���Ľ�����бȶԣ��ж��Ƿ����
	if (gbFirstExist)
	{
		// ������Ľ��С�ڳ������ʵ�80%�����Գ��μ���Ϊ׼
		if ((final_hr * 5) < (gEcgFinalFirst.nEcgHeartRate * 4))
		{
			final_hr = gEcgFinalFirst.nEcgHeartRate;
		}

		// ������֮��������Ч�����ó��εĽ��
		if (9999 == final_hr)
		{
			final_hr = gEcgFinalFirst.nEcgHeartRate;
		}
	}

	return final_hr;
}


// �жϲ�������ɣ����ù�һ�� r-MSSD���м���
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
	//1�� ����1������r-MSSD
	//=========================================================================
	// ��������RRI�Ĳ�ֵ��ƽ����
	for (i = 0; i < (rri_num - 1); i++)
	{
		temp_value = rri_value[i+1] - rri_value[i];
		std_dev += temp_value * temp_value;

		ave_rri += rri_value[i];
	}

	// ����ƽ��RRI
	ave_rri = (ave_rri + rri_value[rri_num - 1]) / rri_num;

	// ����RRIƽ���͵ľ�����
	std_dev = std_dev / (rri_num - 1);
	std_dev = sqrt((float)(std_dev));

	// �����һ�� r-MSSD
	gEcgFinalResult.nIrregCoef = std_dev * 100 / ave_rri;

	// ���������ƶ���ͬ����ֵ
	if (gEcgAlgConfig.bJapanVersion)
	{
		irreg_thr = IRREG_THR_JAP;
	}
	else
	{
		irreg_thr = IRREG_THR_NRM;
	}

	// �ж��Ƿ�����IRREG���������ڵ��ڲ�����ϵ����ֵ
	if (gEcgFinalResult.nIrregCoef >= irreg_thr)
	{
		r_mssd = true;
	}
	else
	{
		r_mssd = false;
	}

	//=========================================================================
	//2�� ����2�����������С��RR���ڵĲ�ֵ
	//=========================================================================
	if (gEcgAlgConfig.bJapanVersion)
	{
		// �ձ��汾�Ƚ�������2������RR���ڼ�����ͳ��
		num_thr = 2;
	}
	else
	{
		num_thr = 3;
	}

	cur_rr = rri_sort[0];
	sum_rr = cur_rr;
	cur_num = 1;
	pre_rr = rri_sort[0] * 115 / 100;		// ���ü���������ֵΪ115%

	for (i = 1; i < gQrsPeakNum; i++)
	{
		cur_rr = rri_sort[i];

		if (0 == cur_rr)
		{
			continue;
		}

		if ((cur_rr <= pre_rr) && (pre_rr != 0))
		{
			// ��ǰ��rr������ǰһ��rr+10����Ϊ��ͬһ��rr���ڣ������ۼӲ�����
			sum_rr += cur_rr;
			cur_num++;
		}
		else
		{
			// ���ȼ�����һ����ƽ��ֵ
			if (cur_num >= num_thr)
			{
				// �����һ����rr�������ڵ���3�����Ϳ�ʼ���������Сƽ������
				if (0 == min_rr)
				{
					// minֻ����һ�Σ�ȷ��ȡ����С��rr��λ
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

			// �µ�rr�ֵ������������ֵ������������
			pre_rr = cur_rr * 115 / 100;
			sum_rr = cur_rr;
			cur_num = 1;
		}
	}

	// ���Ҫ�ټ���һ��ƽ��ֵ����Ϊ�������һ�������ݻ�δ����
	if (cur_num >= num_thr)
	{
		// �����һ����rr�������ڵ���3�����Ϳ�ʼ���������Сƽ������
		if (0 == min_rr)
		{
			// minֻ����һ�Σ�ȷ��ȡ����С��rr��λ
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

	// ���������С���죬��С��С������3/4���ж�Ϊ���������
	if (min_rr < max_rr * 3 / 4)
	{
		max_min_diff = true;
	}
	else
	{
		max_min_diff = false;
	}


	//=========================================================================
	//3�� ����1������2�����ȣ����߶��ﵽҪ�󣬾��ж�Ϊ���������
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
// ����HRV��ز���
// ���������������� Mean��SDNN��rMSSD����ʱ����Ҫ��ô�������ע�͵�����
//=========================================================================
int CalculateFinalHrv(void)
{
	float temp_hrv = 0, temp = 0;
	int final_hrv = 0, i = 0, pnn_num = 0;
	int mean = 0, sdnn = 0, rmssd = 0, pnn50 = 0;

	//=========================================================================
	// ����FinalHrv���˴�����������״��
	//=========================================================================
	if ((10 >= gTotalHrvNum))
	{
		// �������HRV��С����QRS����С�ڵ���10�����򲻼���HRV
		final_hrv = -1;
	}
	else
	{
		// �����������ע�⣬�ý����λΪms���Ŵ�100����
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
		// �����Peak��Ϊ0��˵����������Ҳ��ֹ����
		return 0;
	}
	else
	{
		// ȡ����QRS����STֵ����������
		ave_num = 0;
		for (i = 0; i < gQrsPeakNum; i++)
		{
			if ((9999 != gQrsPeakBuff[i].nStValue) && (1 == gQrsPeakBuff[i].nQrsType) && ((gQrsPeakBuff[i].nPeakPosAll + time_dist) >= gEcgTotalSampleNum))
			{
				// �����޷������STֵ����Ϊ9999�����������
				// ֻ��������QRS��STֵ����ΪPVCһ��STֵ�ܸ�
				// ֻͳ�ƹ涨ʱ���ڣ�holterΪ1���ӣ��ֳֻ�Ϊ10���ӣ�
				st_value[ave_num] = gQrsPeakBuff[i].nStValue;
				ave_num ++;
			}
		}

		// ��total_rri����
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

			// �����м�1/3��ƽ��stֵ
			ave_st = 0;
			left = ave_num / 3;
			right = ave_num * 2 / 3;
			for (i = left; i <= right; i++)
			{
				ave_st += st_value[i];
			}
			ave_st = ave_st / (right - left + 1);

			// �������յ�STֵ��1LSB=1.25uV��Ȼ����ΪmV
			// gEcgFinalResult.nEcgStValue = ave_rri * 10 / 4 / 1000;
			final_st = ave_st * uV_PER_LSB / 1000;
		}
		else
		{
			// �����������RRI����Ϊ0��˵������
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
		// �����Peak��Ϊ0��˵����������Ҳ��ֹ����
		return 0;
	}
	else
	{
		// ȡ����QRS����QTֵ����������
		ave_num = 0;
		for (i = 0; i < gQrsPeakNum; i++)
		{
			if ((0 != gQrsPeakBuff[i].nQtValue) && (1 == gQrsPeakBuff[i].nQrsType) && (((gQrsPeakBuff[i].nPeakPosAll + time_dist) >= gEcgTotalSampleNum)))
			{
				// �����޷������QTֵ����Ϊ0�����������
				// ֻ��������QRS��QTֵ����ΪPVCһ��STֵ�ܸ�
				qt_value[ave_num] = gQrsPeakBuff[i].nQtValue;
				ave_num ++;
			}
		}

		// ��total_rri����
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

			// �����м�1/3��ƽ��QTֵ
			ave_qt = 0;
			left = ave_num / 3;
			right = ave_num * 2 / 3;
			for (i = left; i <= right; i++)
			{
				ave_qt += qt_value[i];
			}
			ave_qt = ave_qt / (right - left + 1);

			// �������յ�QTֵ����λΪms
			final_qt = ave_qt * 1000 / ECG_SAMPLE_RATE;
		}
		else
		{
			// �����������RRI����Ϊ0��˵������
			final_qt = 9999;
		}
	}

	return final_qt;
}


// �������յ�QRS���
int CalculateFinalWidth(int *total_width, int time_dist)
{
	int wid_num = 0, temp = 0, i = 0, j = 0, left = 0, right = 0;
	float wid_time = 0, ave_wid = 0, final_width = 0;

	if (0 >= gQrsPeakNum)
	{
		// �����Peak��Ϊ0��˵����������Ҳ��ֹ����
		return 0;
	}
	else
	{
		// ����ƽ��Qrs���
		ave_wid = 0;
		wid_num = 0;
		for (i = 0; i < gQrsPeakNum; i++)
		{
			if (0 != gQrsPeakBuff[i].nQrsWidth && ((gQrsPeakBuff[i].nPeakPosAll + time_dist) >= gEcgTotalSampleNum))
			{
				// ��ʼ�׶ε�RRIΪ0�����������
				total_width[wid_num] = gQrsPeakBuff[i].nQrsWidth;
				wid_num ++;
			}
		}

		// ��total_rri����
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

			// �����м�1/3��ƽ��RRI
			ave_wid = 0;
			left = wid_num / 3;
			right = wid_num * 2 / 3;
			for (i = left; i <= right; i++)
			{
				ave_wid += total_width[i];
			}
			ave_wid = ave_wid / (right - left + 1);

			// ����ƽ��QRS��ȣ���λΪms
			final_width = 1000 * ave_wid / ECG_SAMPLE_RATE;

			// �����������ʾ�˲���40Hz��ͨ�˲�����SR=500Hz������8��չ����16ms������Ҫ��ȥ
			final_width = final_width - 16;

			// ������������ݵ�ǰ��ȳ�һ��������ʵ��һ��������   todo����ʱ�����������Ż�
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
			// �����������RRI����Ϊ0��˵������
			final_width = 9999;
		}
	}

	return final_width;
}


// 20�������������յ�������
int EcgAlgGetFinalResult(ECG_FINAL_RESULT *final_result, bool reset)
{
	float ave_rri = 0;
	int i = 0, j = 0, left = 0, right = 0, temp = 0, ave_amp = 0, sum_rr = 0, heart_rate = 0;
	int min_thresh = 0, max_thresh = 0, rri_num = 0, noise_sig_num = 0;
	int min_rr = 0, max_rr = 0, cur_rr = 0, pre_rr = 0, cur_num = 0, amp_num = 0;
	float wid_time = 0;		// ��ʱ�����������������

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

	// ��С֧��30BPM��20s����10��QRS�������С��5�����Ͳ����к���ļ���
	if (10 >= gQrsPeakNum)
	{
		if (gbFirstExist)
		{
			// ����û�еõ���������ǳ��ν�����ڣ���ֱ�ӽ����μ���Ľ����ֵ
			//*final_result = gEcgFinalFirst;
			//return 1;
			EcgAlgGetFinalResult(0, true);
		}
		else
		{
			// û��������QRS�����ҳ��ν��Ҳ�����ڣ�����ֱ���˳�
		}
		return 0;
	}

	//=========================================================================
	//0. �ж�����״̬�������ж����������ܶԺ�����״̬�趨�а���
	//=========================================================================
	for (i = 0; i < gEcgNoiseBuffNum; i ++)
	{
		if (gEcgNoiseBuff[i].nNoiseType)
		{
			noise_sig_num++;
		}
	}

	// �������ݶΣ��������źŵ�һ�룬����ʾ�źŲ�
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
	//1. ����ȫ��QRS��ƽ�����Ⱥ�ƽ��RR���ڣ�Ȼ�����һ���ų�
	//=========================================================================
	//1.1 �����з��Ⱥͼ���ȫ�����뻺������׼�������ļ���
	for (i = 0; i < gQrsPeakNum; i++)
	{
		total_amp[i] = gQrsPeakBuff[i].nQrsAmp;
		total_rri[i] = gQrsPeakBuff[i].nRRInterval;
	}

	//1.2 ð�����򣬽����Ⱥͼ��ھ�������������
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

	//1.3 ����ƽ�����Ⱥͼ��ڣ������м�����1/3���ּ���
	// �˴���˼����ȡ�ϴ��1/3�����ֵ����Ҫ��Ϊ���ų�һЩ�ͷ��ȵĸ��ţ��Ѹ��ģ�
	// ���߲��Կ����д����QRS���ţ����Ի���ȡ�м�1/3
	left = gQrsPeakNum / 3;
	right = gQrsPeakNum * 2 / 3;
	for (i = left; i <= right; i++)
	{
		ave_amp += total_amp[i];
		ave_rri += total_rri[i];
	}
	ave_amp = ave_amp / (right - left + 1);
	ave_rri = ave_rri / (right - left + 1);

	//1.4 ���ݷ����ų��������QRS��
	if ((0 < ave_amp) && (0 < ave_rri))
	{
		for (i = 0; i < gQrsPeakNum; i++)
		{
			curPeak = gQrsPeakBuff[i];
			min_thresh = 0;
			max_thresh = 0;

			// ��������ų���ֵ
			if (curPeak.nRRInterval * 3 < ave_rri * 2)
			{
				//��rr���ڱ�ƽ��rr���ڵ�2/3��Сʱ����ֵ���ø���һ�㣬Ҫ����ƽ�����ȵ�1/2��2��֮��
				min_thresh = ave_amp / 2;
				max_thresh = ave_amp * 2;
			}
			else if (curPeak.nRRInterval < ave_rri)
			{
				//rr����С��ƽ�����ڣ���ſ���ֵ��1/3��3��֮�䶼����
				min_thresh = ave_amp / 3;
				max_thresh = ave_amp * 3;
			}
			else
			{
				//����ʱRR�����Ѿ�����ƽ�����ڣ���������ֵ���ƣ����ⳤʱ���Ѳ�ʧ��
				min_thresh = QRS_PEAK_MIN_THRESH;
				max_thresh = QRS_PEAK_MAX_THRESH;
			}

			// �жϷ����Ƿ񳬳���ֵ��Χ�����������ų����������º�һ��QRS����RR����
			if ((curPeak.nQrsAmp < min_thresh) || (curPeak.nQrsAmp > max_thresh))
			{
				// ���ų�����QRS���ĺ���һ��QRS����rr���ڸ���
				if (gQrsPeakNum > (i + 1) && (0 <= (i - 1)))
				{
					// �ú�һ��Peak�Ĳ���λ�ü�ȥǰһ��Peak�Ĳ���λ�ã�����RR����
					gQrsPeakBuff[i + 1].nRRInterval = gQrsPeakBuff[i + 1].nPeakPosAll - gQrsPeakBuff[i - 1].nPeakPosAll;
				}

				// �ų���ǰ��QRS����������
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

	// �����Peak��Ϊ0��˵����������Ҳ��ֹ����
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
	//2. �ø��º��QRS�����¼���ƽ������
	//=========================================================================
	gEcgFinalResult.nEcgQrsAmp = CalculateFinalAmp(total_amp);

	// �ж��Ƿ���ȹ���
	if (QRS_LOW_AMP_THRESH > gEcgFinalResult.nEcgQrsAmp)
	{
		// ƽ������С����С������ֵ���򱨷��ȹ���
		gEcgFinalResult.LowQrsAmp = true;
	}
	else
	{
		gEcgFinalResult.LowQrsAmp = false;
	}

	//=========================================================================
	// ��ȡȫ����RR���ڣ��Ա�����
	//=========================================================================
	ave_rri = 0;
	rri_num = 0;
	amp_num = 0;
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nRRInterval)
		{
			// ��ʼ�׶ε�RRIΪ0�����������
			total_rri[rri_num] = gQrsPeakBuff[i].nRRInterval;
			rri_num ++;

			total_amp[amp_num] = gQrsPeakBuff[i].nRRInterval;
			amp_num++;
		}
	}

	//=========================================================================
	//2. �ø��º��RR�������¼�������
	//=========================================================================
//	if(rri_num > 15)
//	{
//		gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(rri_num - 15, total_rri);
//	}
//	else 
//	{
		gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(rri_num, total_rri);
//	}
	
	
	

	// �ж��Ķ����١��Ķ�������
	if (9999 == gEcgFinalResult.nEcgHeartRate)
	{
		// ����Ϊ��Чֵ
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
	//4. �жϲ�������ɣ�����С��1/2ƽ��RR���ڣ�С������1/2��ƽ��RR���ڵ�һ�룬���ж�Ϊ���������
	//		�˴���total_amp�洢��ʱû�������RRI��total_rri����洢����������RRI
	//		�������� CalculateFinalHr ����ʵ��
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
	//10. HRV���㣬Mean��rMSSD���� JudgeIrregRhythm �м����
	//=========================================================================
	gEcgFinalResult.nEcgFinalHrv = CalculateFinalHrv();
	
	//=========================================================================
	//5. ����STֵ�������λΪuV
	//		���� total_rri �����������Ϊ ST ֵ
	//=========================================================================
	gEcgFinalResult.nEcgStValue = CalculateFinalSt(total_rri, 600*ECG_SAMPLE_RATE);

	// �ж�ST���ߡ�����
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
	//6. ����QTֵ�������λΪms
	//		���� total_rri �����������Ϊ QT ֵ
	//=========================================================================
	gEcgFinalResult.nEcgQtValue = CalculateFinalQt(total_rri, 600*ECG_SAMPLE_RATE);

	// ������Χ����Ч
	if ((gEcgFinalResult.nEcgHeartRate > 200) || (gEcgFinalResult.nEcgHeartRate < 30))
	{
		gEcgFinalResult.nEcgQtValue = 9999;
	}

	// ����QTc
	if (9999 == gEcgFinalResult.nEcgQtValue)
	{
		gEcgFinalResult.nEcgQTc = 9999;
	}
	else
	{
		gEcgFinalResult.nEcgQTc = (float)(gEcgFinalResult.nEcgQtValue) / sqrt((60 / (float)(gEcgFinalResult.nEcgHeartRate)));
	}

	// �ж��ӳ�������
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
	//7. ����ƽ��QRS��ȣ����ж�QRS����
	//		���� total_rri �����������Ϊ QRS width
	//=========================================================================
	gEcgFinalResult.nEcgQrsWidth = CalculateFinalWidth(total_rri, 600*ECG_SAMPLE_RATE);

	// �жϿ�ȹ���
	if (120 < gEcgFinalResult.nEcgQrsWidth)
	{
		gEcgFinalResult.WideQrs = true;
	}
	else
	{
		gEcgFinalResult.WideQrs = false;
	}

	//=========================================================================
	//8. ���PVC�ĸ������Լ��жϽ��
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
	
//	// �����ж�
//	if (gEcgAlgResult.isAsystole != 1) {
//		if (9999 == gEcgFinalResult.nEcgQTc || -1 == gEcgFinalResult.nEcgFinalHrv)
//		{
//			gEcgFinalResult.SignalNoise = 2;
//		}
//	}

	//=========================================================================
	// �жϳ��ν���Ƿ���ڣ�������������д洢
	//=========================================================================
	if (!gbFirstExist)
	{
		// �����ڣ������ν����ֵ
		gEcgFinalFirst = gEcgFinalResult;
		gbFirstExist = true;
	}

	//=========================================================================
	// ������Ľ��
	//=========================================================================
	*final_result = gEcgFinalResult;
	return 1;
}


//=========================================================================
// ��ȡ����QRS������λ�ã��ṩ��PWTT��ECG����������ʱ����Ҫ���øú���
//=========================================================================
int EcgAlgGetQrsPeakPos(int *qrs_pos_buff)
{
	int i = 0, delay = 0;

	/*
	if (1 == gEcgAlgConfig.cViewMode)
	{
		// ģʽһ��0.05 - 40Hz
		delay = DELAY_VIEW_LP40;
	}
	else if (2 == gEcgAlgConfig.cViewMode)
	{
		// ģʽ���� 0.5 - 20Hz
		delay = DELAY_VIEW_HP05 + DELAY_VIEW_LP40;
	}
	else
	{
		// Ĭ��ģʽ���� 0.5 - 20Hz
		delay = DELAY_VIEW_HP05 + DELAY_VIEW_LP40;
	}
	*/

	for (i = 0; i < gQrsPeakNum; i++)
	{
		// ����Ϊ1000Hz������
		qrs_pos_buff[i] = gQrsPeakBuff[i].nPeakPosAll * 4 - delay * 2;
	}

	return gQrsPeakNum;
}


int EcgAlgCalculateHolterHRV(unsigned short curRRI)
{
	static unsigned short last_rri = 0;
	float temp = 0;
	int ratio = 0;


	// �����������
	ratio = 1000 / ECG_SAMPLE_RATE;

	if ((0 == curRRI) || (curRRI * 10 < gEcgHolterResult.nAveRRinterval) || (curRRI > gEcgHolterResult.nAveRRinterval * 5))
	{
		// �޳�������RRIΪ0��RRI����ƽ��RRI��5����RRIС��ƽ��RRI��1/10
	}
	else
	{
		// 2. ���������׼��SDNN����λΪ������
		temp = curRRI - gEcgHolterResult.nAveRRinterval;
		gEcgHolterResult.nHolterSdnn += temp * temp;

		// 3. �����ֵ������rMSSD����λΪ������
		if (0 != last_rri)
		{
			temp = curRRI - last_rri;
			gEcgHolterResult.nHolterRmssd += temp * temp;

			// 3. �������50ms�ı���
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



// Holter 1���ӽ��
int EcgAlgGetHolterOneMinResult(ECG_FINAL_RESULT *final_result, bool reset)
{
	float ave_rri = 0;
	int i = 0, j = 0, left = 0, right = 0, temp = 0, ave_amp = 0, sum_rr = 0, heart_rate = 0;
	int min_thresh = 0, max_thresh = 0, rri_num = 0, noise_sig_num = 0, peak_num = 0;
	int min_rr = 0, max_rr = 0, cur_rr = 0, pre_rr = 0, cur_num = 0, amp_num = 0;
	float wid_time = 0;		// ��ʱ�����������������

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

	// ��С֧��30BPM��20s����10��QRS�������С��5�����Ͳ����к���ļ���
	if (5 >= gQrsPeakNum)
	{
		// û��������QRS�����ҳ��ν��Ҳ�����ڣ�����ֱ���˳�
		*final_result = gEcgFinalResult;
		return 0;
	}

	//=========================================================================
	//0. �ж�����״̬�������ж����������ܶԺ�����״̬�趨�а���
	//=========================================================================
	for (i = 0; i < gEcgNoiseBuffNum; i ++)
	{
		if (gEcgNoiseBuff[i].nNoiseType)
		{
			noise_sig_num++;
		}
	}

	// �������ݶΣ��������źŵ�һ�룬����ʾ�źŲ�
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

	// ������һ���ӵ�������������¼���
	gEcgNoiseBuffNum = 0;

	//=========================================================================
	//1. ����ȫ��QRS��ƽ�����Ⱥ�ƽ��RR���ڣ�Ȼ�����һ���ų�
	//=========================================================================
	//1.1 �����з��Ⱥͼ���ȫ�����뻺������׼�������ļ���
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if ((gQrsPeakBuff[i].nPeakPosAll + ECG_SAMPLE_RATE * 60) > gEcgTotalSampleNum)
		{
			// ֻͳ��1�����ڵ�QRS����������
			total_amp[peak_num] = gQrsPeakBuff[peak_num].nQrsAmp;
			total_rri[peak_num] = gQrsPeakBuff[peak_num].nRRInterval;
			peak_num++;
		}
	}

	if (5 >= peak_num)
	{
		// ���1�����ڵ�QRS����������5�����򱨴�
		*final_result = gEcgFinalResult;
		return 0;
	}

	//1.2 ð�����򣬽����Ⱥͼ��ھ�������������
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

	//1.3 ����ƽ�����Ⱥͼ��ڣ������м�����1/3���ּ���
	// �˴���˼����ȡ�ϴ��1/3�����ֵ����Ҫ��Ϊ���ų�һЩ�ͷ��ȵĸ��ţ��Ѹ��ģ�
	// ���߲��Կ����д����QRS���ţ����Ի���ȡ�м�1/3
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
	//2. �ø��º��QRS�����¼���ƽ������
	//=========================================================================
	gEcgFinalResult.nEcgQrsAmp = ave_amp;

	// �ж��Ƿ���ȹ���
	if (QRS_LOW_AMP_THRESH > gEcgFinalResult.nEcgQrsAmp)
	{
		// ƽ������С����С������ֵ���򱨷��ȹ���
		gEcgFinalResult.LowQrsAmp = true;
	}
	else
	{
		gEcgFinalResult.LowQrsAmp = false;
	}

	//=========================================================================
	// ��ȡȫ����RR���ڣ��Ա�����
	//=========================================================================
	ave_rri = 0;
	rri_num = 0;
	amp_num = 0;
	for (i = 0; i < gQrsPeakNum; i++)
	{
		if (0 != gQrsPeakBuff[i].nRRInterval && ((gQrsPeakBuff[i].nPeakPosAll + ECG_SAMPLE_RATE * 60) > gEcgTotalSampleNum))
		{
			// ֻ����1�������ڵ�RRI
			// ��ʼ�׶ε�RRIΪ0�����������
			total_rri[rri_num] = gQrsPeakBuff[i].nRRInterval;
			rri_num ++;

			total_amp[amp_num] = gQrsPeakBuff[i].nRRInterval;
			amp_num++;
		}
	}

	//=========================================================================
	//2. �ø��º��RR�������¼�������
	//=========================================================================
	gEcgFinalResult.nEcgHeartRate = CalculateFinalHr(rri_num, total_rri);

	// �ж��Ķ����١��Ķ�������
	gEcgFinalResult.Bradycardia = false;
	gEcgFinalResult.Tachycardia = false;
	if (9999 == gEcgFinalResult.nEcgHeartRate)
	{
		// ����Ϊ��Чֵ
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
	//4. �жϲ�������ɣ�����С��1/2ƽ��RR���ڣ�С������1/2��ƽ��RR���ڵ�һ�룬���ж�Ϊ���������
	//		�˴���total_amp�洢��ʱû�������RRI��total_rri����洢����������RRI
	//		�������� CalculateFinalHr ����ʵ��
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
	//10. HRV���㣬Mean��rMSSD���� JudgeIrregRhythm �м����
	//=========================================================================
	gEcgFinalResult.nEcgFinalHrv = CalculateFinalHrv();

	//=========================================================================
	//5. ����STֵ�������λΪuV
	//		���� total_rri �����������Ϊ ST ֵ
	//=========================================================================
	gEcgFinalResult.nEcgStValue = CalculateFinalSt(total_rri, 60 * ECG_SAMPLE_RATE);

	// �ж�ST���ߡ�����
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
	//6. ����QTֵ�������λΪms
	//		���� total_rri �����������Ϊ QT ֵ
	//=========================================================================
	gEcgFinalResult.nEcgQtValue = CalculateFinalQt(total_rri, 60 * ECG_SAMPLE_RATE);

	// ������Χ����Ч
	if ((gEcgFinalResult.nEcgHeartRate > 200) || (gEcgFinalResult.nEcgHeartRate < 30))
	{
		gEcgFinalResult.nEcgQtValue = 9999;
	}

	// ����QTc
	if (9999 == gEcgFinalResult.nEcgQtValue)
	{
		gEcgFinalResult.nEcgQTc = 9999;
	}
	else
	{
		gEcgFinalResult.nEcgQTc = (float)(gEcgFinalResult.nEcgQtValue) / sqrt((60 / (float)(gEcgFinalResult.nEcgHeartRate)));
	}

	// �ж��ӳ�������
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
	//7. ����ƽ��QRS��ȣ����ж�QRS����
	//		���� total_rri �����������Ϊ QRS width
	//=========================================================================
	gEcgFinalResult.nEcgQrsWidth = CalculateFinalWidth(total_rri, 60 * ECG_SAMPLE_RATE);

	// �жϿ�ȹ���
	if (120 < gEcgFinalResult.nEcgQrsWidth)
	{
		gEcgFinalResult.WideQrs = true;
	}
	else
	{
		gEcgFinalResult.WideQrs = false;
	}

	//=========================================================================
	//8. ���PVC�ĸ������Լ��жϽ��
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
	// �ж��Ƿ����������ͣ
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
	//9. ����holter�����ÿ1���Ӽ���һ��
	//=========================================================================
	// 1. �����ĵ����ݵ�Ƭ��
	if (9999 == gEcgFinalResult.nEcgHeartRate)
	{
		// 9999Ϊ��Ч���ʣ����������
	}
	else if (2 == gEcgFinalResult.SignalNoise)
	{
		// �����������������Ч�����������
	}
	else
	{
		gEcgHolterResult.nTotalRhythmNum++;

		// 2. ���������С����
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

		// 3. ����ƽ������
		gEcgHolterResult.nSumHeartRate += gEcgFinalResult.nEcgHeartRate;
		gEcgHolterResult.nAveHeartRate = gEcgHolterResult.nSumHeartRate / gEcgHolterResult.nTotalRhythmNum;

		// ����ƽ��RRI
		gEcgHolterResult.nAveRRinterval = ECG_SAMPLE_RATE * 60 / gEcgHolterResult.nAveHeartRate;
	}
	

	//=========================================================================
	// ������Ľ��
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

	// �����ж������Ƿ���Ч��QRS����С��5������˵������������
	if (5 >= gQrsPeakNum)
	{
		*final_result = gEcgHolterResult;
		return 0;
	}

	ratio = 1000 / ECG_SAMPLE_RATE;

	// ������ص�HRV��������ֹ����
	if (2 < gEcgHolterResult.nRRintervalNum)
	{
		// 2. ���������׼��SDNN
		temp = gEcgHolterResult.nHolterSdnn * ratio * ratio / gEcgHolterResult.nRRintervalNum;
		gEcgHolterResult.nHolterSdnn = sqrt(temp);

		// 3. �����ֵ������rMSSD
		temp = gEcgHolterResult.nHolterRmssd * ratio * ratio / (gEcgHolterResult.nRRintervalNum - 1);
		gEcgHolterResult.nHolterRmssd = sqrt(temp);

		// 4. �������ڼ��ڲ�ֵ>50ms�İٷֱ� PNN50
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
