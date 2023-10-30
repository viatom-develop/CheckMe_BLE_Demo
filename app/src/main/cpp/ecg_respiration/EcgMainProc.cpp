//*************************************************************************
//�ļ����ƣ�EcgMainProc.cpp
//
//�ļ�˵����Ecg�㷨������
//
//������ʷ��Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************
//#include "stdafx.h"
#include "EcgMainProc.h"
#include "RespMain.h"
//=========================================================================
//1. �������ȫ�ֱ���
//=========================================================================
ECG_DEBUG_INFO		gEcgDebugInfo;								// ������Ϣ�������
ECG_ALG_CONFIG		gEcgAlgConfig;								// �㷨������Ϣ
ECG_ALG_RESULT		gEcgAlgResult;									// �㷨ʵʱ���������ÿ�����
ECG_FINAL_RESULT		gEcgFinalResult;								// �㷨���շ���������ź�����������������һ��
ECG_FINAL_RESULT		gEcgFinalFirst;									// �㷨���η����Ľ�����������������ȶ�
bool gbFirstExist;																		// ���η�������Ƿ����

ECG_HOLTER_RESULT gEcgHolterResult;							// Holter������

QRS_INFO gQrsInfo;																// QRSƽ����ȡ�б�ʡ�RR���ڵ���Ϣ

short gViewDataBuff[ECG_DATA_BUFF_LEN];							// �ⲿ�˲����ݻ���������ΪECG�����㷨������
short gLpDataBuff[ECG_DATA_BUFF_LEN];								// ��ͨ�˲����ݻ�����������T����⣬ST������
short gHpDataBuff[ECG_DATA_BUFF_LEN];							// ��ͨ�˲����ݻ�����������T����⣬ST������
short gBpDataBuff[ECG_DATA_BUFF_LEN];								// ��ͨ�˲����ݻ�����
short gDiffDataBuff[ECG_DATA_BUFF_LEN];							// ��ͨ����źŻ�����
short gIntegDataBuff[ECG_DATA_BUFF_LEN];						// ƽ���������ݻ�����������QRS�����

unsigned int gTotalSampleBuff[ECG_DATA_BUFF_LEN];			// �ܲ����������֧��2386Сʱ����
unsigned int gEcgTotalSampleNum;						// �ܵ�Ecg��������
unsigned short gCurBuffPtr;										// ��ǰ��������ָ��λ��
unsigned int gEcgHrvTimeCnt;									// 30s��ʱ������HRV���

unsigned short gEcgNoiseTimeCnt;						// 3���ʱ�������������
int gNoiseOrgSum;														// ������������3sԭʼ����֮��

unsigned int	gEcgPauseTimeCnt;							// ������ͣ��ʱ
unsigned int	gEcgPauseNum;								// ������ͣ������holterģʽ��ÿ���Ӹ���
bool gIsEcgPause;													// �Ƿ�������ͣ

RESP_ALG_RESULT Resp_Result_temp;  // ��������ز���

// �㷨��ʼ������
void EcgAlgInitialize(void)
{
	// �˲�����ʼ��
	InitializeEcgFilter();

	// ���ݴ����ʼ��
	EcgDataProcess(0, true);

	// ���ݻ�������ʼ��
	EcgDataInitialize();

	// QRS�Ѳ���ʼ��
	QrsComplexDetect(true);

	// �����ʼ��
	EcgResultInitialize();

	// QRS�����ʼ��
	QrsClassifyMain(true);

	// ʵʱ���������ʼ��
	EcgAlgGetResult(0, true);

	// ���ʼ����ʼ��
	CalculateHeartRate(true);

	// ST�����ʼ��
	CalculateStValue(true);

	// ���շ��������ʼ��
	gbFirstExist = false;
	EcgAlgGetFinalResult(0, true);

	// ���Ҫ������ʼ��һ�Σ����������EcgAlgGetResult���������ʼ��
	gEcgAlgResult.isAsystole = false;

	// ��������ʼ��
	EcgNoiseEstimate(true);

	// ������Ϣ��ʼ��
	EcgAlgGetDebugInfo(0, true);

	gEcgAlgConfig.sOrgSampleRate = 250;
	gEcgAlgConfig.cWorkMode = 1;					// ��ʼ��Ϊ�ڲ��缫
	gEcgAlgConfig.nCeilingValue = 0;

	gQrsInfo.nAveInterval = 0;
	gQrsInfo.nAveSlope = 0;
	gQrsInfo.nAveWidth = 0;
	gQrsInfo.nAveQrsAmp = 0;

	gEcgPauseTimeCnt = 0;
	gIsEcgPause = false;
	gEcgPauseNum = 0;

	// ����������ʼ��
	RespMain(&Resp_Result_temp,0,1);
}

void EcgDataInitialize(void)
{
	int i = 0;

	// ��ʼ��buffer
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

	// ��ʼ��ָ��
	gCurBuffPtr = 0;
	gEcgTotalSampleNum = 0;
}

//Ecg�㷨����
int EcgAlgSetup(ECG_ALG_CONFIG alg_config, char *ecg_alg_version)
{
	gEcgAlgConfig = alg_config;
	gEcgAlgConfig.nCeilingValue = (2 << 13) / 2;		// ����ǯλֵ��14bit������13bit

	// �ж��㷨�����Ƿ���ȷ
	if ((500 != gEcgAlgConfig.sOrgSampleRate) && (250 != gEcgAlgConfig.sOrgSampleRate))
	{
		// �����������������÷�Χ�򱨴�
		return 0;
	}
	else
	{
		memcpy(ecg_alg_version, ECG_ALG_VERSION, 8);

		return 1;
	}
}

// �ڶ�������ǰ���㷨����һЩ���ã������������Ļ���
int EcgAlgRecalConfig(void)
{
	//=========================================================================
	// �����Ժ󣬰����еĶ���ȫ�����㣬׼�����л���
	//=========================================================================
	// �˲�����ʼ��
	InitializeEcgFilter();

	// ���ݴ����ʼ��
	EcgDataProcess(0, true);

	// ���ݻ�������ʼ��
	EcgDataInitialize();

	// ��QRS���ĸ�������
	gQrsPeakNum = 0;

	gQrsInfo.nAveInterval = 0;
	gQrsInfo.nAveSlope = 0;
	gQrsInfo.nAveWidth = 0;
	gQrsInfo.nAveQrsAmp = 0;

	// ��PVC��������
	gPvcTemplate.nTemplateNum = 0;
	gPvcTemplate.bIsTemplateExist = false;
	//PVC��������
	gPvcNumAll = 0;

	// HRV��ʱ����
	gEcgHrvTimeCnt = 0;
	gTotalHrvSqr = 0;
	gTotalHrvNum = 0;

	// �����������λ���������
	EcgAlgGetFinalResult(0, true);

	// ��������ⲿ�ָ�λ
	EcgNoiseEstimate(true);

	return 1;
}

//Ecg�㷨������ں���
void EcgAlgAnalysis(int data)
{
	int temp_hrv = 0;

	// ��ʼ���㷨���������ز���
	// EcgAlgGetResult(0, true);
	// �����޸���Ϊ�˱���ͣ����ʶ
	gEcgAlgResult.isQrsDetected = false;
	gEcgAlgResult.isHrvChange = false;

	// ��������ʱ��HRV��ʱ
	gEcgHrvTimeCnt++;
	gEcgNoiseTimeCnt++;

	// ��ʼ��������Ϣ�������
	EcgAlgGetDebugInfo(0, true);

	//1. �����ĵ������˲�������������
	if (!EcgDataProcess(data, false))
	{
		// �����˲�δ����㷨���ݻ���������ִ�к������㣬ֱ���˳�
		return;
	}

	// ����QRS�����
	if (QrsComplexDetect(false))
	{
		// ���㵱ǰ������	    
		RespMain(&Resp_Result_temp, 0, 0);
		gEcgAlgResult.RespRate = Resp_Result_temp.RespRate;
		//printf("Resp_Result_temp.RespRate = %d  \n", Resp_Result_temp.RespRate); // debug   

		// ���㵱ǰQRS��������
		gEcgAlgResult.nEcgHeartRate = CalculateHeartRate(false);

		// ������һ��QRS����STֵ����Ϊ��ǰQRS���ոռ����T��������δ�����㷨��
		CalculateStValue(false);

		// ������һ��QRS����QTֵ
		CalculateQTValue(false);

		// ����һ��QRS�����з���
		QrsClassifyMain(false);

		// ����HRV
		if (ECG_HRV_COUNT <= gEcgHrvTimeCnt)
		{
			temp_hrv = CalculateOntimeHrv(false);
			if (temp_hrv)
			{
				// �������ֵ����0�������ȫ��HRV
				gEcgAlgResult.nOntimeHrv = temp_hrv;
				gEcgAlgResult.isHrvChange = true;

				// �����ʱ���������һ�κ�ֻ��Ҫ����10�룬��Ӧ������һ��
				gEcgHrvTimeCnt = ECG_SAMPLE_RATE * 20;
			}
		}
	}

	// �����������
	if (ECG_NOISE_COUNT == gEcgNoiseTimeCnt)
	{
		// ÿ3�����һ���������
		EcgNoiseEstimate(false);

		// �������ʱ�临λ
		gEcgNoiseTimeCnt = 0;
	}

	// ����ͣ�����
	if (gBpDataBuff[gCurBuffPtr] < QRS_PEAK_MIN_THRESH)
	{
		gEcgPauseTimeCnt++;

		// �������룬�ж�Ϊ������ͣ
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
		// ���ȴ�����ֵ������ͣ��ؼ�������
		gEcgPauseTimeCnt = 0;
		gIsEcgPause = false;
	}

}

//Ecg�����˲��ܺ���
int EcgDataProcess(int data, bool reset)
{
	int flag = 0;
	static char sample_count = 0;			//������������ʵ��1/2��1/4ԭʼƵ�ʵĲ���
	static short buff_ptr = 0;						//�������ڵ����ݸ�������Ҫ��Ϊ�˷���gEcgBuffPtr����������

	if (reset)
	{
		sample_count = 0;
		buff_ptr = 0;
		return 0;
	}

	gCurBuffPtr = buff_ptr;

	//=========================================================================
	//3. ���н���������ԭʼ���ݽ�����Ϊ250Hz����
	//=========================================================================
	if (0 == sample_count)
	{
		// �������㣬���й�һ��
		// ά������������
		sample_count ++;
		if (sample_count >= (gEcgAlgConfig.sOrgSampleRate / ECG_SAMPLE_RATE))
		{
			sample_count = 0;
		}

		// ���������͹�һ��������ݴ�����ʾ���ݻ�����
		gViewDataBuff[gCurBuffPtr] = data;
		gNoiseOrgSum += data;
	}
	else
	{
		// �ǽ������㣬����������㷨��ֱ���˳�
		// ά������������
		sample_count ++;
		if (sample_count >= (gEcgAlgConfig.sOrgSampleRate / ECG_SAMPLE_RATE))
		{
			sample_count = 0;
		}

		// ֱ���˳��������к�������
		return 0;
	}

	//=========================================================================
	//3. �㷨�ڲ��Ĵ�ͨ����֡�ƽ�����ֵ��˲�����
	//=========================================================================
	//data_temp = Fir5HighPass(gViewDataBuff[gCurBuffPtr], false);
	//3.1. �����ź� ��ͨ / ��ͨ �˲�
	gLpDataBuff[gCurBuffPtr] = Fir20LowPass(gViewDataBuff[gCurBuffPtr], false);
	gHpDataBuff[gCurBuffPtr] = IndFir5HighPass(gViewDataBuff[gCurBuffPtr], false);

	//3.2. �����źŴ�ͨ�˲�
	gBpDataBuff[gCurBuffPtr] = Fir5HighPass(gLpDataBuff[gCurBuffPtr], false);

	//3.3. �����źŲ���˲�
	gDiffDataBuff[gCurBuffPtr] = EcgDiffFilter(gBpDataBuff[gCurBuffPtr], false);

	//3.4. �����ź������ֵ�����������˲�
	gIntegDataBuff[gCurBuffPtr] = SquareIntegration(gDiffDataBuff[gCurBuffPtr], false);

	//3.5. �ܲ����������������������������
	gTotalSampleBuff[gCurBuffPtr] =  gEcgTotalSampleNum;

	//3.6. ά��ָ����У���������������
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


//	��ȡ�㷨�������
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


//��ȡ�㷨������Ϣ
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

	// ���������Ϣ���ڣ��򷵻ص�����Ϣ
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


// ��ȡ���յ�ȫ��QRS��
int EcgAlgGetDebugQrs(QRS_PEAK *qrs_peak_buff, ECG_NOISETYPE *ecg_noise_buff, int *noise_buff_num, bool reset)
{
	int i = 0;

	if (reset)
	{
	}

	// ������⻺������ֵ
	for (i = 0; i < gEcgNoiseBuffNum; i++)
	{
		ecg_noise_buff[i] = gEcgNoiseBuff[i];
	}
	*noise_buff_num = gEcgNoiseBuffNum;

	// QRS��������ֵ
	for (i = 0; i < gQrsPeakNum; i++)
	{
		qrs_peak_buff[i] = gQrsPeakBuff[i];
	}

	return gQrsPeakNum;
}


//�����ڵ�10s��ʱ�����Ƿ��ж�Ϊ��������2����Ҫ�����ı�־
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
// ������ģ����λ�������ͨ�ú���
//=========================================================================
//��ģ����
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
// ��������: inline int LoopInc(int data, int length)
//
// ����˵��: ѭ���������ڵ�ָ���λ����
//
// �������: int data:	��λǰ��ָ��λ��
//			int length:	ѭ���������ĳ���
//
// �������: int result: ��λ���ָ��λ��
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
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

// ѭ���������ڵ�ָ����λ����
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

// ���������򣬲�����ָ��λ�õ�ֵ
int SortAndSearch(int org[], int len, int mid)
{
	int left = 0, right = 0, mid_value = 0, temp = 0, i = 0, j = 0;

	int value[50];		//�ݶ�����Ϊ10�����������Ҫ���ԸĴ�һЩ

	//Ϊ�˲��ı�ԭ�����е�����˳�򣬽�����ֵ��ֵ���ڲ��ľֲ�����
	for (i = 0; i < len; i++)
	{
		value[i] = org[i];
	}

	// ð������
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

	// ���ش���λ�õ�ֵ
	return value[mid - 1];
}


//************************************************************************
// ��������: inline int RightShift(int data, char bit)
//
// ����˵��: �����������������Ʋ������������ֱ�������bitλ
//
// �������: int data:	��������
//			char bit:	���Ƶ�λ��	
//
// �������: int result: ���ƺ���������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
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
// ��������: inline int GetLowBit12(int data)
//
// ����˵��: ������������ȡ���ݵĵ�12λ
//
// �������: int data:   ��������
//
// �������: int result: ��12λ�������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
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

// ȡ����ֵ
int Abs(int data)
{
	return data >= 0 ? data : (-data);
}