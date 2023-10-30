//*************************************************************************
//文件名称：AF.c
//
//文件说明：ECG心电R值,散点图判断是否房颤
//
//更改历史：Powered by MeiHuan [8 / 10 / 2018]
//*************************************************************************
//#include "stdafx.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "AF.h"

#define MAX_RPEAK_NUM		80			//最大RR间期个数
#define MIN_RPEAK_NUM		34			//散点图运算需要的最小RR间期个数
#define WINDOWSIZE			32

unsigned char af_buf[MAX_RPEAK_NUM] = {0};
unsigned char af_buf_num = 0;
unsigned char af_24[4] = {0};
unsigned char af_24_num = 0;

static unsigned Rvalue[MAX_RPEAK_NUM + 1];		//R点
static char Rpeak_num , R_num;					//RR间期个数 ， R点个数

void ecg_af_init(void)
{
	memset(Rvalue, 0, MAX_RPEAK_NUM + 1);
	R_num = 0;
}

void get_Rvalue(unsigned value)
{
	if(R_num >= MAX_RPEAK_NUM + 1)
		return ;
	if(value == 0)		//剔除空值
		return ;
	Rvalue[R_num] = value;
	R_num++;
}

char ecg_af_begin(void)
{
	unsigned char i,j,k,flag;
	unsigned short Rpeak[MAX_RPEAK_NUM];		//RR间期
	short  Rpeak_diff [MAX_RPEAK_NUM];			//RR间期二次差分
	unsigned char af_num [MAX_RPEAK_NUM];		//出值
	int temp = 0;
	int cnt = 0;
	int isAF = 0;
	
	flag = 0;
	af_buf_num = 0;
	af_24_num = 0;
	memset(af_24, 0, 4);
	
	if(R_num < MIN_RPEAK_NUM)	//R值太少
		return 0;
	
	memset(af_num, 0, 80);
	memset(Rpeak, 0, 80 * sizeof(unsigned short));
	memset(Rpeak_diff, 0, 80 * sizeof(short));
	
	//R值差分，获得RR间期
	for(i = 1; i < R_num; i++){
		Rpeak[i - 1] = Rvalue[i] - Rvalue[i - 1];
	}
	Rpeak_num = R_num - 1;
	
	//RR间期再次差分，得到Rpeak_diff
	for(i = 1; i < Rpeak_num; i++){
		Rpeak_diff[i - 1] = Rpeak[i] - Rpeak[i - 1];
	}
	
	//去掉最后一个Rpeak值，与Rpeak_diff数量对齐
	Rpeak[Rpeak_num - 1] = 0;
	Rpeak_num--;
	
	for(i = 0; i < Rpeak_num; i++){		
		Rpeak[i] /= (250 * 0.025);			//250Hz
		Rpeak_diff[i] /= (250 * 0.025);		//250Hz
	}
	
	for(k = 0; k < Rpeak_num - WINDOWSIZE; k++){	//窗口
		//遍历对比
		for(i = k; i < WINDOWSIZE + k; i++){
			for(j = i+1; j < WINDOWSIZE + k; j++){
				if(Rpeak[i] == Rpeak[j] && Rpeak_diff[i] == Rpeak_diff[j]){		//如果处在同一方格
					flag = 1;
					break;
				}
			}
			if(flag == 0){		//与所有点都不在一个方格
				af_num[k] ++;
			}
			flag = 0;
		}
		temp += af_num[k];	//af求和取平均
		
		af_buf[k] =  af_num[k];		//保存af_num文件缓存
//		af_buf_num = k;
	}//end for body
	af_buf_num = k;
	
	for (i = 0; i < k; i++) {
		if (af_num[i] > 27) {
			if (cnt < 4) {				//保存大于27的数据，最多4个
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
