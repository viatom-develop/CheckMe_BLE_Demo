//*************************************************************************
//�ļ����ƣ�EcgQrsClassify.cpp
//
//�ļ�˵����QRS�����ຯ��
//
//������ʷ��Created by Chenmaomao [9 / 8 / 2014]
//*************************************************************************

//#include "stdafx.h"
#include <math.h>
#include "EcgQrsClassify.h"


//=========================================================================
//����QRS������ص��ڲ�����
//=========================================================================
QRS_TEMPLATE wCurSortQrs;					//��ǰ�������QRS������EcgQrsDetect�е�BandPeakDetect������ȡ
QRS_TEMPLATE gNormTemplate;
QRS_TEMPLATE gPvcTemplate;
QRS_TEMPLATE gPvcTemplateTwo;
QRS_TEMPLATE gPvcTemplateThree;
QRS_TEMPLATE wAbnormTemplate;

QRS_PEAK wCurQrs;

unsigned int wLowCoefCont;					//������ģ������ϵ������80С��90���������ֵĸ���
unsigned int wLowPvcCont;						//������ģ������ϵ������80С��90���������ֵĸ���
unsigned int gPvcNumAll;							// PVC�ĸ���
unsigned int gPvcNumAllLast;					// ǰ�漸���ӵ�PVC��������Ҫ���������㵱ǰһ���ӵ�PVC

//=========================================================================
//�����ʼ��
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

	//2.1 �ж�����ģ���Ƿ����
	if (!gNormTemplate.bIsTemplateExist)
	{
		// ����ģ���в����ڣ��õ�ǰQRS���������ģ�壬Ȼ�󷵻�
		gNormTemplate = wCurSortQrs;
		gNormTemplate.bIsTemplateExist = true;
		return 1;
	}

	//2.2 ���ˣ�˵������ģ����ڣ��������ϵ��
	//		�˴���˼·�ǣ������Զ���Ϊ��׼�������ϵ�����������80���Ͳ�ƫ�Ʊض��ˡ�
	//		���С��80��������ƫ��11���㣬ȡ���ֵ
	//		�ȿ��Լ�����������Ҳ���Ը�����Ϊ������ƫ����ɵ����ϵ�����͵�����
	cur_coef = CalculateCoef(&wCurSortQrs, &gNormTemplate, 0);

	if (85 > cur_coef)
	{
		// ������λ���������������ϵ��
		cur_coef = ShiftAndCalcCoef(&gNormTemplate, cur_coef);
	}
	wCurQrs.nCoefNormal = cur_coef;

	//2.3 �ж��Ƿ���������ģ��
	if (85 <= cur_coef)
	{
		// ������ģ������ϵ��80<coef<90��˵����̬��̫�������Ϊ��������������ģ��
		// ���õ�ǰQRS��������
		wCurSortQrs.nQrsType = 1;
		wCurQrs.nQrsType = wCurSortQrs.nQrsType;

		// ����ģ�������1
		gNormTemplate.nTemplateNum++;

		if (90 <= cur_coef)
		{
			// ����ص�QRS����������
			wLowCoefCont = 0;

			// �����µ�QRS�����������ģ��
			for (i = 0; i < QRS_TEMPLATE_LEN; i++)
			{
				gNormTemplate.nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
			}
		}
		else
		{
			// ����ص�QRS����������
			wLowCoefCont ++;

			// �����������3������ص�QRS����˵��Ŀǰ������ģ�岻̫�ԣ��õ�ǰQRS�����и���
			if (3 <= wLowCoefCont)
			{
				// �����µ�QRS�����������ģ��
				for (i = 0; i < QRS_TEMPLATE_LEN; i++)
				{
					gNormTemplate.nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
				}

				// ����ؼ���������
				wLowCoefCont = 0;
			}
		}
	}
	else
	{
		// ������ģ��û��ƥ��ɹ�������0�����������ƥ��
		return 0;
	}

	return 1;
}


int JudgeAbnormalTemp()
{
	int cur_coef = 0, i = 0;

	if (!wAbnormTemplate.bIsTemplateExist)
	{
		//3.1 "������"ģ�岻���ڣ����
		wAbnormTemplate = wCurSortQrs;
		wAbnormTemplate.bIsTemplateExist = true;
	}
	else
	{
		//3.2 "������"ģ����ڣ��������ϵ��
		cur_coef = CalculateCoef(&wCurSortQrs, &wAbnormTemplate, 0);

		if (80 > cur_coef)
		{
			// ������λ���������������ϵ��
			cur_coef = ShiftAndCalcCoef(&wAbnormTemplate, cur_coef);
		}
		wCurQrs.nCoefNormal = cur_coef + 100;

		//3.3 �������ϵ���ж��Ƿ�ƥ���ģ��
		if (80 <= cur_coef)
		{
			// ��ضȸߣ��ж�ΪAbnormal������Abnormalģ��
			// ���õ�ǰQRS��������
			wCurSortQrs.nQrsType = 3;
			wCurQrs.nQrsType = wCurSortQrs.nQrsType;

			// ����ģ�������1
			wAbnormTemplate.nTemplateNum++;

			// �����µ�QRS���������쳣ģ��
			for (i = 0; i < QRS_TEMPLATE_LEN; i++)
			{
				wAbnormTemplate.nQrsTemp[i] = wCurSortQrs.nQrsTemp[i];
			}
		}
		else
		{
			// ���ˣ�˵��������ģ�岻ƥ�䣬�롰��������ģ��ҳ��ƥ�䣬ֱ�Ӹ��¡���������ģ��
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
		//3.1 "������"ģ�岻���ڣ����
		*PvcTemp = wCurSortQrs;
		PvcTemp->bIsTemplateExist = true;
		return 1;
	}
	else
	{
		//3.2 "������"ģ����ڣ��������ϵ��
		cur_coef = CalculateCoef(&wCurSortQrs, PvcTemp, 0);

		if (80 > cur_coef)
		{
			// ������λ���������������ϵ��
			cur_coef = ShiftAndCalcCoef(PvcTemp, cur_coef);
		}
		wCurQrs.nCoefNormal = cur_coef + 200;

		//3.3 �������ϵ���ж��Ƿ�ƥ���ģ��
		if (80 <= cur_coef)
		{
			// ��ضȸߣ��ж�ΪPVC������PVCģ��
			// ���õ�ǰQRS��������
			wCurSortQrs.nQrsType = 100;
			wCurQrs.nQrsType = wCurSortQrs.nQrsType;

			// PVC��ƥ��������
			wLowPvcCont = 0;

			// PVCģ�������1
			PvcTemp->nTemplateNum++;

			// �����µ�QRS��������PVCģ��
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
//����������
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

	// �տ�ʼ��⵽��QRS�����ܲ�׼ȷ�������з���
	if (gQrsPeakNum < 2)
	{
		return 0;
	}

	//=========================================================================
	//1.��ȡ��ǰ��Ҫ�����QRS��
	//=========================================================================
	//1.1 ���wCurSortQrs
	QrsTemplateInitialize(&wCurSortQrs);

	//1.2 ��ȡ�������QRS����gQrsPeakBuff�еĵ����ڶ�������������ȷ��QRS���Ѿ�����
	wCurQrs = gQrsPeakBuff[gQrsPeakNum - 2];

	//1.3 �洢����λ�ã�ע���Ǵ�ͨ�ź��еĲ���λ�ã��Լ����
	// �ô�ͨ������Ϊƥ���׼�㣬����Ƚϴ���Ϊ�����������Ǵ���ģ�����Ϊ��ʾ��������������
	//wCurSortQrs.nPeakPos = curQrs.nBpPeakPos;

	// �洢��ʾQRS�Ĳ���λ�ã��Լ�QRS���
	wCurSortQrs.nPeakPos = mod((wCurQrs.nPeakPos + HIGH_PASS_DELAY + LOW_PASS_DELAY), ECG_DATA_BUFF_LEN);
	wCurSortQrs.nQrsWidth = wCurQrs.nQrsWidth;

	//1.4 ����ģ��Ĵ洢��ֹλ�ã�һ������61���㣬��ֵ��ǰ���30��
	start_pos = mod((wCurSortQrs.nPeakPos - QRS_TEMPLATE_LEN/2), ECG_DATA_BUFF_LEN);
	end_pos  = mod((wCurSortQrs.nPeakPos + QRS_TEMPLATE_LEN/2), ECG_DATA_BUFF_LEN);

	//1.5 ����ͨ�źŴ���ģ�建����
	i = 0;
	while (end_pos != start_pos)
	{
		// ��ֵ
		wCurSortQrs.nQrsTemp[i] = gBpDataBuff[start_pos];

		// ά��ָ��
		i++;
		start_pos = LoopInc(start_pos, 1, ECG_DATA_BUFF_LEN);
	}

	//=========================================================================
	//2. ����������ģ������ϵ��
	//=========================================================================
	if (JudgeNormalTemp())
	{
		// ����ģ��ƥ��ɹ���ɶҲ����
		NULL;
	}
	else if (PVC_MIN_WIDTH >= wCurSortQrs.nQrsWidth)
	{
		//=========================================================================
		// ��������ģ�壬�ҿ�ȴ���PVC����С���64ms����������
		//=========================================================================
		JudgeAbnormalTemp();
	}
	else
	{
		//=========================================================================
		//3. ������ģ������ϵ��С��80���ҿ�ȴ���PVC����С���64ms����ΪPVC
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

			// ������3��PVC��ƥ�䣬������������ٵ�һ��PVCģ��
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
	//4. �������ģ����»��� ��ԭ������Щ���⡣��������ȼ��滻������о����ԡ������û�н�QRS�������״̬�滻������
	//=========================================================================
	// 4.1 ���������ģ�������ģ���2�����࣬�򽫷�����ģ�����Ϊ����ģ�壬ͬʱ���¶��ߵ�QRS��״̬
	if (wAbnormTemplate.nTemplateNum > gNormTemplate.nTemplateNum * 2)
	{
		// ������������ģ���滻Ϊ����ģ��
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
	//5. �������ȥ
	//=========================================================================
	gQrsPeakBuff[gQrsPeakNum - 2] = wCurQrs;

	return 1;
}


//=========================================================================
//���ģ��
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

	// ������QRS����ģ�����ر�����ʼ��
	qrs_template->bIsTemplateExist = false;						//�����͵�ģ���Ƿ����
	qrs_template->nTemplateNum = 0;								//�����͵�QRS���ĸ���
}


//=========================================================================
//�������ϵ��
//=========================================================================
int CalculateCoef(QRS_TEMPLATE *cur_qrs, QRS_TEMPLATE *template_qrs, int offset)
{
	double coef = 0, up_sum = 0, low_sum = 0, x_sum = 0, y_sum = 0;
	int i = 0, x_base = 0, y_base = 0, x_value = 0, y_value = 0, start_pos = 0, end_pos = 0;

	//=========================================================================
	//1.ȷ���������ϵ������ֹλ�ã���ȡx��y�Ļ�׼�����
	//=========================================================================
	start_pos = QRS_TEMPLATE_LEN/2 - QRS_TEMP_MATCH_LEN/2;
	end_pos = QRS_TEMPLATE_LEN/2 + QRS_TEMP_MATCH_LEN/2;

	x_base = cur_qrs->nQrsTemp[start_pos];
	y_base = template_qrs->nQrsTemp[start_pos];

	//=========================================================================
	//2.�ֱ������Ӻͷ�ĸ
	//=========================================================================
	//2.1 �����ۼӼ���
	for (i = start_pos; i <= end_pos; i++)
	{
		x_value = cur_qrs->nQrsTemp[i + offset] - x_base;
		y_value = template_qrs->nQrsTemp[i] - y_base;

		up_sum += x_value * y_value;
		x_sum += x_value * x_value;
		y_sum += y_value * y_value;
	}

	//2.2 �����ĸ
	low_sum = sqrt(x_sum) * sqrt(y_sum);

	//2.3 �������ϵ��
	coef = up_sum * 100 / low_sum;

	return coef;
}


// ��λ���������ϵ��
int ShiftAndCalcCoef(QRS_TEMPLATE *template_qrs, int cur_coef)
{
	int offset = 0, temp_coef = 0;

	offset = 1;
	temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);
	if (80 <= temp_coef)
	{
		// ƥ��ȸߣ�ֱ������
		cur_coef = temp_coef;
	}
	else if (temp_coef >= cur_coef)
	{
		// ����������ƥ���������˵��������ȷ����������֪���½�Ϊֹ
		// �������ϵ��
		cur_coef = temp_coef;

		// ��������ƥ��
		for (offset = 2; offset <= 10; offset++)
		{
			temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);

			if (80 <= temp_coef)
			{
				// ƥ��ȸߣ�ֱ������
				cur_coef = temp_coef;
				break;
			}
			else if (temp_coef >= cur_coef)
			{
				// ��Ȼ���󣬼���ƥ��
				cur_coef = temp_coef;
			}
			else
			{
				// ��ʼ��С������
				break;
			}
		}
	}
	else
	{
		// ���ˣ�˵�������������ϵ�����٣����з�������
		offset = -1;
		temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);
		if (90 <= temp_coef)
		{
			// ƥ��ȸߣ�ֱ������
			cur_coef = temp_coef;
		}
		else if (temp_coef >= cur_coef)
		{
			// ����������ƥ���������˵��������ȷ����������֪���½�Ϊֹ
			// �������ϵ��
			cur_coef = temp_coef;

			// ��������ƥ��
			for (offset = -2; offset >= -10; offset--)
			{
				temp_coef = CalculateCoef(&wCurSortQrs, template_qrs, offset);

				if (80 <= temp_coef)
				{
					// ƥ��ȸߣ�ֱ������
					cur_coef = temp_coef;
					break;
				}
				else if (temp_coef >= cur_coef)
				{
					// ��Ȼ���󣬼���ƥ��
					cur_coef = temp_coef;
				}
				else
				{
					// ��ʼ��С������
					break;
				}
			}
		}
		else
		{
			// ���ˣ�˵�����򡢷��򶼱��м������ϵ��С��ֱ������
		}
	}

	return cur_coef;
}

