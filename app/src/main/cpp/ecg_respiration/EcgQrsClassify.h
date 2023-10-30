//*************************************************************************
//文件名称：EcgQrsClassify.h
//
//文件说明：QRS波分类函数
//
//更改历史：Created by Chenmaomao [9 / 8 / 2014]
//*************************************************************************

#ifndef  _ECGQRSCLASSIFY_H_
#define _ECGQRSCLASSIFY_H_

#include "EcgMainProc.h"



//=========================================================================
//1. 定义结构体、常量
//=========================================================================
// QRS模板的宽度
#define QRS_TEMP_MATCH_LEN		41
// 根据一些数据调试，正常QRS的宽度约 （15-18）×4 = （60 - 72）ms；PVC约 40×4 = 160ms，因此设置为23个采样点
// （原为16个采样点）
#define PVC_MIN_WIDTH					23
#define QRS_TEMPLATE_LEN			61

// qrs波模板
typedef struct Qrs_Template
{
	// 下面是单个QRS波的相关变量
	short nQrsTemp[QRS_TEMPLATE_LEN];			//Qrs模板

	unsigned short nQrsType;					//该模板的类型，0=初始，1=正常，3=未分类，100=PVC
	unsigned short nStartPos;					//数据缓存区中的起点位置
	unsigned short nPeakPos;					//数据缓存区中的峰值点位置
	unsigned short nEndPos;					//数据缓存区中的终点位置
	unsigned short nQrsWidth;				// QRS波宽度

	int nCoefNormal;									//该QRS波与正常模板的相关系数
	int nCoefPvc;											//该QRS波与PVC的相关系数

	// 下面是QRS总体模板的相关变量
	bool bIsTemplateExist;						//该类型的模板是否存在
	int nTemplateNum;								//该类型的QRS波的个数

}QRS_TEMPLATE;


//=========================================================================
//2.定义相关变量
//=========================================================================
extern QRS_TEMPLATE gNormTemplate;
extern QRS_TEMPLATE gPvcTemplate;
extern unsigned int gPvcNumAll;							// PVC的个数
extern unsigned int gPvcNumAllLast;							// PVC的个数


//=========================================================================
//3.定义相关函数
//=========================================================================
// 分类初始化主函数
void QrsClassifyInitialize(void);

// 分类主函数
int QrsClassifyMain(bool reset);

// 模板变量初始化函数
void QrsTemplateInitialize(QRS_TEMPLATE *qrs_template);

// 计算相关系数
int CalculateCoef(QRS_TEMPLATE *cur_qrs, QRS_TEMPLATE *template_qrs, int offset);

// 移位并计算相关系数
int ShiftAndCalcCoef(QRS_TEMPLATE *template_qrs, int cur_coef);


#endif
