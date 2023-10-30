//*************************************************************************
//�ļ����ƣ�EcgMainProc.cpp
//
//�ļ�˵����Ecg�㷨������
//
//������ʷ��Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************

#ifndef _ECGQRSDETECTION_H_
#define _ECGQRSDETECTION_H_

#include "EcgMainProc.h"

//=========================================================================
//����ṹ�塢����
//=========================================================================

#define INTEG_PEAK_MAX_WIDTH		500 * ECG_SAMPLE_RATE / 1000		// ���ַ���������ֵ��500ms
#define INTEG_PEAK_MIN_WIDTH		20   * ECG_SAMPLE_RATE / 1000		// ���ַ���������ֵ��20ms
#define INTEG_PEAK_BUFF_LEN			5																// ���ַ建��������
#define INTEG_PEAK_MIN_THRESH		50															// ���ַ���С�����ֵ

#define IRREG_THR_NRM						15															// ��һ�� r-MSSD �Ĳ�������ɲ�������ͨ�汾�������Ƚϵ�
#define IRREG_THR_JAP							10															// ��һ�� r-MSSD �Ĳ�������ɲ������ձ��汾�������Ƚϸ�

#define SEARCH_WINDOW					180 * ECG_SAMPLE_RATE / 1000		// ����QRS�����Ϊ120ms����˶����Ѳ����Ϊ180ms����������Ѳ������
#define QRS_PEAK_BUFF_LEN				120															// Qrs���Ļ���������ΪҪ֧��90s�Ĳ��������Կ���120��������

#define ECG_NOISE_BUFF_LEN				30															// ����������������������3����һ�Σ�30��һ��ֻ��10�����棬һ����20�����棬��һ������趨Ϊ30
#define ECG_NOISE_STD_THR				200															// ����ԭʼ�źŵı�׼��

//=========================================================================
//  ��������ص�һЩ������LSB��ʵ�ʷ��ȵĻ����ϵ�� 1.25uV = 1 LSB
//=========================================================================
//#define uV_PER_LSB			1.25
#define uV_PER_LSB				2.467
#define QRS_PEAK_MIN_THRESH		0.2 * 1000 / uV_PER_LSB							// ����QRS���ȵ���С��ֵ����С��ֵΪ0.2mV / 1.25uV per LSB = 200LSB
																															// �˴�ԭ����0.25mV = 200LSB��������Ϊ�˲�֮���������˥��
																															// ����0.25mV�������޷����, ��˸���Ϊ0.2mV = 160LSB
#define QRS_PEAK_MAX_THRESH		10 * 1000 / uV_PER_LSB							// ����QRS���ȵ������ֵ�������ֵΪ10 mV / 1.25uV per LSB = 8000LSB
#define QRS_LOW_AMP_THRESH		0.375 * 1000 / uV_PER_LSB						// ��ƽ��QRS����С�ڸ���ֵ���򱨷��ȹ��ͣ�300LSB = 0.375mV
#define QRS_LOW_AMP_NO_PVC		0.4 * 1000 / uV_PER_LSB							// ��ƽ��QRS����С�ڸ���ֵ����PVC���㣬320LSB = 0.25mV
#define QRS_LOW_AMP_NO_IRREG	0.4 * 1000 / uV_PER_LSB							// ��ƽ��QRS����С�ڸ���ֵ����Irreg�������㣬320LSB = 0.4mV

#define ST_HORIZON_THRESH		0.01 * 1000 / uV_PER_LSB						// ST������ʱ���ж�ˮƽ�ߵ���ֵ��8LSB = 0.01mV
#define T_PEAK_THRESH			0.05 * 1000 / uV_PER_LSB						// T��������ֵ��40LSB = 0.05mV

#define SHORT_QTC_THRESH		350																// ����QTc����
#define LONG_QTC_THRESH			470																// �ٽ�QTc����


//=========================================================================
//�������
//=========================================================================
extern QRS_PEAK gQrsPeakBuff[QRS_PEAK_BUFF_LEN];						//QRS��������������ʵʱ���㣬ֻ��Ҫ���ٵ�QRS������
extern unsigned short gQrsPeakNum;													//QRS�������ڲ��ĸ���

extern ECG_NOISETYPE gEcgNoiseBuff[ECG_NOISE_BUFF_LEN];		//ECG�ź�������������ÿ3����һ�Σ�30��Լ��10�����档0-�ǳ��ã� 1-��������2-�ǳ���
extern unsigned short gEcgNoiseBuffNum;											//ECG�ź���������������

extern float gTotalHrvSqr;																			// �����������HRV��ƽ���ͣ��������յ�HRV����
extern int gTotalHrvNum;																			// �����������HRV��QRS�������������յ�HRV����

//=========================================================================
//��������
//=========================================================================

// QRS���ϲ��Ѳ�����ں���
int QrsComplexDetect(bool reset);

// ���ַ��Ѳ���ֵ��ʼ������
int IntegThreshInitalize(bool reset);

// ���»��ַ��Ѳ���ֵ
void IntegThreshUpdate(void);

// ���ַ��Ѳ�����
int IntegPeakDetection(int cur_data, int cur_pos, INTEG_PEAK *result, bool reset);

// �ж��Ƿ���Ч��QRS��
int QrsComplexJudge(void);

// �ڴ�ͨ����ź��Ͻ���QRS��λ�ó�����λ
int BpDiffDataPeakDetect(BP_DIFF_PEAK *diff_peak);

// ����ʾ�ź��Ͻ���QRS��λ�ü��
int BandPeakDetect(BP_DIFF_PEAK *diff_peak, QRS_PEAK_POS *band_peak);

// ����ʾ�ź��Ͻ���QRS��λ�ü��
int ViewPeakDetect(QRS_PEAK_POS *band_peak, QRS_PEAK_POS *view_peak);

// ������̬����Ϣ���ų�����QRS��
int QrsComplexExclusion(QRS_PEAK_POS *view_peak, QRS_PEAK_POS *band_peak);

// ά��QRS�����У��������QRS������
void QrsComplexUpdate(void);

// ������⺯��
int EcgNoiseEstimate(bool reset);

// ����3���ӵı�׼��
int EcgNoiseCalStd(void);

// �������⺯��
int EcgNoisePeakDetect(void);

// ����ȫ��HRVֵ��30sһ��
float CalculateOntimeHrv(bool reset);




#endif
