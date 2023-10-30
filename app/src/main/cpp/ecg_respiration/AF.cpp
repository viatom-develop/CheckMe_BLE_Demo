//*************************************************************************
//�ļ����ƣ�AF.c
//
//�ļ�˵����ECG�ĵ�Rֵ,ɢ��ͼ�ж��Ƿ񷿲�
//
//������ʷ��Powered by MeiHuan [8 / 10 / 2018]
//*************************************************************************
//#include "stdafx.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "AF.h"

#define MAX_RPEAK_NUM		80			//���RR���ڸ���
#define MIN_RPEAK_NUM		34			//ɢ��ͼ������Ҫ����СRR���ڸ���
#define WINDOWSIZE			32

unsigned char af_buf[MAX_RPEAK_NUM] = {0};
unsigned char af_buf_num = 0;
unsigned char af_24[4] = {0};
unsigned char af_24_num = 0;

static unsigned Rvalue[MAX_RPEAK_NUM + 1];		//R��
static char Rpeak_num , R_num;					//RR���ڸ��� �� R�����

void ecg_af_init(void)
{
	memset(Rvalue, 0, MAX_RPEAK_NUM + 1);
	R_num = 0;
}

void get_Rvalue(unsigned value)
{
	if(R_num >= MAX_RPEAK_NUM + 1)
		return ;
	if(value == 0)		//�޳���ֵ
		return ;
	Rvalue[R_num] = value;
	R_num++;
}

char ecg_af_begin(void)
{
	unsigned char i,j,k,flag;
	unsigned short Rpeak[MAX_RPEAK_NUM];		//RR����
	short  Rpeak_diff [MAX_RPEAK_NUM];			//RR���ڶ��β��
	unsigned char af_num [MAX_RPEAK_NUM];		//��ֵ
	int temp = 0;
	int cnt = 0;
	int isAF = 0;
	
	flag = 0;
	af_buf_num = 0;
	af_24_num = 0;
	memset(af_24, 0, 4);
	
	if(R_num < MIN_RPEAK_NUM)	//Rֵ̫��
		return 0;
	
	memset(af_num, 0, 80);
	memset(Rpeak, 0, 80 * sizeof(unsigned short));
	memset(Rpeak_diff, 0, 80 * sizeof(short));
	
	//Rֵ��֣����RR����
	for(i = 1; i < R_num; i++){
		Rpeak[i - 1] = Rvalue[i] - Rvalue[i - 1];
	}
	Rpeak_num = R_num - 1;
	
	//RR�����ٴβ�֣��õ�Rpeak_diff
	for(i = 1; i < Rpeak_num; i++){
		Rpeak_diff[i - 1] = Rpeak[i] - Rpeak[i - 1];
	}
	
	//ȥ�����һ��Rpeakֵ����Rpeak_diff��������
	Rpeak[Rpeak_num - 1] = 0;
	Rpeak_num--;
	
	for(i = 0; i < Rpeak_num; i++){		
		Rpeak[i] /= (250 * 0.025);			//250Hz
		Rpeak_diff[i] /= (250 * 0.025);		//250Hz
	}
	
	for(k = 0; k < Rpeak_num - WINDOWSIZE; k++){	//����
		//�����Ա�
		for(i = k; i < WINDOWSIZE + k; i++){
			for(j = i+1; j < WINDOWSIZE + k; j++){
				if(Rpeak[i] == Rpeak[j] && Rpeak_diff[i] == Rpeak_diff[j]){		//�������ͬһ����
					flag = 1;
					break;
				}
			}
			if(flag == 0){		//�����е㶼����һ������
				af_num[k] ++;
			}
			flag = 0;
		}
		temp += af_num[k];	//af���ȡƽ��
		
		af_buf[k] =  af_num[k];		//����af_num�ļ�����
//		af_buf_num = k;
	}//end for body
	af_buf_num = k;
	
	for (i = 0; i < k; i++) {
		if (af_num[i] > 27) {
			if (cnt < 4) {				//�������27�����ݣ����4��
				af_24[cnt] = af_num[i];
				af_24_num = cnt+1;
			}
			cnt++;
		}
	}
	
	if (cnt > (k/2 < 20 ? k/2 : 20)) {
		isAF = 1;
	} else {
		isAF = 0;
	}
	
	return isAF;
		
		
	
	
//	//printf
//	printf("\r\nRvalue: ");
//	for(i = 0 ; i < R_num;i++){
//		printf("%d,",Rvalue[i]);
//	}
//	printf("\r\nRpeak: ");
//	for(i = 0 ; i < Rpeak_num;i++){
//		printf("%d ",Rpeak[i]);
//	}
//	printf("\r\nRpeak_diff: ");
//	for(i = 0 ; i < Rpeak_num;i++){
//		printf("%d ",Rpeak_diff[i]);
//	}
//	printf("\r\naf: ");
//	for(i = 0 ; i < 80;i++){
//		printf("%d ",af_num[i]);
//	}
//	return (temp/k);
}
