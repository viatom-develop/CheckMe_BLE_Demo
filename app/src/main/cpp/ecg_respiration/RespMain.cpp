//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "RespMain.h"
#include "EcgResult.h"
#include "assistTools.h"

//extern short Max;
//extern short Min;	
//extern short Aver_All;


extern short PeakNum;
//extern float RespIndex;
//extern float RespIndex_Base; 

extern unsigned short gQrsPeakNum; 	//QRS�������ڲ���ĸ���


unsigned short 	real_qrs_len;			//ʵ�ʿ��õ�QRS�������
int 	Peak_Array[QRS_LEN]; 			//���沨�����ֵ
int		RR_Array[QRS_LEN];				//����RR���ڿ��ֵ
int 	pos_Array[QRS_LEN];				//���沨��λ��

//�������������
const unsigned char 	len_out_array 	= 12;
float 			resp_out_array[len_out_array];
unsigned char 	out_index 	= 0;
unsigned char 	resp_display_tag 	= 0;

//extern short MaxSize_Peak; 

const char* Get_Version_Respiration(void)
{
	const char* Version_SPO2 = "Resp_V0.0.6";
	return Version_SPO2;
};


void RespMain(RESP_ALG_RESULT *Resp_Result,float Calibration,bool reset)
{
	int 	PeakValue;
	int 	RRIntervel;
	int 	PeakTime,	Pre_PeakTime;

	short 	RespMax 	= 0; 
	short 	RespMin 	= 0; 
	short 	RespMean 	= 0; 	
	float 	RespRate 	= 0;
	float 	RespVar 	= 0;
	short 	i 	= 0, 	j  	= 0;


//	short MeasureTime			= 0;

	// �λ�������--------------------------------------//
	if(reset)
	{		
		Resp_Result->RespMax 	= 0;
		Resp_Result->RespMin 	= 0;
		Resp_Result->RespMean 	= 0;

		Resp_Result->RespRate 	= 0;
		Resp_Result->RespVar 	= 0;
		Resp_Result->RespVC 	= 0;
		Resp_Result->RespCalibration 	= 0;

		return;	
	}

	//ÿ�ν�ȡ������������Len�����弰RR���������������
	//�ж��Ƿ�����������������Ĳ������
	if( gQrsPeakNum < 15 ) 				// 30��ֻ�ɼ���15������QRS�����򲻲���������
		return;		
	if( gQrsPeakNum >= QRS_LEN ){
		j 	= gQrsPeakNum - QRS_LEN;	//��ȡ�ĵ�һ������������λ��
		real_qrs_len 	= QRS_LEN;
	}
	else{
		j 	= 0;
		real_qrs_len 	= gQrsPeakNum;
	}
		
	
	for (i = 0; i < real_qrs_len; i++)
	{
		Peak_Array[i] 	= gQrsPeakBuff[j].nQrsAmp;
		RR_Array[i] 	= gQrsPeakBuff[j].nRRInterval;
		pos_Array[i]	= gQrsPeakBuff[j].nPeakPosAll;
		j++;
	}
	RR_Array[0]		= RR_Array[1]; 		//��1λ�����0λ��
	//	save MyValues.hex &Peak_Array[0], &Peak_Array[gQrsPeakNum-1];
	//	log > MyValues.log
	//  d &Peak_Array[0], &Peak_Array[gQrsPeakNum]
	//  displayvalues();
	//	define button "Log Array", "displayvalues()"

	//MeasureTime = gQrsPeakBuff[gQrsPeakNum-1].nPeakPosAll - gQrsPeakBuff[0].nPeakPosAll;

	//�����쳣��ֵ������ǰ�����ݵ�ƽ�������	
	for (i = 1; i < real_qrs_len-1; i++)
	{
		if( Peak_Array[i]>1.36*Peak_Array[i-1] && Peak_Array[i]>1.36*Peak_Array[i+1] )
			Peak_Array[i]	=(Peak_Array[i-1] + Peak_Array[i+1]) / 2;
		
		if( Peak_Array[i-1]>1.36*Peak_Array[i] && Peak_Array[i+1]>1.36*Peak_Array[i] )
			Peak_Array[i]	=(Peak_Array[i-1]+Peak_Array[i+1]) / 2;	
		
		if( RR_Array[i]>1.36*RR_Array[i-1] && RR_Array[i]>1.36*RR_Array[i+1] )
			RR_Array[i]		=(RR_Array[i-1] + RR_Array[i+1]) / 2;
		
		if( RR_Array[i-1]>1.36*RR_Array[i] && RR_Array[i+1]>1.36*RR_Array[i] )
			RR_Array[i]		=(RR_Array[i-1] + RR_Array[i+1]) / 2;			
	}

//	//ͨ��QRS��ֵ���������������
//	peakValley( &Peak_Array[0], QRS_LEN );
//	RespRate_1 		= 15000*PeakNum/(pos_Array[QRS_LEN-1] - pos_Array[0]);
//	RespIndex_1 	= RespIndex * 100;
//	peak_number_1 	= PeakNum;

//	//ͨ��RR�������������������
//	peakValley( &RR_Array[0], QRS_LEN );	
//	RespRate_2 		= 15000*PeakNum/(pos_Array[QRS_LEN-1] - pos_Array[1]);
//	RespIndex_2 	= RespIndex * 100;
//	peak_number_2 	= PeakNum;
//	
//	respiration_rate 	= RespRate_2*0.75 + RespRate_1*0.25;

	RespRate 	= (float)calRespiration( );
	
	// Լ�������ʵĳ�ֵ��ΧHR/3
	if( RespRate > gEcgFinalResult.nEcgHeartRate/3 )
		RespRate 	= gEcgFinalResult.nEcgHeartRate/3;


	//	Resp_Result->RespRate = (int)(RespRate_1+RespRate_2)/2;
	//	Resp_Result->RespVar = (RespIndex_1+RespIndex_2)/2;	
	
	//�����ʵ�ƽ�����
//	resp_out_array[ out_index ] 	= RespRate;
//	if( ++out_index >= len_out_array ){
//		out_index 	= 0;
//		resp_display_tag 	= 1;	//����������ĵ�һ������ʼ��ʾ������
//	}

//	Resp_Result->RespMax 	= Max;
//	Resp_Result->RespMin 	= Min;
//	Resp_Result->RespMean 	= Aver_All;

	//�����ʸ�ֵ����Peak-rateΪ����
//	if( resp_display_tag )
//		Resp_Result->RespRate = (int)(mean_float(resp_out_array, len_out_array) );
	Resp_Result->RespRate = (short)(RespRate + 0.5);

//	if( Calibration )
//	{
//		//Resp_Result->RespVC = 388.0*RespIndex/RespIndex_Base+1400;	
//		Resp_Result->RespVC = 2000.0*RespIndex/Calibration + 400;	
//	}
//	else
//	{
//		Resp_Result->RespCalibration 	= RespIndex;
//	}

	return;			
}


