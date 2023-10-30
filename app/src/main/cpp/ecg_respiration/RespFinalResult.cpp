//#include "stdafx.h"
#include "RespFinalResult.h"


//导入全局数据：peaks, RR, position
extern 	int 	Peak_Array[QRS_LEN];
extern 	int 	RR_Array[QRS_LEN];
extern 	int 	pos_Array[QRS_LEN];
extern 	unsigned short 	real_qrs_len;

//statistical elements
float 	peak_mean 	= 0;
float 	RR_mean 	= 0;
float 	peak_var 	= 0; 	//方差
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
unsigned char	merge_1d_len;		//第一次融合后的实际长度
unsigned char 	merge_2d_len;		//第二次融合后的实际长度
unsigned char	merge_times;		//标记第几次融合

//用于判断是否进行第二次融合的临时变量
float	pre_height,		pre_duration;
float	cur_height,		cur_duration;
float	after_height,	after_duration;
float					merge_duration;

//时间宽度检测相关变量
unsigned char	split_number;	//测量过程中被完美（由高度无法检测）分割了的呼吸波的数量
unsigned char	merge_number;	//测量过程中有两个呼吸波完美地融合在一起的情况的数量


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
	//结果
	float	respiration_rate; 	//呼吸率
	
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
	
	//求每个波段的宽度时间(这里直接由RR复制过来)
	for( index=1; index<real_qrs_len; index++){
		duration_diff[index-1] 	= (float)RR_Array[index] / FS; 	//把采样点数转化为秒
	}
	
	//进行第一次数据融合（相同变化方向的数据相加），得到一个基本波形轮廓
	merge_times 	= 1;
	merge_1d_len 	= merge( merge_diff, duration_diff, real_qrs_len-1, 
							merge_1d_height, merge_1d_duration );
	
	//**排查四种小波动，
	//其中两种：小波动位于呼吸波的上升或下降段
	//其中两种：小波动位于呼吸波的波峰或波谷，是前两类在波峰波谷的特例
	//处理方式：直接丢弃该小波动

	//************************************************************************************
	//模型：当前波段B， 前波段A，后波段C
	//当A的幅度都没有超越了B和C的幅度的0.55倍，则视为小波动位于上升或下降段的中部；
	//当A的幅度没有超越B的幅度的0.55倍，但是超越了C段幅度的0.55倍，则视为小波动位于波峰或波谷；
	//************************************************************************************


//******************************************************************************************
//*************************************由波形的幅度来判断是否继续第二次融合*******************
	duration_diff_max 	= max_float( merge_1d_duration, real_qrs_len-1 );
	duration_diff_min 	= min_float( merge_1d_duration, real_qrs_len-1 );
	if( duration_diff_max < 0.5 ) 	duration_diff_max	= 0.6;	//呼吸波的最小跨幅为1秒
	//此时RR间隔为0.25秒对应最大心率250Hz
	if( duration_diff_min < 0.25 ) 	duration_diff_min 	= 0.25;
	//一般情况下，波段的最大值比最小值大2.5倍以上
	if( duration_diff_max < 2.5*duration_diff_min )
		duration_diff_max 	= 3*duration_diff_min;
	if( duration_diff_max > 4.5 ) duration_diff_max	= 4.5; 	//呼吸波的最大跨幅为7秒
	
	//获取幅度一阶融合后的绝对值数组
	for( index = 1; index < merge_1d_len; index++ ){
		abs_merge_1d_height[index] 	= abs_float_resp( merge_1d_height[index] );
	}
	
	for( index = 1; index < merge_1d_len-1; index++ ){
		//对临时变量赋值
		pre_height 		= abs_merge_1d_height[index-1];
		pre_duration 	= merge_1d_duration[index-1];
		cur_height		= abs_merge_1d_height[index];
		cur_duration	= merge_1d_duration[index];
		after_height	= abs_merge_1d_height[index+1];
		after_duration	= merge_1d_duration[index+1];
		merge_duration 	= pre_duration + cur_duration + after_duration;
		//第一类小波动（前两种小波动）：当前高度比前后高度都尽量小
		//尽量小的定义是当前波幅度应该没有超过前后幅度的0.55倍
		temp 	= pre_height > cur_height ? pre_height : cur_height;
		temp1 	= after_height > cur_height ? after_height : cur_height;
		if( pre_height - cur_height > 0.45 * temp &&
			after_height - cur_height > 0.45 * temp1 ){
			//为防止把由于大漂移产生的后波比前波整体抬升的相似情况下
			//而把两个波之间的过度波（类似小波动）融合，采用时间数据作为参考排查
			if( merge_duration - duration_diff_max < 0.35 * duration_diff_max &&
				 merge_duration < 6.5 ){
					merge_1d_height[index] 	= 0; 	//标记为0，后期与前后融合
					 // 更新最大值
//					 if( merge_duration > duration_diff_max )
//						 duration_diff_max 	= merge_duration;
			}
		}
		
		//第二类小波动（后两种小波动）：当前点小于前面点的一半，与后点大小差不多
		//差不多的定义是当前波下降后在上升的幅度要超过大者幅度的0.55倍
		if( pre_height - cur_height > 0.45 * temp &&
			cur_height - after_height <= 0.45 * temp1 ){
			//为防止把由于大漂移产生的后波比前波整体下降的相似情况下
			//而把两个波之间的过度波（类似小波动）融合，采用时间数据作为参考排查
			if( merge_duration - duration_diff_max < 0.35 * duration_diff_max &&
				merge_duration < 6.5 ){
					merge_1d_height[index] 	= 0; 	//标记为0，后期与前后融合
					// 更新最大值
//					if( merge_duration > duration_diff_max )
//						duration_diff_max 	= merge_duration;
			}	
		}
	}
	
	 //**二阶融合，排查局部小波动(已标记为0），继续整理融合数据
	merge_times 	= 2;
	merge_2d_len 	= merge( merge_1d_height, merge_1d_duration, merge_1d_len,
			merge_2d_height, merge_2d_duration );

//******************************************************************************************
//**************************由波形的时间宽度来判断融合波形************************************
	//*由波形的时间宽度来判断融合波形（检测一些时间上的异常）
	//由于已经经过了幅度的判断筛选，现在只存在一种错误情况：一个呼吸波分为了两个呼吸波
	//这样在时间上实际每个呼吸波的时间跨度比正常的减半，得到四个连续的小跨度的上升或下降波段
	duration_diff_max 	= max_float( merge_2d_duration, merge_2d_len );
	duration_diff_min 	= min_float( merge_2d_duration, merge_2d_len );
	if( duration_diff_max > 4.5 ) 	duration_diff_max 	= 4.5;	//呼吸波的最大跨幅为7秒
	if( duration_diff_max < 0.5 ) 	duration_diff_max 	= 0.6; 	//呼吸波的最小跨幅为1秒
	if( duration_diff_min < 0.25 ) 	duration_diff_min	= 0.25;
	
	split_number 	= 0; 	//记录测量过程中被分割了的呼吸波的数量
	merge_number	= 0;
	for( index=1; index<merge_2d_len-4-1; index++ ){
		//出现4个连续的过小跨幅波动，则认为应该融合
		if( merge_2d_duration[index] 	< 0.5 * duration_diff_max &&
			merge_2d_duration[index+1] 	< 0.5 * duration_diff_max &&
			merge_2d_duration[index+2] 	< 0.5 * duration_diff_max &&
			merge_2d_duration[index+3] 	< 0.5 * duration_diff_max ){
			split_number ++;
			index 	+= 3; 	//本来加4，鉴于循环会自加1，此处只加3
		}
	}
	//出现了一个过大跨幅的波动，则认为其包含两个呼吸波应该分割
	for( index=1; index<merge_2d_len-1; index++ ){
		if( merge_2d_duration[index] - duration_diff_max > duration_diff_min )
			merge_number ++;
	}
	
//******************************************************************************************
//**************************推算呼吸周期并得出呼吸率值**************************************
	//由呼吸波形来计算呼吸周期
	//考查波形首尾高度，太小则不算在呼吸波周期内
	temp1 	= merge_2d_duration[0] + merge_2d_duration[merge_2d_len-1];
	temp1 	= temp1 / duration_diff_max;
	
	//对于每一个呼吸波被分割的话，需要减2来补偿
	respiration_rate 	= 0.5 * ( merge_2d_len - 2 - 
									2*split_number + 2*merge_number + temp1 );
	respiration_rate 	= 15000*respiration_rate / (pos_Array[real_qrs_len-1] - pos_Array[0]);
	
	//dubug
	if( respiration_rate > 27 ){
		int 	a 	= 0;
	}
	return 	respiration_rate;
}


//波段融合函数
unsigned char 	merge( float *height, float *duration, int length,
						float *merge_height, float *merge_duration )
{
	unsigned char 	direction_sign 	= 0; 	//方向标志位：1-向上，0-向下
	float			height_sum 		= 0;
	float			duration_sum	= 0;
	unsigned 		char	i		= 0;
	unsigned 		char	j		= 0;
	
	for( i=0; i<length; i++ ){
		//在第二次融合时，忽略0标记
        if(height[i] == 0 && merge_times == 2)
            continue;
		
		//用第一个数据项来初始化
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
		
		//根据下一个数据项的变化方向情况来融合
		if( height[i] > 0 ){ 	//向上
			//方向反转，开始记录融合后的数据
			if ( direction_sign == 0 ){ 	//前一个方向向下
				merge_height[j] 	= height_sum;
				merge_duration[j] 	= duration_sum;
				//为下一条记录作准备
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
		else{ 					//向下
			if( direction_sign == 1 ){ 	//前一个方向向上
				merge_height[j] 	= height_sum;
				merge_duration[j] 	= duration_sum;
				//为下一条记录作准备
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
		
		//处理最后一项
		if( i == length-1 ){
			merge_height[j] 	= height_sum;
			merge_duration[j] 	= duration_sum;
		}
		
	}
	return 	j+1;	//返回融合后的数组长度
}