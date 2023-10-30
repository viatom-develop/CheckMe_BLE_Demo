//*************************************************************************
//文件名称：EcgQrsClassify.cpp
//
//文件说明：QRS波分类函数
//
//更改历史：Created by Chenmaomao [9 / 8 / 2014]
//*************************************************************************

//#include "stdafx.h"
#include <math.h>
#include "EcgQrsClassify.h"


//=========================================================================
//定义QRS分类相关的内部变量
//=========================================================================
QRS_TEMPLATE wCurSortQrs;					//当前待分类的QRS波，从EcgQrsDetect中的BandPeakDetect函数获取
QRS_TEMPLATE gNormTemplate;
QRS_TEMPLATE gPvcTemplate;
QRS_TEMPLATE gPvcTemplateTwo;
QRS_TEMPLATE gPvcTemplateThree;
QRS_TEMPLATE wAbnormTemplate;

QRS_PEAK wCurQrs;

unsigned int wLowCoefCont;					//与主导模板的相关系数大于80小于90的连续出现的个数
unsigned int wLowPvcCont;						//与主导模板的相关系数大于80小于90的连续出现的个数
unsigned int gPvcNumAll;							// PVC的个数
unsigned int gPvcNumAllLast;					// 前面几分钟的PVC个数，主要是用来计算当前一分钟的PVC

//=========================================================================
//分类初始化
//=========================================================================
void QrsClassifyInitialize()
{
	QrsTemplateInitialize(&wCurSortQrs);
	QrsTemplateInitialize(&gNormTemplate);
	QrsTemplateInitialize(&gPvcTemplate);
	QrsTemplateInitialize(&gPvcTemplateTwo);
	QrsTemplateInitialize(&gPvcTemplateThree);
	QrsTemplateInitialize(&wAbnormTemplate);

	wLowCoefCont = 0;
	wLowPvcCont   = 0;
	gPvcNumAll = 0;
	gPvcNumAllLast = 0;
}

int JudgeNormalTemp()
{
	int cur_coef = 0, i = 0;

	//2.1 判断主导模板是否存在
	if (!gNormTemplate.bIsTemplateExist)
	{
		// 主导模板尚不存在，用当前QRS波填充主导模板，然后返回
		gNormTemplate = wCurSortQrs;
		gNormTemplate.bIsTemplateExist = true;
		return 1;
	}

	//2.2 至此，说明主导模板存在，计算相关系数
	//		此处的思路是，首先以顶点为标准计算相关系数，如果大于80，就不偏移必对了。
	//		如果小于80，则左右偏移11个点，取最大值
	//		既可以减少运算量，也可以改善因为顶点找偏而造成的相关系数过低的问题
	cur_coef = CalculateCoef(&wCurSortQrs, &gNormTemplate, 0);

	if (85 > cur_coef)
	{
		// 左右移位，并计算最大的相关系数
		cur_coef = ShiftAndCalcCoef(&gNormTemplate, cur_coef);
	}
	wCurQrs.nCoefNormal = cur_coef;

	//2.3 判断是否属于主导模板
	if (85 <= cur_coef)
	{
		// 与主导模板的相关系数80<coef<90，说明形态不太相符，判为主导，但不更新模板
		// 设置当前QRS波的类型
		wCurSortQrs.nQrsType = 1;
		wCurQrs.nQrsType = wCurSortQrs.nQrsType;

		// 主导模板个数加1
		gNormTemplate.nTemplateNum++;

		if (90 <= cur_coef)
		{
			// 低相关的QRS波个数清零
			wLowCoefCont = 0;

			// 用最新的QRS波，填充主导模板
			for (i = 0; i < QRS_TEMPLATE_LEN; i++)
			{
				gNormTemplate.nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
			}
		}
		else
		{
			// 低相关的QRS波个数自增
			wLowCoefCont ++;

			// 如果连续出现3个低相关的QRS波，说明目前的主导模板不太对，用当前QRS波进行更新
			if (3 <= wLowCoefCont)
			{
				// 用最新的QRS波，填充主导模板
				for (i = 0; i < QRS_TEMPLATE_LEN; i++)
				{
					gNormTemplate.nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
				}

				// 低相关计数器清零
				wLowCoefCont = 0;
			}
		}
	}
	else
	{
		// 与正常模板没有匹配成功，返回0，进行下面的匹配
		return 0;
	}

	return 1;
}


int JudgeAbnormalTemp()
{
	int cur_coef = 0, i = 0;

	if (!wAbnormTemplate.bIsTemplateExist)
	{
		//3.1 "非正常"模板不存在，填充
		wAbnormTemplate = wCurSortQrs;
		wAbnormTemplate.bIsTemplateExist = true;
	}
	else
	{
		//3.2 "非正常"模板存在，计算相关系数
		cur_coef = CalculateCoef(&wCurSortQrs, &wAbnormTemplate, 0);

		if (80 > cur_coef)
		{
			// 左右移位，并计算最大的相关系数
			cur_coef = ShiftAndCalcCoef(&wAbnormTemplate, cur_coef);
		}
		wCurQrs.nCoefNormal = cur_coef + 100;

		//3.3 根据相关系数判断是否匹配该模板
		if (80 <= cur_coef)
		{
			// 相关度高，判定为Abnormal，更新Abnormal模板
			// 设置当前QRS波的类型
			wCurSortQrs.nQrsType = 3;
			wCurQrs.nQrsType = wCurSortQrs.nQrsType;

			// 主导模板个数加1
			wAbnormTemplate.nTemplateNum++;

			// 用最新的QRS波，更新异常模板
			for (i = 0; i < QRS_TEMPLATE_LEN; i++)
			{
				wAbnormTemplate.nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
			}
		}
		else
		{
			// 至此，说明与主导模板不匹配，与“非正常”模板页不匹配，直接更新“非正常”模板
			wAbnormTemplate = wCurSortQrs;
			wAbnormTemplate.nTemplateNum = 0;
			wAbnormTemplate.bIsTemplateExist = true;
		}
	}

	return 1;
}


int JudgePvcTemp(QRS_TEMPLATE *PvcTemp)
{
	int cur_coef = 0, i = 0;

	if (!PvcTemp->bIsTemplateExist)
	{
		//3.1 "非正常"模板不存在，填充
		*PvcTemp = wCurSortQrs;
		PvcTemp->bIsTemplateExist = true;
		return 1;
	}
	else
	{
		//3.2 "非正常"模板存在，计算相关系数
		cur_coef = CalculateCoef(&wCurSortQrs, PvcTemp, 0);

		if (80 > cur_coef)
		{
			// 左右移位，并计算最大的相关系数
			cur_coef = ShiftAndCalcCoef(PvcTemp, cur_coef);
		}
		wCurQrs.nCoefNormal = cur_coef + 200;

		//3.3 根据相关系数判断是否匹配该模板
		if (80 <= cur_coef)
		{
			// 相关度高，判定为PVC，更新PVC模板
			// 设置当前QRS波的类型
			wCurSortQrs.nQrsType = 100;
			wCurQrs.nQrsType = wCurSortQrs.nQrsType;

			// PVC不匹配数清零
			wLowPvcCont = 0;

			// PVC模板个数加1
			PvcTemp->nTemplateNum++;

			// 用最新的QRS波，更新PVC模板
			for (i = 0; i < QRS_TEMPLATE_LEN; i++)
			{
				PvcTemp->nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
			}
		}
		else
		{
			return 0;
		}
	}

	return 1;
}


//=========================================================================
//分类主函数
//=========================================================================
int QrsClassifyMain(bool reset)
{
	int i = 0, start_pos = 0, end_pos = 0, cur_coef = 0, temp_coef = 0;
	bool update_temp = false;

	if (reset)
	{
		QrsClassifyInitialize();
		return 0;
	}

	// 刚开始检测到的QRS波可能不准确，不进行分类
	if (gQrsPeakNum < 2)
	{
		return 0;
	}

	//=========================================================================
	//1.获取当前需要分类的QRS波
	//=========================================================================
	//1.1 清空wCurSortQrs
	QrsTemplateInitialize(&wCurSortQrs);

	//1.2 获取待分类的QRS波，gQrsPeakBuff中的倒数第二个，这样可以确保QRS波已经完整
	wCurQrs = gQrsPeakBuff[gQrsPeakNum - 2];

	//1.3 存储波峰位置，注意是带通信号中的波峰位置，以及宽度
	// 用带通波峰作为匹配基准点，问题比较大，因为可能正负向是错误的，更改为显示波波峰解决该问题
	//wCurSortQrs.nPeakPos = curQrs.nBpPeakPos;

	// 存储显示QRS的波峰位置，以及QRS宽度
	wCurSortQrs.nPeakPos = mod((wCurQrs.nPeakPos + HIGH_PASS_DELAY + LOW_PASS_DELAY), ECG_DATA_BUFF_LEN);
	wCurSortQrs.nQrsWidth = wCurQrs.nQrsWidth;

	//1.4 计算模板的存储起止位置，一共存贮61个点，峰值点前后各30点
	start_pos = mod((wCurSortQrs.nPeakPos - QRS_TEMPLATE_LEN/2), ECG_DATA_BUFF_LEN);
	end_pos  = mod((wCurSortQrs.nPeakPos + QRS_TEMPLATE_LEN/2), ECG_DATA_BUFF_LEN);

	//1.5 将带通信号存入模板缓存区
	i = 0;
	while (end_pos != start_pos)
	{
		// 赋值
		wCurSortQrs.nQrsTemp[i] = gBpDataBuff[start_pos];

		// 维护指针
		i++;
		start_pos = LoopInc(start_pos, 1, ECG_DATA_BUFF_LEN);
	}

	//=========================================================================
	//2. 计算与主导模板的相关系数
	//=========================================================================
	if (JudgeNormalTemp())
	{
		// 主导模板匹配成功，啥也不做
		NULL;
	}
	else if (PVC_MIN_WIDTH >= wCurSortQrs.nQrsWidth)
	{
		//=========================================================================
		// 不是正常模板，且宽度大于PVC的最小宽度64ms，则归入干扰
		//=========================================================================
		JudgeAbnormalTemp();
	}
	else
	{
		//=========================================================================
		//3. 与主导模板的相关系数小于80，且宽度大于PVC的最小宽度64ms，分为PVC
		//=========================================================================
		if (JudgePvcTemp(&gPvcTemplate))
		{
			gPvcNumAll++;
		}
		else if (JudgePvcTemp(&gPvcTemplateTwo))
		{
			gPvcNumAll ++;
		}
		else if (JudgePvcTemp(&gPvcTemplateThree))
		{
			gPvcNumAll ++;
		}
		else
		{
			wLowPvcCont++;

			// 若出现3个PVC不匹配，则更新数量最少的一个PVC模板
			if (3 <= wLowPvcCont)
			{
				if (gPvcTemplate.nTemplateNum >= gPvcTemplateTwo.nTemplateNum )
				{
					if (gPvcTemplateTwo.nTemplateNum <= gPvcTemplateThree.nTemplateNum)
					{
						gPvcTemplateTwo = wCurSortQrs;
					}
					else
					{
						gPvcTemplateThree = wCurSortQrs;
					}
				}
				else
				{
					if (gPvcTemplate.nTemplateNum <= gPvcTemplateThree.nTemplateNum)
					{
						gPvcTemplate = wCurSortQrs;
					}
					else
					{
						gPvcTemplateThree = wCurSortQrs;
					}
				}

				wLowPvcCont = 0;
			}
		}
	}
	
	//=========================================================================
	//4. 设计主导模板更新机制 （原策略有些问题。首先是相等即替换，这个感觉不对。其次是没有将QRS波本身的状态替换过来）
	//=========================================================================
	// 4.1 如果非正常模板比正常模板的2倍还多，则将非正常模板更新为正常模板，同时更新二者的QRS波状态
	if (wAbnormTemplate.nTemplateNum > gNormTemplate.nTemplateNum * 2)
	{
		// 将“非正常”模板替换为正常模板
		gNormTemplate = wAbnormTemplate;

		for (i = 0; i < gQrsPeakNum; i++)
		{
			if (1 == gQrsPeakBuff[i].nQrsType)
			{
				gQrsPeakBuff[i].nQrsType = 3;
			}
			else if (3 == gQrsPeakBuff[i].nQrsType)
			{
				gQrsPeakBuff[i].nQrsType = 1;
			}
			else
			{
				NULL;
			}
		}
	}

	//=========================================================================
	//5. 最后填充回去
	//=========================================================================
	gQrsPeakBuff[gQrsPeakNum - 2] = wCurQrs;

	return 1;
}


//=========================================================================
//清空模板
//=========================================================================
void QrsTemplateInitialize(QRS_TEMPLATE *qrs_template)
{
	int i = 0;

	for (i = 0; i < QRS_TEMPLATE_LEN; i++)
	{
		qrs_template->nQrsTemp[i] = 0;
	}
	qrs_template->nQrsType = 0;
	qrs_template->nCoefNormal = 0;
	qrs_template->nCoefPvc = 0;

	qrs_template->nStartPos = 0;
	qrs_template->nPeakPos = 0;
	qrs_template->nEndPos   = 0;

	// 下面是QRS总体模板的相关变量初始化
	qrs_template->bIsTemplateExist = false;						//该类型的模板是否存在
	qrs_template->nTemplateNum = 0;								//该类型的QRS波的个数
}


//=========================================================================
//计算相关系数
//=========================================================================
int CalculateCoef(QRS_TEMPLATE *cur_qrs, QRS_TEMPLATE *template_qrs, int offset)
{
	double coef = 0, up_sum = 0, low_sum = 0, x_sum = 0, y_sum = 0;
	int i = 0, x_base = 0, y_base = 0, x_value = 0, y_value = 0, start_pos = 0, end_pos = 0;

	//=========================================================================
	//1.确定计算相关系数的起止位置，获取x、y的基准点幅度
	//=========================================================================
	start_pos = QRS_TEMPLATE_LEN/2 - QRS_TEMP_MATCH_LEN/2;
	end_pos = QRS_TEMPLATE_LEN/2 + QRS_TEMP_MATCH_LEN/2;

	x_base = cur_qrs->nQrsTemp[start_pos];
	y_base = template_qrs->nQrsTemp[start_pos];

	//=========================================================================
	//2.分别计算分子和分母
	//=========================================================================
	//2.1 进行累加计算
	for (i = start_pos; i <= end_pos; i++)
	{
		x_value = cur_qrs->nQrsTemp[i + offset] - x_base;
		y_value = template_qrs->nQrsTemp[i] - y_base;

		up_sum += x_value * y_value;
		x_sum += x_value * x_value;
		y_sum += y_value * y_value;
	}

	//2.2 计算分母
	low_sum = sqrt(x_sum) * sqrt(y_sum);

	//2.3 计算相关系数
	coef = up_sum * 100 / low_sum;

	return coef;
}


// 移位并计算相关系数
int ShiftAndCalcCoef(QRS_TEMPLATE *template_qrs, int cur_coef)
{
	int offset = 0, temp_coef = 0;

	offset = 1;
	temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);
	if (80 <= temp_coef)
	{
		// 匹配度高，直接跳出
		cur_coef = temp_coef;
	}
	else if (temp_coef >= cur_coef)
	{
		// 正向搜索的匹配度逐渐增大，说明方向正确，继续搜索知道下降为止
		// 更新相关系数
		cur_coef = temp_coef;

		// 继续搜索匹配
		for (offset = 2; offset <= 10; offset++)
		{
			temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);

			if (80 <= temp_coef)
			{
				// 匹配度高，直接跳出
				cur_coef = temp_coef;
				break;
			}
			else if (temp_coef >= cur_coef)
			{
				// 仍然增大，继续匹配
				cur_coef = temp_coef;
			}
			else
			{
				// 开始减小，跳出
				break;
			}
		}
	}
	else
	{
		// 至此，说明正向搜索相关系数减少，进行反向搜索
		offset = -1;
		temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);
		if (90 <= temp_coef)
		{
			// 匹配度高，直接跳出
			cur_coef = temp_coef;
		}
		else if (temp_coef >= cur_coef)
		{
			// 正向搜索的匹配度逐渐增大，说明方向正确，继续搜索知道下降为止
			// 更新相关系数
			cur_coef = temp_coef;

			// 继续搜索匹配
			for (offset = -2; offset >= -10; offset--)
			{
				temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);

				if (80 <= temp_coef)
				{
					// 匹配度高，直接跳出
					cur_coef = temp_coef;
					break;
				}
				else if (temp_coef >= cur_coef)
				{
					// 仍然增大，继续匹配
					cur_coef = temp_coef;
				}
				else
				{
					// 开始减小，跳出
					break;
				}
			}
		}
		else
		{
			// 至此，说明正向、反向都比中间点的相关系数小，直接跳出
		}
	}

	return cur_coef;
}

