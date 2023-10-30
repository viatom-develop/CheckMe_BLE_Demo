//************************************************************************
// �ļ�����: EcgFilter.c
//
// �ļ�˵��: ����Ecg�˲�������غ���������
//
// �������ݣ�	������: 250Hz��500Hz��1000Hz ���ֲ�����
//				��������: ���֧��24λ��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************

//#include "stdafx.h"
#include "EcgFilter.h"


//=========================================================================
//����FIR�˲�������س���ϵ��
//=========================================================================
#define FIR_LP20_LENGTH 5
#define FIR_LP20_250	4

#define FIR_HP5_LENGTH	33
#define FIR_HP5_250		32

#define ECG_DIFF_LENGTH	5
#define ECG_DIFF_250	4

#define SQUARE_INTEG_LENGTH	33
#define SQUARE_INTEG_250	32

//======================================================================
// �Ѳ��㷨�˲�����������
//======================================================================

// 20HzFir��ͨ�˲�������һ�������������֧��1000Hz�����ʣ����泤��λ16��
int gFir20LpFormerBuff[FIR_LP20_LENGTH];
int gFir20LpFormerSum;

int gFir20LpLatterBuff[FIR_LP20_LENGTH];
int gFir20LpLatterSum;
int gFir20LpPtr;

// 5HzFir��ͨ�˲�������һ�������������֧��1000Hz�����ʣ����泤��Ϊ100��
int gFir5HpFormerBuff[FIR_HP5_LENGTH];
int gFir5HpFormerSum;

int gFir5HpLatterBuff[FIR_HP5_LENGTH];
int gFir5HpLatterSum;
int gFir5HpPtr;

// 5HzFir������ͨ�˲�������һ�������������֧��1000Hz�����ʣ����泤��Ϊ100��
int gIndFir5HpFormerBuff[FIR_HP5_LENGTH];
int gIndFir5HpFormerSum;

int gIndFir5HpLatterBuff[FIR_HP5_LENGTH];
int gIndFir5HpLatterSum;
int gIndFir5HpPtr;

// ����˲���ѭ��������������Ϊ16��
int gDiffFilterBuff[ECG_DIFF_LENGTH];
int gDiffPtr;

// ƽ�����ֻ�����������Ϊ128��
int gSquareIntegBuff[SQUARE_INTEG_LENGTH];
int gSquareIntegPtr;
int gSquareIntegSum;


//************************************************************************
// ��������: void InitializeEcgFilter(void)
//
// ����˵��: �˲�����ʼ������
//
// �������: ��	
//
// �������: ��
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
void InitializeEcgFilter(void)
{
	//======================================================================
	// �Ѳ��˲����ֳ�ʼ��
	//======================================================================
	Fir20LowPass(0, true);
	Fir5HighPass(0, true);
	IndFir5HighPass(0, true);
	EcgDiffFilter(0, true);
	SquareIntegration(0, true);

	//	����
	return ;
}


//======================================================================
// �Ѳ��㷨���õ�����˲���
//======================================================================

//************************************************************************
// ��������: int Fir20LowPass(int data,   bool reset)
//
// ����˵��: 20Hz FIR��ͨ�˲���
//
// �������: int data:   �˲�ǰ����������
//			 int gEcgOrgSampleRate:  ������
//			 bool reset: ��ʼ����־	
//
// �������: int result: �˲��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int Fir20LowPass(int data, bool reset)
{
	int temp = 0, i = 0, final_result = 0, sub_ptr = 0;
	int buffer_len = 0, shift_bit = 0;

	if (reset)
	{
		for (i = 0; i < FIR_LP20_LENGTH; i++)
		{
			gFir20LpFormerBuff[i] = 0;
			gFir20LpLatterBuff[i] = 0;
		}
		gFir20LpPtr = 0;
		gFir20LpFormerSum = 0;
		gFir20LpLatterSum = 0;
	}
	else
	{
		//======================================================================
		// Part 1: ���ݲ����ʸ�ֵ��Ӧ��ϵ����Ŀǰ��֧��250Hz
		//======================================================================
		buffer_len = FIR_LP20_250;
		shift_bit = 2;

		//======================================================================
		// Part 1: ��һ��ƽ��
		//======================================================================
		sub_ptr = LoopInc(gFir20LpPtr, 1, (buffer_len + 1));

		gFir20LpFormerBuff[gFir20LpPtr] = data;

		gFir20LpFormerSum += data;
		gFir20LpFormerSum -= gFir20LpFormerBuff[sub_ptr];

		temp = gFir20LpFormerSum >> shift_bit;

		//======================================================================
		// Part 2: �ڶ���ƽ����Ȼ��������
		//======================================================================
		gFir20LpLatterBuff[gFir20LpPtr] = temp;
		gFir20LpLatterSum += temp;
		gFir20LpLatterSum -= gFir20LpLatterBuff[sub_ptr];

		final_result = gFir20LpLatterSum >> shift_bit;

		// ָ��������ָ����һ��λ��
		gFir20LpPtr = LoopInc(gFir20LpPtr, 1, (buffer_len + 1));
	}

	return final_result;
}

//************************************************************************
// ��������: int Fir5HighPass(int data,   bool reset)
//
// ����˵��: 4.5Hz FIR��ͨ�˲���
//
// �������: int data:   �˲�ǰ����������
//			 int gEcgOrgSampleRate:  ������
//			 bool reset: ��ʼ����־	
//
// �������: int result: �˲��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int Fir5HighPass(int data, bool reset)
{
	int i = 0, sub_ptr = 0, add_ptr = 0, temp = 0, final_result = 0;
	int buffer_len = 0, section_num = 0, shift_bit = 0;

	if (reset)
	{
		for (i = 0; i < FIR_HP5_LENGTH; i++)
		{
			gFir5HpFormerBuff[i] = 0;
			gFir5HpLatterBuff[i] = 0;
		}
		gFir5HpFormerSum = 0;
		gFir5HpLatterSum = 0;
		gFir5HpPtr = 0;
	}
	else
	{
		//======================================================================
		// Part 1: ���ݲ����ʸ�ֵ��Ӧ��ϵ������֧��250Hz
		//======================================================================
		buffer_len = FIR_HP5_250;
		shift_bit = 5;

		//======================================================================
		// Part 1: ��һ��ƽ��
		//======================================================================
		sub_ptr = LoopInc(gFir5HpPtr, 1, (buffer_len + 1));

		gFir5HpFormerBuff[gFir5HpPtr] = data;

		gFir5HpFormerSum += data;
		gFir5HpFormerSum -= gFir5HpFormerBuff[sub_ptr];

		temp = gFir5HpFormerSum >> shift_bit;

		//======================================================================
		// Part 2: �ڶ���ƽ����Ȼ��������
		//======================================================================
		gFir5HpLatterBuff[gFir5HpPtr] = temp;
		gFir5HpLatterSum += temp;
		gFir5HpLatterSum -= gFir5HpLatterBuff[sub_ptr];

		temp = gFir5HpLatterSum >> shift_bit;

		//======================================================================
		// Part 3: ԭʼ������ʱ��Ȼ���ȥ���ߣ��õ���ͨ����
		//======================================================================
		//final_result = gFir5HpFormerBuff[sub_ptr] - (temp >> shift_bit);
		final_result = gFir5HpFormerBuff[sub_ptr] - temp;

		// ָ��ָ��ѭ������������һ��λ��
		gFir5HpPtr = LoopInc(gFir5HpPtr, 1, (buffer_len + 1));
	}

	return final_result;
}


//************************************************************************
// ��������: int Fir5HighPassInd(int data,   bool reset)
//
// ����˵��: 4.5Hz FIR��ͨ�˲���
//					�ú�������ʹ�ã����ڻ�ȡ��ͨ�˲�����
//
// �������: int data:   �˲�ǰ����������
//			 int gEcgOrgSampleRate:  ������
//			 bool reset: ��ʼ����־	
//
// �������: int result: �˲��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int IndFir5HighPass(int data, bool reset)
{
	int i = 0, sub_ptr = 0, add_ptr = 0, temp = 0, final_result = 0;
	int buffer_len = 0, section_num = 0, shift_bit = 0;

	if (reset)
	{
		for (i = 0; i < FIR_HP5_LENGTH; i++)
		{
			gIndFir5HpFormerBuff[i] = 0;
			gIndFir5HpLatterBuff[i] = 0;
		}
		gIndFir5HpFormerSum = 0;
		gIndFir5HpLatterSum = 0;
		gIndFir5HpPtr = 0;
	}
	else
	{
		//======================================================================
		// Part 1: ���ݲ����ʸ�ֵ��Ӧ��ϵ������֧��250Hz
		//======================================================================
		buffer_len = FIR_HP5_250;
		shift_bit = 5;

		//======================================================================
		// Part 1: ��һ��ƽ��
		//======================================================================
		sub_ptr = LoopInc(gIndFir5HpPtr, 1, (buffer_len + 1));

		gIndFir5HpFormerBuff[gIndFir5HpPtr] = data;

		gIndFir5HpFormerSum += data;
		gIndFir5HpFormerSum -= gIndFir5HpFormerBuff[sub_ptr];

		temp = gIndFir5HpFormerSum >> shift_bit;

		//======================================================================
		// Part 2: �ڶ���ƽ����Ȼ��������
		//======================================================================
		gIndFir5HpLatterBuff[gIndFir5HpPtr] = temp;
		gIndFir5HpLatterSum += temp;
		gIndFir5HpLatterSum -= gIndFir5HpLatterBuff[sub_ptr];

		temp = gIndFir5HpLatterSum >> shift_bit;

		//======================================================================
		// Part 3: ԭʼ������ʱ��Ȼ���ȥ���ߣ��õ���ͨ����
		//======================================================================
		//final_result = gFir5HpFormerBuff[sub_ptr] - (temp >> shift_bit);
		final_result = gIndFir5HpFormerBuff[sub_ptr] - temp;

		// ָ��ָ��ѭ������������һ��λ��
		gIndFir5HpPtr = LoopInc(gIndFir5HpPtr, 1, (buffer_len + 1));
	}

	return final_result;
}


//************************************************************************
// ��������: int EcgDiffFilter(int data,   bool reset)
//
// ����˵��: ����˲���
//
// �������: int data:   �˲�ǰ����������
//			 int gEcgOrgSampleRate:  ������
//			 bool reset: ��ʼ����־	
//
// �������: int result: �˲��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int EcgDiffFilter(int data, bool reset)
{
	int i = 0, result = 0, buffer_len = 0, sub_ptr = 0;

	if (1 == reset)
	{
		for (i = 0; i < ECG_DIFF_LENGTH; i++)
		{
			gDiffFilterBuff[i] = 0;
		}
		gDiffPtr = 0;
	}
	else
	{
		//=========================================================================
		//1. ���ݲ����ʸ�ֵ���������ȣ���֧��250Hz
		//=========================================================================
		buffer_len = ECG_DIFF_250;	

		//=========================================================================
		//2. ��������ֵ�������ֽ����ά������ָ��
		//=========================================================================
		sub_ptr = LoopInc(gDiffPtr, 1, buffer_len);

		gDiffFilterBuff[gDiffPtr] = data;
		result = gDiffFilterBuff[gDiffPtr] * 3 - gDiffFilterBuff[sub_ptr] * 3;

		gDiffPtr = LoopInc(gDiffPtr, 1, buffer_len);
	}

	return result;
}

//************************************************************************
// ��������: int SquareIntegration(int data,   bool reset)
//
// ����˵��: ȡ����ֵ��ƽ��̫�󣬲�������24bit���ݣ���Ȼ����л�������
//
// �������: int data:   �˲�ǰ����������
//			 int gEcgOrgSampleRate:  ������
//			 bool reset: ��ʼ����־	
//
// �������: int result: �˲��������
//
// ��    ʷ: Created by Chen Maomao : 6 / 15 / 2013   
//************************************************************************
int SquareIntegration(int data, bool reset)
{
	int i = 0, result = 0, buffer_len = 0, sub_ptr = 0, shift_bit = 0;

	if (1 == reset)
	{
		for (i = 0; i < SQUARE_INTEG_LENGTH; i++)
		{
			gSquareIntegBuff[i] = 0;
		}
		gSquareIntegPtr = 0;
		gSquareIntegSum = 0;
	}
	else
	{
		//=========================================================================
		//1. ���ݲ����ʸ�ֵ���������ȣ�250Hz
		//=========================================================================
		buffer_len = SQUARE_INTEG_250;
		shift_bit = 5;

		//=========================================================================
		//2. ȡ����ֵ������ֵ������
		//=========================================================================
		sub_ptr = LoopInc(gSquareIntegPtr, 1, (buffer_len + 1));
		gSquareIntegBuff[gSquareIntegPtr] = (data >= 0) ? data : (-data);	//Ϊ����������㣬���þ���ֵ

		//=========================================================================
		//3. ��������
		//=========================================================================
		gSquareIntegSum += gSquareIntegBuff[gSquareIntegPtr];
		gSquareIntegSum -= gSquareIntegBuff[sub_ptr];

		result = gSquareIntegSum >> shift_bit;

		//ά��ָ�룬ָ����һ����
		gSquareIntegPtr = LoopInc(gSquareIntegPtr, 1, (buffer_len + 1));
	}

	return result;
}

