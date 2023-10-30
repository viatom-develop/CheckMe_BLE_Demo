 //#include <stdbool.h>
#include <string>

//=========================================================================
// ����QRS�Ѳ���ؽṹ��
//=========================================================================
//���ַ�ṹ��
typedef struct Integ_Peak
{
	unsigned int nStartHight;				//���ַ�������
	unsigned int nStartPos;				//���ַ����λ��
	unsigned int nPeakHight;				//���ַ岨�����
	unsigned int nPeakPos;				//���ַ岨��λ��
}INTEG_PEAK;

// ��ͨ��ַ�ṹ��
typedef struct Bp_Diff_Peak
{
	unsigned int nDiffStartPos;			//��ַ���ʼλ�ã������϶�Ӧ�ڴ�ͨ�źŵ����λ��
	unsigned int nDiffEndPos;			//��ַ��յ�λ�ã������϶�Ӧ�ڴ�ͨ�źŵ��յ�λ��
	unsigned int nDiffZeroPos;			//��ַ�����λ�ã������϶�Ӧ�ڴ�ͨ�źŵķ�ֵ��λ��
	unsigned int nDiffMaxPos;			//��ַ����ֵλ��
	unsigned int nDiffMinPos;			//��ַ���Сֵλ��
}BP_DIFF_PEAK;

// ��ʾ�źŷ�ṹ��
typedef struct Qrs_Peak_Pos
{
	unsigned int nStartPos;				//QRS���λ��
	unsigned int nEndPos;					//QRS�յ�λ��
	unsigned int nPeakPos;				//QRS����λ��
}QRS_PEAK_POS;

typedef struct Qrs_Peak
{
	// λ����Ϣ
	unsigned short nStartPos;			// ���ݻ������е����λ��
	unsigned short nPeakPos;			// ���ݻ������еķ�ֵ��λ��
	unsigned short nEndPos;			// ���ݻ������е��յ�λ��
	unsigned short nIsoPos;				// ���ݻ������е�ISO��λ��
	unsigned short nStPos;				// ���ݻ������е�ST��λ��
	unsigned short nTpeakPos;		// T��λ��
	unsigned short nTendPos;			// T��������λ�ã�����QT����
	unsigned short nPpeakPos;		// P��λ��
	unsigned short nBpPeakPos;		// ��ͨ�źŵĲ���λ�ã���Ҫ��Ϊ�˺���ģ��ƥ���׼��

	unsigned int nPeakPosAll;			// �ܲ������еķ�ֵ��

	bool isPeakBefore;						// ��ַ����ͣ��жϸ÷�����ͣ�0�������ں󡢲�����ǰ��1��������ǰ�������ں�
	bool isPositive;								// �ò����ͣ�0�����򲨣�1������
	bool isSolid;									// ��QRS���Ƿ���ţ�����ƽ��RR�����жϣ�
	bool isNoisePeak;						// �Ƿ�Χ�ڵ�QRS��

	// QRS���������
	int nQrsAmp;								// QRS������
	int nQrsHpAmp;
	int nStValue;									// Stֵ
	int nQtValue;									// QTֵ
	unsigned short nQrsWidth;		// QRS�����
	unsigned short nRRInterval;		// RR����

	// QRS��������Ϣ
	unsigned short nQrsType;			// ��QRS�����ͣ�0=��ʼ��1=������2=PVC��3=δ����
	short nCoefNormal;						// ��QRS��������ģ������ϵ��

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
	unsigned int nNoiseStartPosAll;				// ���μ�����ʼλ��
	unsigned short nNoiseType;						// �������ݵ��������ͣ�0-��������1-������
	unsigned int nNoiseStd;								// �����ж������ı�׼��
	unsigned short nPeakRatio;						// ��ַ��Ƿ񳬹�QRS�������1.5��
	unsigned int	nNoiseThr;								// �ж���������ֵ
}ECG_NOISETYPE;



//=========================================================================
//	����ȫ����ؽṹ��
//=========================================================================
// �㷨���ýṹ��
typedef struct Ecg_Alg_Config
{
	unsigned short	sOrgSampleRate;					// Ecg����ԭʼ����Ƶ�ʣ��㷨�ڲ�������250Hz��������Ϊ250 / 500 / 1000
	unsigned char	cWorkMode;						// �豸�Ĺ���ģʽ��1���ڲ��缫��2���ⲿ����
	int					nCeilingValue;						// ǯλֵ����ֹԽ�磬����AD�Ĳ���λ������ó�
	bool					bJapanVersion;					// �ձ��汾��trueΪ�ձ��汾��PVC��Irreg�����Ƚϸߣ�falseΪ��ͨ�汾
}ECG_ALG_CONFIG;

// �㷨����������ṹ��
typedef struct Ecg_Alg_Result
{
	bool isQrsDetected;				// �Ƿ���QRS��������м����������ʡ���ȡ����ȵ���Ϣ
	bool isHrvChange;					// ʵʱrMSSD�Ƿ�ı䣬�����Ҫ�������������
	bool isAsystole;						// �ж��Ƿ�ͣ��������5�����������ж�Ϊͣ��
	bool bIsSolid;							// ��QRS���Ƿ����
	int nEcgHeartRate;					// ���ʣ�ÿ��⵽һ��QRS������һ��
	int nQrsWidth;							// QRS�����
	int nQrsAmp;							// QRS������
	int nQrsHpAmp;						// ��ͨ�źŵ�QRS������
	int nQrsInterval;						// QRS��RR����
	int nQrsPeakPos;						// QRS���Ĳ���λ��
	int nOntimeHrv;						// 30sʵʱHRVֵ����λΪms���Ŵ�100��

	QRS_PEAK EcgOntimeQrs;		// ʵʱ��������QRS���������Ϣ

	short RespRate;					//������
}ECG_ALG_RESULT;


// HRV����ʱ����Ҫ��ô��HRV���ݣ�
// typedef struct Hrv_Para
// {
// 	int nHrvMean;					// RRƽ��ֵ
// 	int nHrvSdnn;
// 	int nHrvRmssd;					// ��ֵ����ƽ����������RR���㣬��ӳ���ɷ�
// 	int nHrvPnn50;
// }HRV_PARA;

// �㷨���յķ������
typedef struct Ecg_Final_Result
{
	unsigned short nEcgHeartRate;			// ���ʣ���λΪBPM
	unsigned short nEcgQrsWidth;				// QRS��ƽ����ȣ���λΪms
	unsigned short nEcgQrsAmp;				// QRS��ƽ�����ȣ���λΪLSB
	unsigned short nPvcNum;						// PVC�ĸ���
	unsigned short nIrregCoef;					// ���������ϵ��
	unsigned char SignalNoise;					// �ź��Ƿ�������� 0-��������1-������;2-������

	unsigned short nEcgQtValue;				// QTֵ����λΪms
	unsigned short nEcgQTc;						// QTc������HRУ����QT
	unsigned short nPausePerMin;
	unsigned char nQTcStatus;					// QTc״̬��0��Short QTc��1��Normal QTc��2��Prolonged QTc

	float nEcgStValue;				// STֵ����λΪuV
	int nEcgFinalHrv;					// ���յ�HRV���

	bool Bradycardia;					// �Ķ�������С��60
	bool Tachycardia;				// �Ķ����٣�����100
	bool Irregular;						// ���������
	bool WideQrs;						// Qrs������
	bool HighStValue;				// Stƫ��
	bool LowStValue;					// STƫ��
	bool LowQrsAmp;				// QRS�����ȹ���
	bool PvcBeat;						// ���������粫
	bool PauseBeat;
	
	//HRV_PARA HrvResult;			// ����Ҫ��ô��HRV�����һ���͹���

}ECG_FINAL_RESULT;


typedef struct Ecg_Holter_Result
{
	unsigned short nAveHeartRate;
	unsigned int nSumHeartRate;
	unsigned short nAveRRinterval;				// ��λΪ������
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


// �㷨�ڲ�������Ϣ�ṹ��
typedef struct  Ecg_Debug_Info
{
	bool bIsDebugInfo;						//������Ϣ�Ƿ���ڣ���Ϊ�ǽ������㣬�򲻷��ص�����Ϣ
	bool bIntegPeak;							//�Ƿ���ڻ��ַ塣�����ַ����������ĵ�����Ϣ����

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
	BP_DIFF_PEAK BpDiffPeak;				//��ͨ����ź��ϵ��Ѳ������������ڻ�ͼ
	QRS_PEAK_POS BandPeak;				//��ͨ�ź��ϵ��Ѳ����
	QRS_PEAK_POS HighPeak;				//��ͨ�ź��ϵ��Ѳ����
	QRS_PEAK_POS ViewPeak;				//��ʾ�ź��ϵ��Ѳ����

}ECG_DEBUG_INFO;
