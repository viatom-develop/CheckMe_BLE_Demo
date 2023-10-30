//*************************************************************************
//�ļ����ƣ�EcgInterface.h
//
//�ļ�˵���������㷨��ڣ������������Զ����������
//
//������ʷ��Created by Chenmaomao [8 / 20 / 2013]
//*************************************************************************

#ifndef _ECGINTERFACE_H_
#define _ECGINTERFACE_H_

#include "EcgMainProc.h"
#include "EcgResult.h"


//=========================================================================
// 2������ӿں���
//=========================================================================

// �㷨���ú���
extern int EcgAlgSetup(ECG_ALG_CONFIG alg_config, char *ecg_alg_version);

// �㷨�������ʼ������
extern void EcgAlgInitialize(void);

// �㷨������ں���
extern void EcgAlgAnalysis(int data);


//=========================================================================
//3. ����ӿں���
//=========================================================================
//	����㷨�������
extern int EcgAlgGetResult(ECG_ALG_RESULT alg_result, bool reset);

// ��ȡ�㷨���շ������
extern int EcgAlgGetFinalResult(ECG_FINAL_RESULT *final_result, bool reset);

// ��ȡȫ����QRS����λ�ã�����ӿڣ��ṩ��PWTT��
extern int EcgAlgGetQrsPeakPos(int *qrs_pos_buff);


//=========================================================================
//3. ������Ϣ�ӿں���
//=========================================================================
// ����㷨�ڲ���ʵʱ������Ϣ
extern int EcgAlgGetDebugInfo(ECG_DEBUG_INFO *debug_info, bool reset);

// ��ȡ���յ�ȫ��QRS��
extern int EcgAlgGetDebugQrs(QRS_PEAK *qrs_peak_buff, ECG_NOISETYPE *ecg_noise_buff, int *noise_buff_num, bool reset);






#endif