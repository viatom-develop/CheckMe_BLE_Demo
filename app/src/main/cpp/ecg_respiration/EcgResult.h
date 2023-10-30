//*************************************************************************
//�ļ����ƣ�EcgResult.cpp
//
//�ļ�˵����Ecg�������㺯��
//
//������ʷ��Created by Chenmaomao [5 / 15 / 2014]
//*************************************************************************



#ifndef _ECGRESULT_H_
#define _ECGRESULT_H_

#include "EcgMainProc.h"
//=========================================================================
//1. ����ֲ��ṹ��
//=========================================================================
typedef struct Local_Peak
{
	int nStartPos;								//peak���λ��
	int nStartValue;								//peak����ֵ
	int nPeakPos;								//peak����λ��
	int nPeakValue;								//peak�����ֵ
	int nPeakHight;								//peak����
}LOCAL_PEAK;


//=========================================================================
//1. ���庯��
//=========================================================================
// �����ʼ��
void EcgResultInitialize(void);

// ��������
int CalculateHeartRate(bool reset);

// ������ǰQRS����IOS�㣬������һ��QRS����STֵ
int CalculateStValue(bool reset);

// ������һ��QRS����QTֵ
int CalculateQTValue(bool reset);

// ��ȡ�㷨���շ������������ӿڣ�
int EcgAlgGetFinalResult(ECG_FINAL_RESULT *final_result, bool reset);

// ����HRV��ز���
int CalculateFinalHrv(void);

// �жϲ��������
int JudgeIrregRhythm(int rri_num, int *rri_value, int *rri_sort);

// ��ȡȫ����QRS����λ�ã�����ӿڣ��ṩ��PWTT��
int EcgAlgGetQrsPeakPos(int *qrs_pos_buff);


//=========================================================================
//1. Holter��ؽӿں���
//=========================================================================

// ��Flash�������е�RR���ڣ��������յ�HRV
int EcgAlgCalculateHolterHRV(unsigned short curRRI);

// holterÿ���ӻ�ȡ���
int EcgAlgGetHolterOneMinResult(ECG_FINAL_RESULT *final_result, bool reset);

// holter24Сʱ���ս��
int EcgAlgGetHolterFinalResult(ECG_HOLTER_RESULT *final_result, bool reset);


#endif
