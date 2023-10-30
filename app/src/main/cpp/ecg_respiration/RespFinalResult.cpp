//#include "stdafx.h"
#include "RespFinalResult.h"


//����ȫ�����ݣ�peaks, RR, position
extern 	int 	Peak_Array[QRS_LEN];
extern 	int 	RR_Array[QRS_LEN];
extern 	int 	pos_Array[QRS_LEN];
extern 	unsigned short 	real_qrs_len;

//statistical elements
float 	peak_mean 	= 0;
float 	RR_mean 	= 0;
float 	peak_var 	= 0; 	//����
float 	RR_var 		= 0;

int 	peak_min 	= 0;
int 	peak_max 	= 0;
int 	RR_min 		= 0;
int 	RR_max 		= 0;
float 	peak_normalized_std 	= 0;	//standard deviation
float 	RR_normalized_std 		= 0;
	
//merge the peak and RR array by the scales
float 	peak_scale 			= 0;
float 	RR_scale 			= 0;
float 	merge_peak_RR[QRS_LEN];
float 	merge_mean 			= 0;		//mean line
float 	merge_diff[QRS_LEN-1]; 	//differencial value between neighbors
float 	merge_diff_max 		= 0;
float	duration_diff[QRS_LEN-1];
float	duration_diff_max 	= 0;
float	duration_diff_min 	= 0;

//1D and 2D merge array
float 	merge_1d_height[QRS_LEN];
float 	abs_merge_1d_height[QRS_LEN];
float 	merge_1d_duration[QRS_LEN];
float 	merge_2d_height[QRS_LEN];
float 	merge_2d_duration[QRS_LEN];
unsigned char	merge_1d_len;		//��һ���ںϺ��ʵ�ʳ���
unsigned char 	merge_2d_len;		//�ڶ����ںϺ��ʵ�ʳ���
unsigned char	merge_times;		//��ǵڼ����ں�

//�����ж��Ƿ���еڶ����ںϵ���ʱ����
float	pre_height,		pre_duration;
float	cur_height,		cur_duration;
float	after_height,	after_duration;
float					merge_duration;

//ʱ���ȼ����ر���
unsigned char	split_number;	//���������б��������ɸ߶��޷���⣩�ָ��˵ĺ�����������
unsigned char	merge_number;	//�����������������������������ں���һ������������


//calculate the number of cycles of respiration within a interval
float calRespiration( void )
/*
in_peaks: peaks array from ECG; 		in_RR: RR interval array from ECG;
in_array_size: the size of the array in_peaks and in_RR;
*/
{
	short 	index 				= 0;
	float 	peak_sum 			= 0;
	float 	peak_square_sum 	= 0;
	float 	RR_sum 				= 0;
	float 	RR_square_sum 		= 0;
	float 	temp, 	temp1;
	//���
	float	respiration_rate; 	//������
	
	peak_min 	= min( Peak_Array, real_qrs_len );
	peak_max 	= max( Peak_Array, real_qrs_len );
	
	RR_min 		= min( RR_Array, real_qrs_len );
	RR_max 		= max( RR_Array, real_qrs_len );
	
	//calculate the SNR( Signal Noise Rate ) = mean / std
	//var = (square_sum-sum^2/N)/(N-1); std = sqrt( var )
	for( index=0; index<real_qrs_len; index++ ){
		temp 				= (float)(Peak_Array[index]-peak_min)/(peak_max-peak_min);
		peak_sum 			+= temp;
		peak_square_sum 	+= temp * temp;
		
		temp 				= (float)(RR_Array[index]-RR_min)/(RR_max-RR_min);
		RR_sum 				+= temp;
		RR_square_sum 		+= temp * temp;
	}
	peak_mean 	= peak_sum / real_qrs_len;
	peak_var 	= (peak_square_sum - peak_sum*peak_sum/real_qrs_len)/(real_qrs_len-1);
	peak_normalized_std 	= sqrt( peak_var );
	RR_mean 	= RR_sum / real_qrs_len;
	RR_var 	= (RR_square_sum - RR_sum*RR_sum/real_qrs_len)/(real_qrs_len-1);
	RR_normalized_std 		= sqrt( RR_var );
	
	//evaluate the peak and RR merge scale
	RR_scale 	= peak_normalized_std / (RR_normalized_std + peak_normalized_std);
	peak_scale 	= 1 - RR_scale;
	//Adjust the scale by this principle that the RR is more standy than peaks
	if( peak_scale > RR_scale ){
		//Adjust the scale within the limit (0.35, 0.65)
		if( peak_scale > 0.35 ) 		peak_scale 	= 0.35;
		if( RR_scale < 0.65 ) 		RR_scale 	= 0.65;
	}
	else{
		//Adjust the scale within the limit(0.35, 0.65)
		if( RR_scale > 0.8 ) 		RR_scale	= 0.8;
		if( peak_scale < 0.2 ) 	peak_scale 	= 0.2;
	}
	
	//merge the peak and RR array 
	for( index=0; index<real_qrs_len; index++ ){
		temp 				= (float)(Peak_Array[index]-peak_min)/(peak_max-peak_min);
		merge_peak_RR[index] 	= peak_scale*temp;
		temp 				= (float)(RR_Array[index]-RR_min)/(RR_max-RR_min);
		merge_peak_RR[index] 	+= RR_scale*temp;
	}
	
	//calculate mean line for cross point
	merge_mean 	= mean_float( merge_peak_RR, real_qrs_len );
	
	//diff absolute array present the availability of one segment
	merge_diff_max 	= 0;
	for( index=0; index<real_qrs_len-1; index++ ){
		merge_diff[index] 	= merge_peak_RR[index+1] - merge_peak_RR[index];
		if( abs_float_resp(merge_diff[index]) > merge_diff_max )
			merge_diff_max 	= abs_float_resp(merge_diff[index]);
	}
	for( index=0; index<real_qrs_len-1; index++ ){
		merge_diff[index] 	= merge_diff[index] / merge_diff_max;
	}
	
	//��ÿ�����εĿ��ʱ��(����ֱ����RR���ƹ���)
	for( index=1; index<real_qrs_len; index++){
		duration_diff[index-1] 	= (float)RR_Array[index] / FS; 	//�Ѳ�������ת��Ϊ��
	}
	
	//���е�һ�������ںϣ���ͬ�仯�����������ӣ����õ�һ��������������
	merge_times 	= 1;
	merge_1d_len 	= merge( merge_diff, duration_diff, real_qrs_len-1, 
							merge_1d_height, merge_1d_duration );
	
	//**�Ų�����С������
	//�������֣�С����λ�ں��������������½���
	//�������֣�С����λ�ں������Ĳ���򲨹ȣ���ǰ�����ڲ��岨�ȵ�����
	//����ʽ��ֱ�Ӷ�����С����

	//************************************************************************************
	//ģ�ͣ���ǰ����B�� ǰ����A���󲨶�C
	//��A�ķ��ȶ�û�г�Խ��B��C�ķ��ȵ�0.55��������ΪС����λ���������½��ε��в���
	//��A�ķ���û�г�ԽB�ķ��ȵ�0.55�������ǳ�Խ��C�η��ȵ�0.55��������ΪС����λ�ڲ���򲨹ȣ�
	//************************************************************************************


//******************************************************************************************
//*************************************�ɲ��εķ������ж��Ƿ�����ڶ����ں�*******************
	duration_diff_max 	= max_float( merge_1d_duration, real_qrs_len-1 );
	duration_diff_min 	= min_float( merge_1d_duration, real_qrs_len-1 );
	if( duration_diff_max < 0.5 ) 	duration_diff_max	= 0.6;	//����������С���Ϊ1��
	//��ʱRR���Ϊ0.25���Ӧ�������250Hz
	if( duration_diff_min < 0.25 ) 	duration_diff_min 	= 0.25;
	//һ������£����ε����ֵ����Сֵ��2.5������
	if( duration_diff_max < 2.5*duration_diff_min )
		duration_diff_max 	= 3*duration_diff_min;
	if( duration_diff_max > 4.5 ) duration_diff_max	= 4.5; 	//�������������Ϊ7��
	
	//��ȡ����һ���ںϺ�ľ���ֵ����
	for( index = 1; index < merge_1d_len; index++ ){
		abs_merge_1d_height[index] 	= abs_float_resp( merge_1d_height[index] );
	}
	
	for( index = 1; index < merge_1d_len-1; index++ ){
		//����ʱ������ֵ
		pre_height 		= abs_merge_1d_height[index-1];
		pre_duration 	= merge_1d_duration[index-1];
		cur_height		= abs_merge_1d_height[index];
		cur_duration	= merge_1d_duration[index];
		after_height	= abs_merge_1d_height[index+1];
		after_duration	= merge_1d_duration[index+1];
		merge_duration 	= pre_duration + cur_duration + after_duration;
		//��һ��С������ǰ����С����������ǰ�߶ȱ�ǰ��߶ȶ�����С
		//����С�Ķ����ǵ�ǰ������Ӧ��û�г���ǰ����ȵ�0.55��
		temp 	= pre_height > cur_height ? pre_height : cur_height;
		temp1 	= after_height > cur_height ? after_height : cur_height;
		if( pre_height - cur_height > 0.45 * temp &&
			after_height - cur_height > 0.45 * temp1 ){
			//Ϊ��ֹ�����ڴ�Ư�Ʋ����ĺ󲨱�ǰ������̧�������������
			//����������֮��Ĺ��Ȳ�������С�������ںϣ�����ʱ��������Ϊ�ο��Ų�
			if( merge_duration - duration_diff_max < 0.35 * duration_diff_max &&
				 merge_duration < 6.5 ){
					merge_1d_height[index] 	= 0; 	//���Ϊ0��������ǰ���ں�
					 // �������ֵ
//					 if( merge_duration > duration_diff_max )
//						 duration_diff_max 	= merge_duration;
			}
		}
		
		//�ڶ���С������������С����������ǰ��С��ǰ����һ�룬�����С���
		//���Ķ����ǵ�ǰ���½����������ķ���Ҫ�������߷��ȵ�0.55��
		if( pre_height - cur_height > 0.45 * temp &&
			cur_height - after_height <= 0.45 * temp1 ){
			//Ϊ��ֹ�����ڴ�Ư�Ʋ����ĺ󲨱�ǰ�������½������������
			//����������֮��Ĺ��Ȳ�������С�������ںϣ�����ʱ��������Ϊ�ο��Ų�
			if( merge_duration - duration_diff_max < 0.35 * duration_diff_max &&
				merge_duration < 6.5 ){
					merge_1d_height[index] 	= 0; 	//���Ϊ0��������ǰ���ں�
					// �������ֵ
//					if( merge_duration > duration_diff_max )
//						duration_diff_max 	= merge_duration;
			}	
		}
	}
	
	 //**�����ںϣ��Ų�ֲ�С����(�ѱ��Ϊ0�������������ں�����
	merge_times 	= 2;
	merge_2d_len 	= merge( merge_1d_height, merge_1d_duration, merge_1d_len,
			merge_2d_height, merge_2d_duration );

//******************************************************************************************
//**************************�ɲ��ε�ʱ�������ж��ںϲ���************************************
	//*�ɲ��ε�ʱ�������ж��ںϲ��Σ����һЩʱ���ϵ��쳣��
	//�����Ѿ������˷��ȵ��ж�ɸѡ������ֻ����һ�ִ��������һ����������Ϊ������������
	//������ʱ����ʵ��ÿ����������ʱ���ȱ������ļ��룬�õ��ĸ�������С��ȵ��������½�����
	duration_diff_max 	= max_float( merge_2d_duration, merge_2d_len );
	duration_diff_min 	= min_float( merge_2d_duration, merge_2d_len );
	if( duration_diff_max > 4.5 ) 	duration_diff_max 	= 4.5;	//�������������Ϊ7��
	if( duration_diff_max < 0.5 ) 	duration_diff_max 	= 0.6; 	//����������С���Ϊ1��
	if( duration_diff_min < 0.25 ) 	duration_diff_min	= 0.25;
	
	split_number 	= 0; 	//��¼���������б��ָ��˵ĺ�����������
	merge_number	= 0;
	for( index=1; index<merge_2d_len-4-1; index++ ){
		//����4�������Ĺ�С�������������ΪӦ���ں�
		if( merge_2d_duration[index] 	< 0.5 * duration_diff_max &&
			merge_2d_duration[index+1] 	< 0.5 * duration_diff_max &&
			merge_2d_duration[index+2] 	< 0.5 * duration_diff_max &&
			merge_2d_duration[index+3] 	< 0.5 * duration_diff_max ){
			split_number ++;
			index 	+= 3; 	//������4������ѭ�����Լ�1���˴�ֻ��3
		}
	}
	//������һ���������Ĳ���������Ϊ���������������Ӧ�÷ָ�
	for( index=1; index<merge_2d_len-1; index++ ){
		if( merge_2d_duration[index] - duration_diff_max > duration_diff_min )
			merge_number ++;
	}
	
//******************************************************************************************
//**************************����������ڲ��ó�������ֵ**************************************
	//�ɺ��������������������
	//���鲨����β�߶ȣ�̫С�����ں�����������
	temp1 	= merge_2d_duration[0] + merge_2d_duration[merge_2d_len-1];
	temp1 	= temp1 / duration_diff_max;
	
	//����ÿһ�����������ָ�Ļ�����Ҫ��2������
	respiration_rate 	= 0.5 * ( merge_2d_len - 2 - 
									2*split_number + 2*merge_number + temp1 );
	respiration_rate 	= 15000*respiration_rate / (pos_Array[real_qrs_len-1] - pos_Array[0]);
	
	//dubug
	if( respiration_rate > 27 ){
		int 	a 	= 0;
	}
	return 	respiration_rate;
}


//�����ںϺ���
unsigned char 	merge( float *height, float *duration, int length,
						float *merge_height, float *merge_duration )
{
	unsigned char 	direction_sign 	= 0; 	//�����־λ��1-���ϣ�0-����
	float			height_sum 		= 0;
	float			duration_sum	= 0;
	unsigned 		char	i		= 0;
	unsigned 		char	j		= 0;
	
	for( i=0; i<length; i++ ){
		//�ڵڶ����ں�ʱ������0���
        if(height[i] == 0 && merge_times == 2)
            continue;
		
		//�õ�һ������������ʼ��
		if( i == 0 ){
			if( height[i] > 0 ){
				direction_sign 	= 1;
				height_sum 		= height[i];
				duration_sum	= duration[i];
				continue;
			}
			else{
				direction_sign 	= 0;
				height_sum 		= height[i];
				duration_sum	= duration[i];
				continue;
			}
		}
		
		//������һ��������ı仯����������ں�
		if( height[i] > 0 ){ 	//����
			//����ת����ʼ��¼�ںϺ������
			if ( direction_sign == 0 ){ 	//ǰһ����������
				merge_height[j] 	= height_sum;
				merge_duration[j] 	= duration_sum;
				//Ϊ��һ����¼��׼��
				height_sum 		= height[i];
				duration_sum	= duration[i];
				direction_sign 	= 1;
				j 				= j + 1;
			}
			else{
				height_sum 		= height_sum 	+ height[i];
				duration_sum 	= duration_sum 	+ duration[i];
			}
		}
		else{ 					//����
			if( direction_sign == 1 ){ 	//ǰһ����������
				merge_height[j] 	= height_sum;
				merge_duration[j] 	= duration_sum;
				//Ϊ��һ����¼��׼��
				height_sum 		= height[i];
				duration_sum 	= duration[i];
				direction_sign 	= 0;
				j 				= j + 1;
			}
			else{
				height_sum 		= height_sum 	+ height[i];
				duration_sum 	= duration_sum 	+ duration[i];
			}
		}
		
		//�������һ��
		if( i == length-1 ){
			merge_height[j] 	= height_sum;
			merge_duration[j] 	= duration_sum;
		}
		
	}
	return 	j+1;	//�����ںϺ�����鳤��
}