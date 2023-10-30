//************************************************************************
// �ļ�����: EcgFilter.h
//
// �ļ�˵��: ����Ecg�˲�������غ���������
//
// �������ݣ�	������: 250Hz��500Hz��1000Hz ���ֲ�����
//				��������: ���֧��24λ��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************

#ifndef _ECGFILTER_H_
#define _ECGFILTER_H_

#include "EcgMainProc.h"


//=========================================================================
// ���峣��
//=========================================================================
// �ⲿ��ʾ�˲��ӳ�
#define DELAY_VIEW_HP05		499			// 0.5Hz��ͨ��ʱ��499�㣨500Hz��
#define DELAY_VIEW_LP20			11			// 20Hz��ͨ��ʱ��11�㣨500Hz��
#define DELAY_VIEW_LP40			4				// 40Hz��ͨ��ʱ��4�㣨500Hz��

// �㷨�ڲ��˲��ӳ�
#define HIGH_PASS_DELAY		32			//5Hz��ͨFIR�˲��ӳ٣�32�㣨250Hz��
#define LOW_PASS_DELAY		4				//20Hz��ͨFIR�˲��ӳ٣�4�㣨250Hz��
#define DIFF_PASS_DELAY			2				//����˲��ӳ٣�2�㣨250Hz��
#define BAND_PASS_DELAY		HIGH_PASS_DELAY + LOW_PASS_DELAY			//��ͨ�˲��ӳ٣�36�㣨250Hz��



//======================================================================
// Methods declaration
//======================================================================
void InitializeEcgFilter(void);

int Fir20LowPass(int data, bool reset);
int Fir5HighPass(int data, bool reset);
int IndFir5HighPass(int data, bool reset);
int EcgDiffFilter(int data, bool reset);
int SquareIntegration(int data, bool reset);

#endif
