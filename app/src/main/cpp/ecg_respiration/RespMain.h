#ifndef		_RESP_MAIN_H_
#define		_RESP_MAIN_H_
//RespMain 

//#include "Respiration_Rate.h"
#include "RespFinalResult.h"

#define 	QRS_LEN		30 		//计算呼吸率的输入的波峰或者RR间期的最大个数
#define		FS			250 	//心电信号的采样率为250Hz

// 呼吸波上升段或下降段的规格参数上下限

// 算法相关运算结果结构体
typedef struct Resp_Alg_Result
{
	short RespMax; 
	short RespMin; 
	short RespMean;

	short RespRate;					//呼吸率
	float RespVar;          //呼吸深度
	float RespVC;           //肺活量  Vital capacity
	float RespCalibration;           //肺活量  Vital capacity

}RESP_ALG_RESULT;

const char* Get_Version_Respiration(void);


void RespMain(RESP_ALG_RESULT *Resp_Result, float Calibration, bool reset);

#endif
