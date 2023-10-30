//*************************************************************************
//�ļ����ƣ�EcgMainProc.cpp
//
//�ļ�˵����Ecg�㷨������
//
//������ʷ��Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************
//include "stdafx.h"
#include "EcgQrsDetect.h"
#include <math.h>

//=========================================================================
//����QRS��ص�ȫ��
//=========================================================================
QRS_PEAK gQrsPeakBuff[QRS_PEAK_BUFF_LEN];					//QRS��������
unsigned short gQrsPeakNum;													//QRS�������ڲ��ĸ���

ECG_NOISETYPE gEcgNoiseBuff[ECG_NOISE_BUFF_LEN];		//ECG�ź�������������ÿ3����һ�Σ�30��Լ��10�����档0-�ǳ��ã� 1-��������2-�ǳ���
unsigned short gEcgNoiseBuffNum;										//ECG�ź���������������

float gTotalHrvSqr;																		// �����������HRV��ƽ���ͣ��������յ�HRV����
int gTotalHrvNum;																		// �����������HRV��QRS�������������յ�HRV����

//=========================================================================
//���屾�ļ��ڲ�ʹ�õľֲ�����
//=========================================================================
INTEG_PEAK wIntegPeakBuff[INTEG_PEAK_BUFF_LEN];		// ���ֲ���Ļ�����
int wIntegPeakNum;
int wIntegPeakAmp[INTEG_PEAK_BUFF_LEN];						// ������Ч���ַ�ķ��ȣ�Ŀ���Ƿ������������ֵ
int wPeakAmpNum;
int wIntegNoiseAmp[INTEG_PEAK_BUFF_LEN];						// ����������ķ��ȣ�Ŀ���Ƿ������������ֵ
int wNoiseAmpNum;
int wIntegAverageAmp;

int wIntegSigThresh;																	// �����źŷ�ķ�����ֵ	
int wIntegNoiseThresh;																// ����������ķ�����ֵ
int wTimeNoQrs;																			// δ��⵽QRS����ʱ��
QRS_PEAK wCurQrsPeak;															// ��ǰ��������QRS���������Ϣ

// ����������
int wAveDiffPeak;																			// 3����QRS�����Ӧ�Ĳ�ַ��ƽ�����ȣ������������
int wAveDiffPeakNum;																// 3����QRS��Ӧ�Ĳ�ַ�ĸ���
int wDiffNoisePeakBuff[ECG_NOISE_BUFF_LEN]	;					// 3��������⵽�Ĳ�ַ建����
int wDiffNoisePeakNum;																// 3��������⵽�Ĳ�ַ����


//QRS���ϲ��������ں���
int QrsComplexDetect(bool reset)
{
	INTEG_PEAK peak_result;
	int mid_value = 0, temp = 0, integ_thresh = 0, i = 0, delay = 0, temp_amp = 0;
	float temp_hrv = 0;

	static int time_integ_init, integ_peak_num;
	static bool flag_thresh_init, flag_find_integ;
	
	if (reset)
	{
		// wCurQrsPeak��ʼ��
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

		// QRS����������ʼ��
		for (i = 0; i < QRS_PEAK_BUFF_LEN; i++)
		{
			gQrsPeakBuff[i] = wCurQrsPeak;
		}
		gQrsPeakNum = 0;

		// ���ַ��ʼ��
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

		// ����������ʼ��
		wIntegSigThresh = 0;
		wIntegNoiseThresh = 0;

		time_integ_init = 0;
		integ_peak_num = 0;
		flag_thresh_init = false;
		flag_find_integ = false;

		wTimeNoQrs = 0;

		IntegThreshInitalize(true);
		IntegPeakDetection(0, 0, &peak_result, true);

		// ʵʱHRV��ʼ��
		CalculateOntimeHrv(true);
		
		return 0;
	}

	//=========================================================================
	//1. ���ַ��Ѳ���ֵ��ʼ��
	//=========================================================================
	if (!flag_thresh_init)
	{
		if (IntegThreshInitalize(false))
		{
			flag_thresh_init = true;
		}
		
		// ���ַ��Ѳ���ֵ��δ��ʼ����ֱ���˳�
		return 0;
	}

	//=========================================================================
	//2. �ڻ����ź��н��з�ֵ���
	//=========================================================================
	if (IntegPeakDetection(gIntegDataBuff[gCurBuffPtr], gCurBuffPtr, &peak_result, false))
	{
		//1. ����δ���ʱ�䣬�����Ѳ���ֵ
		temp = wTimeNoQrs * 100 / ECG_SAMPLE_RATE;

		// ���ý�����ֵ
		if (temp <= 100)
		{
			// 1���ڣ���ֵ���ֲ���
			integ_thresh = wIntegSigThresh;
		}
		else if (temp <= 250)
		{
			// 1�� - 2.5��֮�䣬��ֵ��ʱ������½�
			integ_thresh = wIntegSigThresh * 100 / temp;
		}
		else
		{
			// ������2.5��δ��⵽�µ�������������ֵ����Ϊ��С���ַ���ֵ
			integ_thresh = INTEG_PEAK_MIN_THRESH;
		}

		//2. �ж��Ƿ񳬹��Ѳ���ֵ
		if ((peak_result.nPeakHight - peak_result.nStartHight) > integ_thresh)
		{
			// ���ˣ�˵���ѵ��˺��ʵĻ��ַ壬������ַ������Ϣ
			gEcgDebugInfo.IntegPeak.nPeakPos = gTotalSampleBuff[peak_result.nPeakPos];
			gEcgDebugInfo.IntegPeak.nStartPos = gTotalSampleBuff[peak_result.nStartPos];
			gEcgDebugInfo.bIntegPeak = true;

			// ���ַ���ȴ�����ֵ�������ַ建����
			if (wIntegPeakNum < INTEG_PEAK_BUFF_LEN)
			{
				//��������δ������ֱ�����
				wIntegPeakBuff[wIntegPeakNum] = peak_result;
				wIntegPeakNum++;
			}
			else
			{
				//���������������������λ�������µĻ��ַ���䵽��β
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegPeakBuff[i] = wIntegPeakBuff[i + 1];
				}
				wIntegPeakBuff[INTEG_PEAK_BUFF_LEN - 1] = peak_result;
			}

			//2. ��һ���ж��Ƿ�Ϊ��Ч�Ļ��ַ�
			if (QrsComplexJudge())
			{
				// ���ˣ�˵���Ѿ���������Ч��QRS��
				// ά��QRS�����У��������QRS������
				QrsComplexUpdate();
				
				// ���²����������
				gEcgAlgResult.isQrsDetected = true;
				gEcgAlgResult.nQrsAmp = wCurQrsPeak.nQrsAmp;
				gEcgAlgResult.nQrsHpAmp = wCurQrsPeak.nQrsHpAmp;
				gEcgAlgResult.nQrsInterval = wCurQrsPeak.nRRInterval;
				gEcgAlgResult.nQrsWidth = wCurQrsPeak.nQrsWidth;
				gEcgAlgResult.bIsSolid = wCurQrsPeak.isSolid;
				
				// ����QRS�������׼ȷλ�ã��ṩ��PWTT����ʵʱ���㣨����ʾģʽ�޹أ�
				delay = 0;
				gEcgAlgResult.nQrsPeakPos = wCurQrsPeak.nPeakPosAll * 4 - delay * 2;
				
				// ��QRS����ʱ����
				wTimeNoQrs = 0;

				// ��ͣ����ʶ��λ
				gEcgAlgResult.isAsystole = false;

				// �����Ч���ַ�����
				if (wPeakAmpNum < INTEG_PEAK_BUFF_LEN)
				{
					//��������δ������ֱ�����
					wIntegPeakAmp[wPeakAmpNum] = peak_result.nPeakHight - peak_result.nStartHight;
					wPeakAmpNum++;
				}
				else
				{
					//���������������������λ�������µĻ��ַ���䵽��β
					for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
					{
						wIntegPeakAmp[i] = wIntegPeakAmp[i + 1];
					}
					wIntegPeakAmp[INTEG_PEAK_BUFF_LEN - 1] = peak_result.nPeakHight - peak_result.nStartHight;
				}

				// �����Ѳ���ֵ
				IntegThreshUpdate();

				return 1;
			}
		}
		else
		{

		}

		// ���ˣ�˵���÷�Ϊ�����壬�������������
		temp_amp = peak_result.nPeakHight - peak_result.nStartHight;
		if ((temp_amp * 2) < wIntegAverageAmp)
		{
			if (wNoiseAmpNum < INTEG_PEAK_BUFF_LEN)
			{
				//��������δ������ֱ�����
				wIntegNoiseAmp[wNoiseAmpNum] = temp_amp;
				wNoiseAmpNum++;
			}
			else
			{
				//���������������������λ�������µĻ��ַ���䵽��β
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
	//3. ������5��û���ѵ������������жϳ���ͣ��
	//=========================================================================
	if (ECG_SAMPLE_RATE * 5 <= wTimeNoQrs)
	{
		// ����5�룬�ж�Ϊͣ������ʼ��������������ж�Ϊͣ��
		EcgAlgGetResult(0, true);

		gEcgAlgResult.isAsystole = true;
	}
	else
	{
		gEcgAlgResult.isAsystole = false;
	}

	//δ��⵽QRS����ʱ������
	wTimeNoQrs++;
	return 0;
}

//ƽ�������ź��в����⺯��
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

				//ͨ��QRS���Ϊ60 - 200ms��ǰ�����أ��˴��������ֵ����Ϊ20 - 500ms
				//if ((peak_width > INTEG_PEAK_MAX_WIDTH) || (peak_width < INTEG_PEAK_MIN_WIDTH))
				if (peak_width < INTEG_PEAK_MIN_WIDTH)
				{
					// ���Է���������ֵ���ò����ʣ���Ϊ���ַ�������ܶ�λ���磬���¿�Ƚϴ󣬽�����������ֵ
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


// ���ַ��Ѳ���ֵ��ʼ������
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
		//�ҵ���һ�����ַ�󷽿�ʼ����ʱ�䣬���⽫��ʼ����Ч�źż���
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
			// ���Ѳ����������ַ建����
			if (wIntegPeakNum < INTEG_PEAK_BUFF_LEN)
			{
				//��������δ������ֱ�����
				wIntegPeakBuff[wIntegPeakNum] = peak_result;
				wIntegPeakNum++;
			}
			else
			{
				//���������������������λ�������µĻ��ַ���䵽��β
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegPeakBuff[i] = wIntegPeakBuff[i + 1];
				}
				wIntegPeakBuff[INTEG_PEAK_BUFF_LEN - 1] = peak_result;
			}

			// �����Ч���ַ��������
			if (wPeakAmpNum < INTEG_PEAK_BUFF_LEN)
			{
				//��������δ������ֱ�����
				wIntegPeakAmp[wPeakAmpNum] = temp_amp;
				wPeakAmpNum++;
			}
			else
			{
				//���������������������λ�������µĻ��ַ���䵽��β
				for (i = 0; i < INTEG_PEAK_BUFF_LEN - 1; i++)
				{
					wIntegPeakAmp[i] = wIntegPeakAmp[i + 1];
				}
				wIntegPeakAmp[INTEG_PEAK_BUFF_LEN - 1] = temp_amp;
			}

			integ_peak_num ++;

			flag_find_integ = true;
		}

		// ������Ч���ַ��ƽ������
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
		// ��������������������ֵ��ʼ��
		// 1. ���ѵ���һ������ʼ�������룻2. �����ѵ�5�����ַ�
		
		// ��ȡ��ֵ���δ�ֵ
		mid_value = SortAndSearch(wIntegPeakAmp, INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN + 1) / 2);
		sec_value = SortAndSearch(wIntegPeakAmp, INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN - 1));

		// �źŷ���ֵΪ��ֵ��δ�ֵ�ľ�ֵ����Ϊ����ֵ���Զ�������С�����ܻ�©���źŷ壬�������ų�������
		// ��Ϊ��ֵ��һ�룬��Ϊ�����5������ȷ����ô����ֵ�������׵���©�죬���Ҹ��ź��������������
		wIntegSigThresh = (mid_value + sec_value) / 4;
		// ��������ֵΪ�źŷ���ֵ��1/8
		wIntegNoiseThresh = mid_value / 8;

		// ����źŷ�������建����
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

// ���»��ַ��Ѳ���ֵ
void IntegThreshUpdate(void)
{
	int peak_mid = 0, noise_mid = 0, peak_pre = 0, peak_last = 0, temp = 0, integ_sum = 0, i = 0;

	//=========================================================================
	//1. ��ȡ�źŷ����������ֵ����ȡ���µ������źŷ�ֵ���Ա������ж�
	//=========================================================================
	peak_mid  = SortAndSearch(wIntegPeakAmp,  INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN + 1) / 2);				//�źŷ���ֵ
	noise_mid = SortAndSearch(wIntegNoiseAmp, INTEG_PEAK_BUFF_LEN, (INTEG_PEAK_BUFF_LEN + 1) / 2);				//��������ֵ

	if (2 > wPeakAmpNum)
	{
		return;
	}
	else
	{
		// ��ȡ���¼������źŷ���ȣ����ж��ź�ͻ��
		peak_last = wIntegPeakAmp[wPeakAmpNum - 1];
		peak_pre = wIntegPeakAmp[wPeakAmpNum- 2];
	}
	

	//=========================================================================
	//2. �ж��ź��Ƿ����ͻ��
	//=========================================================================
	if (((peak_mid > peak_last * 4) && (peak_mid > peak_pre * 4)) || ((peak_mid * 4 < peak_last) && (peak_mid * 4 < peak_pre)))
	{
		// �ź�ͻ�䣬������������źŷ�ľ�ֵ������ֵ
		temp = (peak_last + peak_pre) / 2 - noise_mid;
	}
	else
	{
		// �ź�δͻ�䣬���źŷ���ֵ������ֵ
		temp = peak_mid - noise_mid;
	}

	//=========================================================================
	//3. ���㲢������ֵ
	//=========================================================================
	// �����źš��������ֵ��0.1875 �������࣬������ֵΪ��ֵ��0.5��modified by mao 14.05.03
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
	//4. ������Ч���ַ�ľ�ֵ�����⽫©��Ĳ������������
	//=========================================================================
	// ������ֵ
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


// QRS����Ч���жϺ���
int QrsComplexJudge(void)
{
	BP_DIFF_PEAK diff_peak;
	QRS_PEAK_POS band_peak;
	QRS_PEAK_POS view_peak;
	int flag = 0, temp_value = 0;

	//=========================================================================
	//1. �����ڴ�ͨ����ź�����������źŵ����ֵ����Сֵ�������ȣ������Ϲ�����Ӧ�ڴ�ͨ�źŵĲ���λ��
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
		// ���ˣ�˵��ȷ��QRS���Ѳ��ɹ�

		// ��ַ�����ۼӣ�Ϊ�������������׼��
		temp_value = gDiffDataBuff[diff_peak.nDiffMaxPos] - gDiffDataBuff[diff_peak.nDiffMinPos];
		if (0 <= temp_value)
		{
			wAveDiffPeak = wAveDiffPeak + temp_value;
		}
		else
		{
			// ������temp_valueӦ�ö��Ǵ�����ģ�������һ����˵���������
			wAveDiffPeak = wAveDiffPeak - temp_value;
		}
		wAveDiffPeakNum++;

		// �����ʾ����Ѳ�������Ϣ
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

// �ڴ�ͨ����ź��Ͻ���QRS��λ�ó�����λ
int BpDiffDataPeakDetect(BP_DIFF_PEAK *diff_peak)
{
	int temp_pos = 0, max_pos = 0, min_pos = 0, max_value = 0, min_value = 0;
	int i = 0, pos_diff = 0, slop_thresh = 0, wind_begin = 0, wind_end = 0, wind_len = 0;
	int last_state = 0, cur_state = 0, last_value = 0, cur_value = 0, slop_cnt = 0, peak_cnt = 0;

	// ��ʼ�������Ϣ
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

	// ȷ����������ʼ����ֹλ��
	wind_begin = mod((wIntegPeakBuff[wIntegPeakNum - 1].nPeakPos - wind_len), ECG_DATA_BUFF_LEN);
	wind_end = gCurBuffPtr;
	
	//=========================================================================
	//1. ��������ź������ֵ���δ�ֵ����Сֵ��λ��
	//=========================================================================
	// ������������ʼ�㿪ʼ�����������λ�����Сֵλ��
	temp_pos = wind_begin;
	for(i = 0; i < wind_len; i++)
	{
		cur_value = gDiffDataBuff[temp_pos];

		if(cur_value > max_value)
		{
			//���Աȣ�ȷ�����ֵλ��
			if(last_value <= 0)
			{
				// �����һ����ֵ����Сֵ����ôֻ�е���ǰֵ����ǰ��һ������ֵ��2��ʱ�����Ž��и���
				// ������Ե�Ŀ���Ƿ�ֹQRS��������������������������QRS��Ϊ����
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
			// ���Աȣ�ȷ����Сֵλ��
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

		// ָ������
		temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
	}

	// �жϸ÷��Ƿ���Ч
	if (gDiffDataBuff[max_pos] * gDiffDataBuff[min_pos] > 0)
	{
		// �����С���ֵͬ��˵���÷���Ч
		return 0;
	}

	//=========================================================================
	//2. ���ˣ�˵���ò�ַ���Ч���жϸ�Peak�����ͣ������������Сб�ʵ�λ��
	//=========================================================================
	pos_diff = gTotalSampleBuff[max_pos] - gTotalSampleBuff[min_pos];
	diff_peak->nDiffMaxPos = max_pos;
	diff_peak->nDiffMinPos = min_pos;

	if (0 < pos_diff)
	{
		// ��Сֵ��ǰ�����ֵ�ں�˵���Ǹ����
		diff_peak->nDiffStartPos = min_pos;
		diff_peak->nDiffEndPos = max_pos;
		wCurQrsPeak.isPeakBefore = 0;
	}
	else if(0 > pos_diff)
	{
		// ���ֵ��ǰ����Сֵ�ں�˵���������
		diff_peak->nDiffStartPos = max_pos;
		diff_peak->nDiffEndPos = min_pos;
		wCurQrsPeak.isPeakBefore = 1;
	}
	else
	{
		// �����Сֵ��ͬһλ�ã�������ֱ�ӷ���
		return 0;
	}

	//=========================================================================
	//3. ȷ�������λ�ã���Ӧ��ͨ�źŵĲ���λ��
	//=========================================================================
	temp_pos = diff_peak->nDiffStartPos;
	last_value = gDiffDataBuff[temp_pos];
	while (temp_pos != diff_peak->nDiffEndPos)
	{
		cur_value = gDiffDataBuff[temp_pos];
		if (cur_value * last_value <= 0)
		{
			// ��ǰ������һ�㲻ͬ�ţ�˵���ҵ�����㣬��¼�����
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
	//4. ȷ�����λ�ã���������С�����ֵ1/5�������㣩
	//=========================================================================
	// �趨��ֵ
	slop_thresh = Abs(gDiffDataBuff[diff_peak->nDiffStartPos]) / 5;

	// ȷ��������ʼλ�ã���ʱnDiffStartPos��б�����/��Сֵλ��
	temp_pos = diff_peak->nDiffStartPos;
	cur_value = gDiffDataBuff[temp_pos];
	last_value = cur_value;
	slop_cnt = 0;

	// ����ʼλ�ÿ�ʼ��ǰ������ȷ�����
	while (temp_pos != wind_begin)
	{
		cur_value = gDiffDataBuff[temp_pos];

		if (0 >= (cur_value * last_value))
		{
			//������б�ʹ���㣬��Ӧ��ͨ�źŷ�ֵ�㣬��Ϊ�ҵ�����㣬����
			break;
		}
		else
		{
			// �����ǰֵС�����б�ʵ�1/5������������
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
				// ����6��С����ֵ����Ϊ�ҵ�����㣬����
				break;
			}

			// ά�����м���ز���
			last_value = cur_value;
			temp_pos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}
	}

	// ������������ʼλ�û�û�ҵ���㣬˵�����������˳�
	if ((temp_pos != diff_peak->nDiffStartPos) && (temp_pos != wind_begin))
	{
		// ��ʱnDiffStartPos����㸽���ļ�ֵ��λ�ã�temp_pos�����λ��
		max_value = Abs(gDiffDataBuff[diff_peak->nDiffStartPos] - gDiffDataBuff[temp_pos]);
		diff_peak->nDiffStartPos = temp_pos;
	}
	else
	{
		return 0;
	}

	//=========================================================================
	//5. ȷ���յ�λ�ã���������С�����ֵ1/5�������㣩
	//=========================================================================
	// �趨��ֵ
	slop_thresh = Abs(gDiffDataBuff[diff_peak->nDiffEndPos]) / 5;

	// ȷ��������ʼλ�ã���ʱnDiffStartPos��б�����/��Сֵλ��
	temp_pos = diff_peak->nDiffEndPos;
	cur_value = gDiffDataBuff[temp_pos];
	last_value = cur_value;
	slop_cnt = 0;

	// ����ʼλ�ÿ�ʼ����������ȷ���յ�
	while (temp_pos != wind_end)
	{
		cur_value = gDiffDataBuff[temp_pos];

		if (0 >= (cur_value * last_value))
		{
			//������б�ʹ���㣬��Ӧ��ͨ�źŷ�ֵ�㣬��Ϊ�ҵ�����㣬����
			break;
		}
		else
		{
			// �����ǰֵС�����б�ʵ�1/5������������
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
				// ����6��С����ֵ����Ϊ�ҵ�����㣬����
				break;
			}

			// ά�����м���ز���
			last_value = cur_value;
			temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}
	}

	// ������������ʼλ�û�û�ҵ���㣬˵�����������˳�
	if ((temp_pos != diff_peak->nDiffEndPos) && (temp_pos != wind_end))
	{
		// ��ʱnDiffEndPos���յ㸽���ļ�ֵ��λ�ã�temp_pos���յ�λ��
		min_value = Abs(gDiffDataBuff[diff_peak->nDiffEndPos] - gDiffDataBuff[temp_pos]);
		diff_peak->nDiffEndPos = temp_pos;
	}
	else
	{
		return 0;
	}

	//=========================================================================
	//6. ��̬�жϣ��������С�������̫�󣬻���ֹ����ֵ����࣬����Ϊ���
	//=========================================================================
	//6.1 ͨ����˵������QRS���������κ��½��ε�б��Ӧ�ýϽӽ�������������Сб�ʷ��ȱ�ֵ��������������λ�Ư����
	if ((max_value * 5 < min_value) || (max_value > min_value * 5))
	{
		//�����Сб�ʷ��ȱ�ֵ����5������Ϊ����
		return 0;
	}

	//6.2 �Դ�ͨ�����˵��һ�鼫ֵ����2���壬ͨ��Ӧ�ò�����4���塣����5���弰���ϣ���˵�������п������𵴸�������
	//i = mod((diff_peak->nDiffStartPos + 2), ECG_DATA_BUFF_LEN);
	//temp_pos   = mod((diff_peak->nDiffEndPos - 2), ECG_DATA_BUFF_LEN);

	i = mod((diff_peak->nDiffStartPos + 1), ECG_DATA_BUFF_LEN);
	cur_value = 0;
	cur_state = 0;
	last_value = 0;
	last_state = 0;
	peak_cnt = 0;

	// ͳ�Ʒ�ȸ���
	while (i != diff_peak->nDiffEndPos)
	{
		// ��ȡ�м���ֵ
		cur_value = gDiffDataBuff[i];

		// ���Ƽ�⣬����������Ϊ1�������½���Ϊ-1
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

		// ��ֵ��⣬������״̬�����ı䣬˵���з�ȴ���
		if (0 > (cur_state * last_state))
		{
			peak_cnt++;
		}

		last_state = cur_state;
		last_value = cur_value;

		/*
		// ��ȡ�м���ֵ
		cur_value = gDiffDataBuff[i];

		// ��ȡǰһ���ֵ
		wind_begin = mod((i - 1), ECG_DATA_BUFF_LEN);
		min_value = gDiffDataBuff[wind_begin];

		// ��ȡ��һ���ֵ
		wind_end = mod((i + 1), ECG_DATA_BUFF_LEN);
		max_value = gDiffDataBuff[wind_end];

		// �ж��Ƿ񲨷�
		if ((cur_value >= min_value) && (cur_value >= max_value))
		{
			// �м�����ǰ��㣬Ϊ����
			peak_cnt ++;
		}
		else if ((cur_value <= min_value) && (cur_value <= max_value))
		{
			// �м��С��ǰ��㣬Ϊ����
			peak_cnt ++;
		}
		else
		{
			// �Ƿ��
		}
		*/

		i = LoopInc(i, 1, ECG_DATA_BUFF_LEN);
	}
	
	// �ж��Ƿ����
	if (peak_cnt > 6)
	{
		return 0;
	}


	// ���ˣ�˵����ַ������ɹ������������Ϣ
	gEcgDebugInfo.BpDiffPeak.nDiffStartPos = gTotalSampleBuff[diff_peak->nDiffStartPos];
	gEcgDebugInfo.BpDiffPeak.nDiffZeroPos = gTotalSampleBuff[diff_peak->nDiffZeroPos];
	gEcgDebugInfo.BpDiffPeak.nDiffEndPos = gTotalSampleBuff[diff_peak->nDiffEndPos];
	gEcgDebugInfo.BpDiffPeak.nDiffMaxPos = gTotalSampleBuff[diff_peak->nDiffMaxPos];
	gEcgDebugInfo.BpDiffPeak.nDiffMinPos = gTotalSampleBuff[diff_peak->nDiffMinPos];

	return 1;
}


// �ڴ�ͨ�����ϣ����QRS����λ��
int BandPeakDetect(BP_DIFF_PEAK *diff_peak, QRS_PEAK_POS *band_peak)
{
	int temp_pos = 0, cur_value = 0, max_value = 0, min_value = 0, max_pos = 0, min_pos = 0;
	int last_value = 0, next_value = 0, search_end = 0, search_start = 0, i = 0;
	bool is_peak = false, is_valley = false;
	QRS_PEAK_POS high_peak;

	// �����ͨ�ź��ϵ�Peakλ�ã����ڵ���
	band_peak->nStartPos = mod((diff_peak->nDiffStartPos - DIFF_PASS_DELAY), ECG_DATA_BUFF_LEN);
	band_peak->nPeakPos = mod((diff_peak->nDiffZeroPos - DIFF_PASS_DELAY), ECG_DATA_BUFF_LEN);
	band_peak->nEndPos  = mod((diff_peak->nDiffEndPos   - DIFF_PASS_DELAY), ECG_DATA_BUFF_LEN);

	//=========================================================================
	//1. ���ݲ�����̬��׼ȷ��λ�塢�ȵ�λ��
	//=========================================================================
	if (wCurQrsPeak.isPeakBefore)
	{
		// ������ǰ
		// ����źŵ�5���㣬��㡢���㡢����㡢       ��С�㡢�յ�
		// ��Ӧ�ڴ�ͨ5���㣬��㡢�����㡢�����ֵ�㡢�½��㡢�ȵ�
		// ���Բ��÷�ֵ����յ���Ϊ�����Ĳο���
		max_pos = band_peak->nPeakPos;
		min_pos = band_peak->nEndPos;	
	}
	else
	{
		// �����ں�
		// ����źŵ�5���㣬��㡢    ��С�㡢����㡢       ���㡢�յ�
		// ��Ӧ�ڴ�ͨ5���㣬���㡢�½��㡢�����ֵ�㡢�����㡢�յ�
		// ���Բ������ͷ�ֵ����Ϊ�����Ĳο���
		max_pos = band_peak->nStartPos;
		min_pos = band_peak->nPeakPos;
	}

	// 1.1 ��max_posΪ��׼��ǰ������5���㣬Ѱ�����ֵ
	temp_pos = mod((max_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((max_pos + 5), ECG_DATA_BUFF_LEN);
	max_value = gBpDataBuff[temp_pos];

	// ����ʮ���㣬��λ����λ��
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

	// 1.2 ��min_posΪ��׼��ǰ������5���㣬Ѱ�ҹ�ֵ��
	temp_pos = mod((min_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((min_pos + 5), ECG_DATA_BUFF_LEN);
	min_value = gBpDataBuff[temp_pos];

	// ����ʮ���㣬��λ����λ��
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
		// ������ǰ
		 band_peak->nPeakPos = max_pos;
		 band_peak->nEndPos = min_pos;	
	}
	else
	{
		// ������ǰ
		 band_peak->nStartPos = max_pos;
		 band_peak->nPeakPos = min_pos;
	}


	//=========================================================================
	//2.�жϣ�����Ͳ����Ƿ���ڣ��п��ܲ����ڲ��ȣ�ֱ����������Сֵ��Զ�벨�壬����Ϊ�յ����������ݣ�
	//     todo����һ���ֵĽ��û������
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
	//3. �жϸ�QRS��������
	//=========================================================================
	if (wCurQrsPeak.isPeakBefore)
	{
		// ��������ǰ��ֱ���ж�Ϊ����QRS��
		wCurQrsPeak.isPositive = true;
	}
	else
	{
		// �����ں�����Ҫ���ǲ��岨�ȵķ������������QRS���ķ���
		// ��ʱnStartPos�ǲ��壬nPeakPos�ǲ��ȣ�nEndPos���յ�
		max_value = Abs(gBpDataBuff[band_peak->nStartPos] - gBpDataBuff[band_peak->nEndPos]);
		min_value = Abs(gBpDataBuff[band_peak->nPeakPos] - gBpDataBuff[band_peak->nEndPos]);
		
		// ��������ȴ��ڲ��ȷ��ȵ�2/3������Ϊ������QRS��
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
	// 4. �����ͨQRS���ķ��ȣ����ﻹ�Ǽ�����ֵ����Ϊ����������Ƚ����ѣ�������ɽ϶�����
	//=========================================================================
	// ȷ���������򣬴����ǰ2�㿪ʼ������ֵ���2�����
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

	// ���ͷ��λ�ø�ֵ
	high_peak.nStartPos =min_pos;
	high_peak.nPeakPos = max_pos;

	wCurQrsPeak.nQrsHpAmp = Abs(gHpDataBuff[high_peak.nPeakPos] - gHpDataBuff[high_peak.nStartPos]);


	//=========================================================================
	// 5. ���ˣ�˵����ͨ�������ɹ������������Ϣ
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

// ����ʾ�����Ͻ������յ�QRS�������㶨λ
int ViewPeakDetect(QRS_PEAK_POS *band_peak, QRS_PEAK_POS *view_peak)
{
	int temp_pos = 0, search_end = 0, cur_value = 0, max_value = 0, min_value = 0, max_pos = 0, min_pos = 0;
	int last_value = 0, next_value = 0, cur_slope = 0, max_slope = 0, slope_cnt = 0;
	bool is_peak = false, is_valley = false;

	// ����λ�ù�ϵ�����ȴ��Լ�����ʾ�ź��ϵ�Qrs������㡢�յ�Ͳ���λ��
	view_peak->nStartPos = mod((band_peak->nStartPos - (HIGH_PASS_DELAY + LOW_PASS_DELAY)), ECG_DATA_BUFF_LEN);
	view_peak->nPeakPos = mod((band_peak->nPeakPos - (HIGH_PASS_DELAY + LOW_PASS_DELAY)), ECG_DATA_BUFF_LEN);
	view_peak->nEndPos  = mod((band_peak->nEndPos   - (HIGH_PASS_DELAY + LOW_PASS_DELAY)), ECG_DATA_BUFF_LEN);

	//=========================================================================
	//1. ���ݲ�����̬��׼ȷ��λ�塢�ȵ�λ��
	//=========================================================================
	if (wCurQrsPeak.isPeakBefore)
	{
		// ������ǰ
		max_pos = view_peak->nPeakPos;
		min_pos = view_peak->nEndPos;	
	}
	else
	{
		// ������ǰ
		max_pos = view_peak->nStartPos;
		min_pos = view_peak->nPeakPos;
	}

	// ��peakposΪ��׼��ǰ������5���㣬Ѱ�����ֵ
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

	// ��EndPosΪ��׼��ǰ������5���㣬Ѱ�ҹ�ֵ��
	temp_pos = mod((min_pos - 5), ECG_DATA_BUFF_LEN);
	search_end = mod((min_pos + 5), ECG_DATA_BUFF_LEN);
	min_value = gBpDataBuff[temp_pos];

	// ����ʮ���㣬��λ���塢����λ��
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
		// ������ǰ
		view_peak->nPeakPos = max_pos;
		view_peak->nEndPos = min_pos;	
	}
	else
	{
		// ������ǰ
		view_peak->nStartPos = max_pos;
		view_peak->nPeakPos = min_pos;
	}

	// �жϣ�����Ͳ����Ƿ���ڣ��п��ܲ����ڲ��ȣ�ֱ����������Сֵ��Զ�벨�壬����Ϊ�յ����������ݣ�
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
	//2. ��λ���λ��  todo������QRS��û�н���ϸ���Ѳ�����������
	//=========================================================================
	if (wCurQrsPeak.isPositive && !wCurQrsPeak.isPeakBefore)
	{
		// qrs��Ϊ���򲨣��Ҳ�����ǰ����ʱnStartʵ������nPeak�������δ��⵽
		// ��nStart�㣬ʵ����R����ֵ�㣬�趨Ϊ�Ѳ����
		temp_pos = view_peak->nStartPos;

		// �����Ѳ��յ㣬�ݶ�ΪQRS�벨���15��		todo�������ƽ��RR���ڵ���
		search_end = mod((temp_pos - 15), ECG_DATA_BUFF_LEN);
		max_slope = 0;
		slope_cnt = 0;

		while (temp_pos != search_end)
		{
			// ��ȡ��ǰ�㣬����ǰ������ķ���
			cur_value = gViewDataBuff[temp_pos];
			last_value = gViewDataBuff[mod((temp_pos - 1), ECG_DATA_BUFF_LEN)];
			next_value = gViewDataBuff[mod((temp_pos + 1), ECG_DATA_BUFF_LEN)];

			// ���������ȣ�����Ϊ�ҵ������λ��
			if ((cur_value <= last_value) && (cur_value <= next_value))
			{
				break;
			}

			// �������㣬б��С�����б��1/5
			cur_slope = next_value - last_value;
			if (cur_slope >= max_slope)
			{
				// �������б��ֵ
				max_slope = cur_slope;
				slope_cnt = 0;
			}
			else if (cur_slope * 5 <= max_slope)
			{
				// ��С�����б�ʵ�1/5����slope��������
				slope_cnt ++;

				// ���������������ϵĵ㣬��λΪ���
				if (3 <= slope_cnt)
				{
					break;
				}
			}
			else
			{
				// б��С�����б�ʣ����������б�ʵ�1/5����������
				slope_cnt = 0;
			}

			temp_pos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}

		// ���¶������ͷ�ֵ��
		view_peak->nPeakPos = view_peak->nStartPos;
		view_peak->nStartPos = temp_pos;
	}
	else if (wCurQrsPeak.isPositive && wCurQrsPeak.isPeakBefore)
	{
		// ��ʱnPeak���ǲ���λ�ã�nStartΪ���λ��
		// ��nStartΪ��ͨ���λ�ã���Ҫϸ������

		// ��nPeak��ʼǰ���������յ�ΪnStart
		temp_pos = view_peak->nPeakPos;
		search_end = view_peak->nStartPos;
		max_slope = 0;
		slope_cnt = 0;

		while (temp_pos != search_end)
		{
			// ��ȡ��ǰ�㣬����ǰ������ķ���
			cur_value = gViewDataBuff[temp_pos];
			last_value = gViewDataBuff[mod((temp_pos - 1), ECG_DATA_BUFF_LEN)];
			next_value = gViewDataBuff[mod((temp_pos + 1), ECG_DATA_BUFF_LEN)];

			// �������㣬б��С�����б��1/5
			cur_slope = next_value - last_value;
			if (cur_slope >= max_slope)
			{
				// �������б��ֵ
				max_slope = cur_slope;
				slope_cnt = 0;
			}
			else if (cur_slope * 5 <= max_slope)
			{
				// ��С�����б�ʵ�1/5����slope��������
				slope_cnt ++;

				// ���������������ϵĵ㣬��λΪ���
				if (3 <= slope_cnt)
				{
					break;
				}
			}
			else
			{
				// б��С�����б�ʣ����������б�ʵ�1/5����������
				slope_cnt = 0;
			}

			temp_pos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}

		// ���¶������
		if (view_peak->nStartPos == temp_pos)
		{
			// ˵��һֱ��������㶼û�ҵ������½��㣬�򲻸��£��Ե�ǰ���Ϊ���
		}
		else
		{
			// �����֮ǰ�ѵ���б�������½��㣬��������
			// ��Ϊ����������С�����б�ʵĵ㣬���Ҫȡ��һ��
			view_peak->nStartPos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
			//view_peak->nStartPos = temp_pos;
		}
	}
	else
	{
		;
	}

	//=========================================================================
	//3. ��λ�յ�λ��   todo������QRS��û�н���ϸ���Ѳ�����������
	//=========================================================================
	if (wCurQrsPeak.isPositive)
	{
		// qrs��Ϊ���򲨣���ȷ��λ�յ�λ��
		// �趨nStartΪ�Ѳ����λ�ã�nEndΪ�Ѳ��յ�λ��
		temp_pos = view_peak->nPeakPos;

		// �����Ѳ��յ㣬�ݶ�ΪQRS�벨���15��		todo�������ƽ��RR���ڵ���
		search_end = view_peak->nEndPos;
		max_slope = 0;
		slope_cnt = 0;

		while (temp_pos != search_end)
		{
			// ��ȡ��ǰ�㣬����ǰ������ķ���
			cur_value = gViewDataBuff[temp_pos];
			last_value = gViewDataBuff[mod((temp_pos - 1), ECG_DATA_BUFF_LEN)];
			next_value = gViewDataBuff[mod((temp_pos + 1), ECG_DATA_BUFF_LEN)];

			// �������㣬б��С�����б��1/5
			cur_slope = last_value - next_value;
			if (cur_slope >= max_slope)
			{
				// �������б��ֵ
				max_slope = cur_slope;
				slope_cnt = 0;
			}
			else if (cur_slope * 5 <= max_slope)
			{
				// ��С�����б�ʵ�1/5����slope��������
				slope_cnt ++;

				// ���������������ϵĵ㣬��λΪ���
				if (3 <= slope_cnt)
				{
					break;
				}
			}
			else
			{
				// б��С�����б�ʣ����������б�ʵ�1/5����������
				slope_cnt = 0;
			}

			temp_pos = LoopInc(temp_pos, 1, ECG_DATA_BUFF_LEN);
		}

		// ���¶����յ�
		if (view_peak->nEndPos == temp_pos)
		{
			// ˵��һֱ�������յ㶼û�ҵ������½��㣬�򲻸��£��Ե�ǰ�յ�Ϊ�յ�
		}
		else
		{
			// �����֮ǰ�ѵ���б�������½��㣬������յ�
			// ��Ϊ����������С�����б�ʵĵ㣬���Ҫȡ��һ��
			view_peak->nEndPos = LoopDec(temp_pos, 1, ECG_DATA_BUFF_LEN);
			//view_peak->nEndPos = temp_pos;
		}
	}

	return 1;
}

// ���QRS���ų�
int QrsComplexExclusion(QRS_PEAK_POS *view_peak, QRS_PEAK_POS *band_peak)
{
	int min_thresh = 0, max_thresh = 0, i = 0, cur_value = 0, last_ptr = 0, last_rr = 0;

	//=========================================================================
	//1. ����QRS������ز���
	//=========================================================================
	// Qrs������ֵ��ֵ
	wCurQrsPeak.nStartPos = view_peak->nStartPos;
	wCurQrsPeak.nPeakPos = view_peak->nPeakPos;
	wCurQrsPeak.nEndPos  = view_peak->nEndPos;

	wCurQrsPeak.nPeakPosAll = gTotalSampleBuff[wCurQrsPeak.nPeakPos];

	// ����ͨ�źŵĲ���λ�ñ�����������������ķ���
	wCurQrsPeak.nBpPeakPos = band_peak->nPeakPos;

	// �������
	// wCurQrsPeak.nQrsAmp = Abs(gViewDataBuff[wCurQrsPeak.nPeakPos] - gViewDataBuff[wCurQrsPeak.nStartPos]);
	// ��PeakPos - StartPos�ķ������������⣬�����λ��׼�����Ȼ�ǳ�С������©��
	// ��˴˴����������յ�������Сֵ�����Ըò�ֵ��Ϊ����ֵ
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

	// ������
	wCurQrsPeak.nQrsWidth = gTotalSampleBuff[wCurQrsPeak.nEndPos] - gTotalSampleBuff[wCurQrsPeak.nStartPos];
	if (0 >= wCurQrsPeak.nQrsWidth)
	{
		//Qrs�����С�ڵ���0��˵���ò������⣬ɾ��
		return 0;
	}

	// ������ǰһQRS����RR����
	if (0 != gQrsPeakNum)
	{
		wCurQrsPeak.nRRInterval = wCurQrsPeak.nPeakPosAll - gQrsPeakBuff[gQrsPeakNum - 1].nPeakPosAll;

		if (0 >= wCurQrsPeak.nRRInterval)
		{
			//RR����С�ڵ���0�������⣬ɾ��
			return 0;
		}
	}
	else
	{
		// ��ʱ��û��QRS���������������Ϊ0
		wCurQrsPeak.nRRInterval = 0;
	}

	//=========================================================================
	//2. �����жϣ��ų�һЩ���߻���͵ĸ���
	//=========================================================================
	// �������С����С����������ֵ��100LSB��Լ0.25mV / 4000LSB��10mV�������ų���
	if ((QRS_PEAK_MIN_THRESH > wCurQrsPeak.nQrsAmp) || (QRS_PEAK_MAX_THRESH < wCurQrsPeak.nQrsAmp))
	{
		return 0;
	}

	// ���ƽ�����Ⱥ�ƽ�����ھ���Ϊ�㣬����ƽ�����Ƚ��бȽ�
	if ((0 < gQrsInfo.nAveQrsAmp) && (0 < gQrsInfo.nAveInterval))
	{
		min_thresh = 0;
		max_thresh = 0;

		// ������ʷ������ֵ
		if (0 == gQrsPeakNum)
		{
			// ����ط���������Ƶģ�
			// ���ƽ����Ⱥ�ƽ�����ڶ��Ѿ����ڣ�����QRS����Ϊ0��˵��������
			// ����ĳ�ʼ�׶Σ�������ΪnRRIntervalΪ0�������ж���������
			// �����п���©���ʼ�׶ε�PVC����˵�������RR���ڴ���
			min_thresh = gQrsInfo.nAveQrsAmp / 3;
			max_thresh = gQrsInfo.nAveQrsAmp * 3;
		}
		else if (wCurQrsPeak.nRRInterval * 3 < gQrsInfo.nAveInterval * 2)
		{
			//��rr���ڱ�ƽ��rr���ڵ�2/3��Сʱ����ֵ���ø���һ�㣬Ҫ����ƽ�����ȵ�1/2��2��֮��
			min_thresh = gQrsInfo.nAveQrsAmp / 2;
			max_thresh = gQrsInfo.nAveQrsAmp * 2;
		}
		else if (wCurQrsPeak.nRRInterval < gQrsInfo.nAveInterval)
		{
			//rr����С��ƽ�����ڣ���ſ���ֵ��1/3��3��֮�䶼����
			min_thresh = gQrsInfo.nAveQrsAmp / 3;
			max_thresh = gQrsInfo.nAveQrsAmp * 3;
		}
		else
		{
			//����ʱRR�����Ѿ�����ƽ�����ڣ���������ֵ���ƣ����ⳤʱ���Ѳ�ʧ��
			min_thresh = QRS_PEAK_MIN_THRESH;
			max_thresh = QRS_PEAK_MAX_THRESH;
		}

		if ((wCurQrsPeak.nQrsAmp < min_thresh) || (wCurQrsPeak.nQrsAmp > max_thresh))
		{
			// �򳬳�ƽ��������ֵ�����ų���
			return 0;
		}
	}

	//=========================================================================
	//3. ����жϣ���Ҫ�������ų�T�����
	//=========================================================================
	// ���ƽ��������ƽ����Ⱦ���Ϊ�㣬����п�ȵ��жϣ����޳�T��
	if ((0 < gQrsInfo.nAveWidth) && (0 < gQrsInfo.nAveInterval))
	{
		min_thresh = 0;
		max_thresh = 0;

		// ���ÿ����ֵ
		if (0 == gQrsPeakNum)
		{
			// ����ط���������Ƶģ�
			// ���ƽ����Ⱥ�ƽ�����ڶ��Ѿ����ڣ�����QRS����Ϊ0��˵��������
			// ����ĳ�ʼ�׶Σ�������ΪnRRIntervalΪ0�������ж���������
			// �����п���©���ʼ�׶ε�PVC����˵�������RR���ڴ���
			min_thresh = gQrsInfo.nAveWidth / 3;
			max_thresh = gQrsInfo.nAveWidth * 3;
		}
		else if (wCurQrsPeak.nRRInterval * 2 < gQrsInfo.nAveInterval)
		{
			//��rr���ڱ�ƽ��rr���ڵ�1/2��Сʱ����ֵ���ø���һ�㣬Ҫ����ƽ�����ȵ�1/2��1.5��֮��
			min_thresh = gQrsInfo.nAveWidth / 2;
			max_thresh = gQrsInfo.nAveWidth * 3 / 2;
		}
		else if (wCurQrsPeak.nRRInterval < gQrsInfo.nAveInterval)
		{
			//rr����С��ƽ�����ڣ���ſ���ֵ��1/3��3��֮�䶼����
			min_thresh = gQrsInfo.nAveWidth / 3;
			max_thresh = gQrsInfo.nAveWidth * 3;
		}
		else
		{
			//����ʱRR�����Ѿ�����ƽ�����ڣ���������ֵ���ƣ����ⳤʱ���Ѳ�ʧ��
			min_thresh = 1;
			max_thresh = 100;
		}

		if ((wCurQrsPeak.nQrsWidth < min_thresh) || (wCurQrsPeak.nQrsWidth > max_thresh))
		{
			// �����������ֵ�����ų���
			return 0;
		}
	}


	//=========================================================================
	// 4. RR�����жϣ����������������һЩ�����RR���ڲ������
	//=========================================================================
	// ����Ĭ��Ϊfalse
	wCurQrsPeak.isSolid = false;

	// �������������ж�Ϊ�ɿ�
	// 1. ��ƽ��RR���ڵ�0.5 - 1.5֮��
	// 2. ����һ��RR���ڵ�0.8 - 1.2֮��
	if (0 != gQrsInfo.nAveInterval)
	{
		if ((gQrsInfo.nAveInterval <= wCurQrsPeak.nRRInterval * 2) && (gQrsInfo.nAveInterval * 3 >= wCurQrsPeak.nRRInterval * 2))
		{
			// ��ȡ��һ��QRS����rr���ڣ���ʱ��Ӧ�ò���õ���Чֵ
			last_rr = gQrsPeakBuff[gQrsPeakNum - 1].nRRInterval;
			
			if ((wCurQrsPeak.nRRInterval * 10 >= last_rr * 8) && (wCurQrsPeak.nRRInterval * 10 <= last_rr * 12))
			{
				wCurQrsPeak.isSolid = true;
			}
		}
	}

	//=========================================================================
	// б���ų����¼��QRS����б�ʲ�ӦС����ʷƽ��б�ʵ�1/5    todo
	//=========================================================================

	return 1;
}

// ά��QRS�����У��������QRS������
void QrsComplexUpdate(void)
{
	int i = 0, sum = 0, cunt = 0, temp = 0;

	//=========================================================================
	//1. ���QRS��������
	//=========================================================================
	if (gQrsPeakNum < QRS_PEAK_BUFF_LEN)
	{
		//��������δ������ֱ�����
		gQrsPeakBuff[gQrsPeakNum] = wCurQrsPeak;
		gQrsPeakNum++;
	}
	else
	{
		//���������������������λ�������µ�QRS����䵽��β
		for (i = 0; i < QRS_PEAK_BUFF_LEN - 1; i++)
		{
			gQrsPeakBuff[i] = gQrsPeakBuff[i + 1];
		}
		gQrsPeakBuff[QRS_PEAK_BUFF_LEN - 1] = wCurQrsPeak;
	}

	// ����ǰ��QRS�������㷨ʵʱ�����Ϣ
	gEcgAlgResult.EcgOntimeQrs = wCurQrsPeak;

	//=========================================================================
	//2. ����QRS����ȫ����������  todo��Ӧ�ò�������ȡ��ֵ�÷�������
	//=========================================================================
	if (4 >= gQrsPeakNum)
	{
		// QRS��̫�ٵ�����£������д˼���
		return;
	}

	// ����ƽ�����
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

	// ����ƽ������
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

	// ����ƽ��rr����
	sum = 0;
	cunt = 0;
	gQrsInfo.nAveInterval = 0;
	for (i = gQrsPeakNum > 5 ? gQrsPeakNum - 5:0; i < gQrsPeakNum; i++)
	{
		temp = gQrsPeakBuff[i].nRRInterval;
		// �����ǰ��RR������2s��30BPM����200ms��300BPM��֮�䣬����Լ���
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


// �������������
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
	// 1. ����ԭʼ�źŵı�׼��
	//=========================================================================
	// ֱ����һ������ֵ�����ж�̫�ֱ���һ�㣬Ҫ����QRS���ĸ߶�
	if (gQrsInfo.nAveQrsAmp)
	{
		// �������ƽ��QRS�����ȣ������� 1/4 ��Ϊ��ֵ
		noise_thr = gQrsInfo.nAveQrsAmp / 4;
	}

	// �Թ�С����ֵ�������ƣ�����QRS�����Ⱥ�С��ʱ������������
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
	//  2. Ϊ�˱��⽫�Ҳ������ٵ��ź�����Ϊ�������˴��ж�3��������QRS���ķ��ȱ仯���
	//		�������������С���ȵ�1.1�����ڣ�˵�������١��Ҳ����Σ���������
	//		ͬʱ�ڴ��ж�PVC���������PVC���򲻱�����
	//=========================================================================
	if (1 == noise_type)
	{
		final_pos = gQrsPeakBuff[gQrsPeakNum-1].nPeakPosAll;
		diff_max = gQrsPeakBuff[gQrsPeakNum-1].nQrsAmp;
		diff_min = gQrsPeakBuff[gQrsPeakNum-1].nQrsAmp;

		for (i = 0; i < gQrsPeakNum - 1; i++)
		{
			// ��ͳ�������ڵ�QRS������������С��QRS���ķ���
			if ((ECG_SAMPLE_RATE * 3) >= (final_pos - gQrsPeakBuff[i].nPeakPosAll))
			{
				// ��Χ�ڵ�QRS����������
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

				// �ж��Ƿ���PVC
				if (100 == gQrsPeakBuff[i].nQrsType)
				{
					is_PVC = 1;
				}
			}
		}

		// �������������С���ȵ�1.1�����ڣ�˵�������١��Ҳ����Σ���������
		// ����ķ���Ҳ��һ�����⣬�������У����ֻ��⵽һ�������������������ߵĲ�ֵǡǡ��С����ʱ����©������
		// ��������ʱ���ź������Ǻܴ��
		// �������PVC���򲻱�����
		if (((diff_max * 10 <= diff_min * 11) && (peak_num > 2)) || (1 == is_PVC))
		{
			noise_type = 0;
		}
	}

	//=========================================================================
	// 3. �����ͨ����ź��нϴ�Ĳ��������
	//		todo����ʱ������ַ�ļ�����������Ŀǰ���Ը�׼���ж�����
	//=========================================================================
	// ��ƽ����ַ���ڣ������������
	if (wAveDiffPeakNum)
	{
		//���
		 EcgNoisePeakDetect();

		//�ж�����1����ַ��������QRS������1.5��
		if (((wDiffNoisePeakNum * 2) <= (wAveDiffPeakNum * 3)) || (0 >= wDiffNoisePeakNum))
		{
			// ��������������QRS����������С��1.5������Ϊ�������������ⲻ��
			// ����������С�ڵ���0����Ϊû������
			peak_ratio = 0;
		}
		else
		{
			// todo��������������Ƶģ���ʱ������ַ���ж�׼�򣬵���Ŀǰ���Ը�׼���ж�����
			peak_ratio = 1;

			// ��������������QRS��������1.5������Ϊ�����ǳ���
			// �ж�����2��������ַ�������Сֵ���쳬��1.5��
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

			/* todo: ��ʱ�ɵ������Ըñ�׼����
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
		// ��ַ岻���ڣ�˵����û�м�⵽QRS���������������
		peak_ratio = 0;
	}

	//=========================================================================
	// 3. ����������Լ��Ƿ�������䵽��������������
	//=========================================================================
	//���
	if (gEcgNoiseBuffNum < ECG_NOISE_BUFF_LEN)
	{
		// ��δ����ֱ�����
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
		//����������λ��䣬�����µĲ������
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
	// 4. ����������QRS���ı�־
	//=========================================================================
	if (noise_type)
	{
		for (i = 0; i < gQrsPeakNum - 1; i++)
		{
			// ��ͳ�������ڵ�QRS������������С��QRS���ķ���
			if ((ECG_SAMPLE_RATE * 3) >= (final_pos - gQrsPeakBuff[i].nPeakPosAll))
			{
				gQrsPeakBuff[i].isNoisePeak = true;
			}
		}
	}

	//=========================================================================
	// 5. ���ã�����
	//=========================================================================
	gNoiseOrgSum = 0;
	wAveDiffPeak = 0;
	wAveDiffPeakNum = 0;
	wDiffNoisePeakNum = 0;

	return 1;
}

// ����3���ӵı�׼��
int EcgNoiseCalStd(void)
{
	int i = 0, temp = 0,  org_ave = 0;
	double square_sum = 0, std_result = 0;

	// �����ֵ
	org_ave = gNoiseOrgSum / gEcgNoiseTimeCnt;

	for (i = 0; i < gEcgNoiseTimeCnt; i++)
	{
		temp = gViewDataBuff[i] - org_ave;
		square_sum += temp * temp;
	}

	std_result = sqrt(square_sum / gEcgNoiseTimeCnt);

	return std_result;
}


// �������⺯��
int EcgNoisePeakDetect(void)
{
	int noise_thr = 0, i = 0, start_pos = 0, end_pos = 0, temp_pos = 0, noise_peak_cnt = 0;
	int pre_value = 0, cur_value = 0, max_value = 0, min_value = 0, temp_value = 0;
	bool go_up = false, is_max = false, if_min = false;

	//=========================================================================
	//1. ����������ַ�ķ�����ֵ��ƽ��QRS��ַ��1/3
	//=========================================================================
	noise_thr = wAveDiffPeak / wAveDiffPeakNum;
	noise_thr = noise_thr / 2;
	if (0 >= noise_thr)
	{
		// ��ֵС�ڵ���0��ֱ�ӷ���
		return 0;
	}

	//=========================================================================
	//2. ����3���ڵĲ�ּ�ֵ��
	//=========================================================================
	// ����Buff����ʼλ��
	start_pos = mod((gCurBuffPtr + 1), ECG_DATA_BUFF_LEN);
	end_pos = mod((gCurBuffPtr - 1), ECG_DATA_BUFF_LEN);
	i = start_pos;

	//��ʼ���Ѳ���ز���
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

	// ��ʼ������ֵ��
	while (i != end_pos)
	{
		//��ȡǰ���С�������ķ���ֵ
		cur_value = gDiffDataBuff[i];

		if (go_up)
		{
			// Ŀǰ���ڲ���״̬
			if (cur_value >= pre_value)
			{
				//��Ȼ���������Σ��������ֵ
				if (cur_value >= max_value)
				{
					max_value = cur_value;
				}
			}
			else
			{
				// �����½�״̬�ˣ��ж��Ƿ����0ֵ��
				if (0 >= (cur_value * pre_value))
				{
					// ������ֵ�ߣ�������ҵ�����ֵ����ת�벨��״̬
					is_max = true;
					go_up = false;
					min_value = cur_value;
				}
				else
				{
					// ���½������ǻ�û����0ֵ�ߣ�ʲô������������
				}
			}
		}
		else
		{
			// ���ڲ���״̬
			if (cur_value <= pre_value)
			{
				// �½��У��жϲ�������Сֵ
				if (cur_value <= min_value)
				{
					min_value = cur_value;
				}
			}
			else
			{
				// ���������У��ж��Ƿ񳬹���ֵ��
				if (0 >= (cur_value * pre_value))
				{
					// ͻ����ֵ�ߣ���ʱ��Ϊһ����ֵ���Ѳ����ڽ��������������������ж�
					if (is_max)
					{
						// ����ֵ���ڣ����㼫ֵ�Եķ���
						temp_value = max_value - min_value;

						if (temp_value >= noise_thr)
						{
							// ��ֵ�Եķ��ȴ�����ֵ���ȣ������������ֵ+1
							// ���������ĸ���С�ڻ��������ȣ���ֱ�����
							if (wDiffNoisePeakNum < ECG_NOISE_BUFF_LEN - 1)
							{
								wDiffNoisePeakBuff[wDiffNoisePeakNum] = temp_value;
								wDiffNoisePeakNum ++;
							}
							else
							{
								// ����������Ѿ���������󳤶ȣ���������ˣ��Ѿ��ܴ���
							}
						}

						// ��ռ���ֵ��־
						is_max = false;
					}

					// ���벨��״̬
					go_up = true;
					max_value = cur_value;
				}
				else
				{
					// δͻ����ֵ�ߣ�����
				}
			}
		}

		pre_value = cur_value;
		i = LoopInc(i, 1, ECG_DATA_BUFF_LEN);
	}

	return 1;
}


// ����HRVֵ
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
		// QRS���ĸ���С��5��������HRV
		return 0;
	}

	//=========================================================================
	// 1. �ж�����״̬�����ҹ�ȥ30s��10��������������е���������������������HRV��QRS����Χ
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

	// ����ʱ�����ĳ��ȣ�������start_noise���棬���ٲ�������30s+�޳�����������
	start_noise = ECG_HRV_COUNT + ECG_SAMPLE_RATE * noise_cunt * 3;

	//=========================================================================
	// 2. ���������źų��Ȳ������򲻼���HRV�������������RRI�Ĳ�ֵ��ƽ����
	//=========================================================================
	if (gEcgTotalSampleNum > start_noise)
	{
		if (0 == gEcgAlgResult.nOntimeHrv)
		{
			// ����ǵ�һ�μ���HRV����ӵ�6��QRS����ʼ��⣬�����ʼ�׶ε����Ӱ��
			rrd_ave = 6;
		}
		else
		{
			rrd_ave = 0;
		}

		for (i = rrd_ave; i < (gQrsPeakNum - 1); i++)
		{
			// 1. 30s�ڵ�QRS��;  2. ֻ���ÿ��ŵ�QRS��;  3. ���������ڼ��QRS��
			if ((gQrsPeakBuff[i].nPeakPosAll + start_noise) >= gQrsPeakBuff[gQrsPeakNum-1].nPeakPosAll)
			{
				if ((0 != gQrsPeakBuff[i].nRRInterval) && (!gQrsPeakBuff[i].isNoisePeak))
				{
					//=========================================================================
					// ��һ��������ʵʱ�����յ�HRV����
					//=========================================================================
					// 1. ����ǰ��QRS�����ڵĲ�ֵ��2. ��λת��Ϊms��3. ȷ�����ȣ��Ŵ�100����4. ����ƽ����
					temp_value = Abs(gQrsPeakBuff[i+1].nRRInterval - gQrsPeakBuff[i].nRRInterval);
					temp_value = temp_value * 100000 / ECG_SAMPLE_RATE;

					std_dev += temp_value * temp_value;
					hrv_cunt ++;
				}
			}
		}

		//=========================================================================
		// 3. �����ŵ�RR���ڸ�������Ҫ�������RRIƽ���͵ľ�����
		//=========================================================================
		if (hrv_cunt >= 5)
		{
			// ���Ƚ�ƽ�����Լ���������QRS���ĸ����洢����
			gTotalHrvSqr += std_dev;
			gTotalHrvNum += hrv_cunt;

			// ���������
			ontime_hrv = std_dev / hrv_cunt;
			ontime_hrv = sqrt(ontime_hrv);
		}
	}
	else
	{
		// ���������źŲ���30s��HRV��Ч
		ontime_hrv = 0;
	}

	
	//=========================================================================
	// 4. ������
	//=========================================================================
	return ontime_hrv;
}

