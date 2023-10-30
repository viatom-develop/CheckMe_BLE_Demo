#ifndef		_RESP_MAIN_H_
#define		_RESP_MAIN_H_
//RespMain 

//#include "Respiration_Rate.h"
#include "RespFinalResult.h"

#define 	QRS_LEN		30 		//��������ʵ�����Ĳ������RR���ڵ�������
#define		FS			250 	//�ĵ��źŵĲ�����Ϊ250Hz

// �����������λ��½��εĹ�����������

// �㷨����������ṹ��
typedef struct Resp_Alg_Result
{
	short RespMax; 
	short RespMin; 
	short RespMean;

	short RespRate;					//������
	float RespVar;          //�������
	float RespVC;           //�λ���  Vital capacity
	float RespCalibration;           //�λ���  Vital capacity

}RESP_ALG_RESULT;

const char* Get_Version_Respiration(void);


void RespMain(RESP_ALG_RESULT *Resp_Result, float Calibration, bool reset);

#endif
