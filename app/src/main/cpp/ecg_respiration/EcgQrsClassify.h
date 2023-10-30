//*************************************************************************
//�ļ����ƣ�EcgQrsClassify.h
//
//�ļ�˵����QRS�����ຯ��
//
//������ʷ��Created by Chenmaomao [9 / 8 / 2014]
//*************************************************************************

#ifndef  _ECGQRSCLASSIFY_H_
#define _ECGQRSCLASSIFY_H_

#include "EcgMainProc.h"



//=========================================================================
//1. ����ṹ�塢����
//=========================================================================
// QRSģ��Ŀ��
#define QRS_TEMP_MATCH_LEN		41
// ����һЩ���ݵ��ԣ�����QRS�Ŀ��Լ ��15-18����4 = ��60 - 72��ms��PVCԼ 40��4 = 160ms���������Ϊ23��������
// ��ԭΪ16�������㣩
#define PVC_MIN_WIDTH					23
#define QRS_TEMPLATE_LEN			61

// qrs��ģ��
typedef struct Qrs_Template
{
	// �����ǵ���QRS������ر���
	short nQrsTemp[QRS_TEMPLATE_LEN];			//Qrsģ��

	unsigned short nQrsType;					//��ģ������ͣ�0=��ʼ��1=������3=δ���࣬100=PVC
	unsigned short nStartPos;					//���ݻ������е����λ��
	unsigned short nPeakPos;					//���ݻ������еķ�ֵ��λ��
	unsigned short nEndPos;					//���ݻ������е��յ�λ��
	unsigned short nQrsWidth;				// QRS�����

	int nCoefNormal;									//��QRS��������ģ������ϵ��
	int nCoefPvc;											//��QRS����PVC�����ϵ��

	// ������QRS����ģ�����ر���
	bool bIsTemplateExist;						//�����͵�ģ���Ƿ����
	int nTemplateNum;								//�����͵�QRS���ĸ���

}QRS_TEMPLATE;


//=========================================================================
//2.������ر���
//=========================================================================
extern QRS_TEMPLATE gNormTemplate;
extern QRS_TEMPLATE gPvcTemplate;
extern unsigned int gPvcNumAll;							// PVC�ĸ���
extern unsigned int gPvcNumAllLast;							// PVC�ĸ���


//=========================================================================
//3.������غ���
//=========================================================================
// �����ʼ��������
void QrsClassifyInitialize(void);

// ����������
int QrsClassifyMain(bool reset);

// ģ�������ʼ������
void QrsTemplateInitialize(QRS_TEMPLATE *qrs_template);

// �������ϵ��
int CalculateCoef(QRS_TEMPLATE *cur_qrs, QRS_TEMPLATE *template_qrs, int offset);

// ��λ���������ϵ��
int ShiftAndCalcCoef(QRS_TEMPLATE *template_qrs, int cur_coef);


#endif
