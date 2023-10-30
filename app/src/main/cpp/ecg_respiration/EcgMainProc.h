//*************************************************************************
//�ļ����ƣ�
//
//�ļ�˵����
//
//������ʷ��Created by Chenmaomao [8 / 15 / 2013]
//*************************************************************************

#ifndef _ECGMAINPROC_H_
#define _ECGMAINPROC_H_

#include "EcgDataType.h"
#include "EcgFilter.h"
#include "EcgQrsDetect.h"
#include "EcgResult.h"
#include "EcgQrsClassify.h"
#include "AF.h"

#include <string.h>

//=========================================================================
//1. ���峣��
//=========================================================================
#define ECG_ALG_VERSION				"01.19.27"									// �����㷨���汾

#define ECG_SAMPLE_RATE				250												// �����㷨���õĲ����ʣ�250Hz
#define ECG_DATA_BUFF_LEN			ECG_SAMPLE_RATE * 3			// �������ݻ������ĳ���
#define ECG_HRV_COUNT					ECG_SAMPLE_RATE * 25			// ÿ30s����һ��HRV����
#define ECG_NOISE_COUNT				ECG_SAMPLE_RATE * 3			// ÿ3s����һ��Noise���
#define ECG_HR_MAX_THRESH		251												// ���������ֵ��250BPM
#define ECG_HR_MIN_THRESH			29												// ������С��ֵ��30BPM

//=========================================================================
//2. ��������
//=========================================================================

extern short gViewDataBuff[ECG_DATA_BUFF_LEN];						// �ⲿ�˲����ݻ���������ΪECG�����㷨������
extern short gLpDataBuff[ECG_DATA_BUFF_LEN];							// ��ͨ�˲����ݻ�����������T����⣬ST������
extern short gHpDataBuff[ECG_DATA_BUFF_LEN];							// ��ͨ�˲����ݻ�����������T����⣬ST������
extern short gBpDataBuff[ECG_DATA_BUFF_LEN];							// ��ͨ�˲����ݻ�����
extern short gDiffDataBuff[ECG_DATA_BUFF_LEN];							// ��ͨ����źŻ�����
extern short gIntegDataBuff[ECG_DATA_BUFF_LEN];						// ƽ���������ݻ�����������QRS�����

extern unsigned int gTotalSampleBuff[ECG_DATA_BUFF_LEN];		// �ܲ����������֧��4min����
extern unsigned int gEcgTotalSampleNum;										// �ܵ�Ecg��������
extern unsigned short gCurBuffPtr;														// ��ǰ��������ָ��λ��
extern unsigned int gEcgHrvTimeCnt;												// 30s��ʱ������HRV���
extern unsigned short gEcgNoiseTimeCnt;										// 3���ʱ�������������
extern int gNoiseOrgSum;																		// ������������3sԭʼ����֮��

extern unsigned int	gEcgPauseTimeCnt;												// ������ͣ��ʱ
extern unsigned int	gEcgPauseNum;													// ������ͣ������holterģʽ��ÿ���Ӹ���
extern bool				gIsEcgPause;															// �Ƿ�������ͣ


extern ECG_ALG_RESULT	gEcgAlgResult;											// ����������
extern ECG_ALG_CONFIG gEcgAlgConfig;											// �㷨������Ϣ
extern ECG_DEBUG_INFO gEcgDebugInfo;											// ������Ϣ�������
extern ECG_FINAL_RESULT gEcgFinalResult;										// �㷨���շ���������ź�����������������һ��
extern ECG_FINAL_RESULT gEcgFinalFirst;											// �㷨���η����Ľ�����������������ȶ�
extern bool gbFirstExist;																		// ���η�������Ƿ����

extern ECG_HOLTER_RESULT gEcgHolterResult;								// Holter������

extern QRS_INFO gQrsInfo;																	// QRSƽ����ȡ�б�ʡ�RR���ڵ���Ϣ


//=========================================================================
//3. ��������
//=========================================================================

// �㷨������Ϣ���ú���������ӿڣ�
int EcgAlgSetup(ECG_ALG_CONFIG alg_config, char *ecg_alg_version);

// �㷨��ʼ������������ӿڣ�
void EcgAlgInitialize(void);

// �㷨��������ں���������ӿڣ�
void EcgAlgAnalysis(int data);

// ��ȡ�㷨�ڲ��ĵ�����Ϣ������ӿڣ�
int EcgAlgGetDebugInfo(ECG_DEBUG_INFO *debug_info, bool reset);

// ��ȡ���յ�ȫ��QRS��������ӿڣ�
int EcgAlgGetDebugQrs(QRS_PEAK *qrs_peak_buff, ECG_NOISETYPE *ecg_noise_buff, int *noise_buff_num, bool reset);

// ��ȡ�㷨�ķ������������ӿڣ�
int EcgAlgGetResult(ECG_ALG_RESULT *alg_result, bool reset);

// Ecg���ݻ�������ʼ������
void EcgDataInitialize(void);

// Ecg�����˲�����ں���
int EcgDataProcess(int data, bool reset);

// �ڶ�������ǰ���㷨����һЩ���ã������������Ļ���
int EcgAlgRecalConfig(void);

//�����ڵ�10s��ʱ�����Ƿ��ж�Ϊ��������2����Ҫ�����ı�־
bool EcgGetNeedResetFlag(void);
//=========================================================================
//4. ͨ�ú�������
//=========================================================================
// ��ģ����
int mod(int i, int j);

// ѭ���������ڵ�ָ���λ����
int LoopInc(int data, int inc, int length);

// ѭ���������ڵ�ָ����λ����
int LoopDec(int data, int inc, int length);

// ���������򣬲�����ָ��λ�õ�ֵ
int SortAndSearch(int org[], int len, int mid);

// �������Ʋ���������ָ����bitλ
int RightShift(int data, char bit);

// ȡ�����ݵĵ�12λ
int GetLowBit12(int data);

// ȡ����ֵ
int Abs(int data);


#endif

