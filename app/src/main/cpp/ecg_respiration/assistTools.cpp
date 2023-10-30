//#include "stdafx.h"
#include "assistTools.h"



//calculate the minimum of the input array
int 	min( const int array_data[], const int array_size )
{
	int 	index 		= 0;
	int 	minimum 	= 0;
	
	minimum 	= array_data[0];
	for( index=0; index<array_size; index++ ){
		if( array_data[index] < minimum ){
			minimum 	= array_data[index];
		}
	}
	
	return minimum;
}

//calculate the minimum of the input array
float 	min_float( const float array_data[], const int array_size )
{
	int 	index 		= 0;
	float 	minimum 	= 0;
	
	minimum 	= array_data[0];
	for( index=0; index<array_size; index++ ){
		if( array_data[index] < minimum ){
			minimum 	= array_data[index];
		}
	}
	
	return minimum;
}

//calculate the maximum of the input array
int 	max( const int array_data[], const int array_size )
{
	int 	index 		= 0;
	int 	maximum 	= 0;
	
	maximum 	= array_data[0];
	for( index=0; index<array_size; index++ ){
		if( array_data[index] > maximum ){
			maximum 	= array_data[index];
		}
	}
	
	return maximum;
}

//calculate the maximum of the input array
float 	max_float( const float array_data[], const int array_size )
{
	int 	index 		= 0;
	float 	maximum 	= 0;
	
	maximum 	= array_data[0];
	for( index=0; index<array_size; index++ ){
		if( array_data[index] > maximum ){
			maximum 	= array_data[index];
		}
	}
	
	return maximum;
}


//计算总平均值
float 	mean( const int array_data[], const int array_size )
{
	int 	index		= 0;
	float 	sum 		= 0;
	float 	mean_value 	= 0;
	
	for( index=0; index<array_size; index++ ){
		sum 	+= array_data[index];
	}
	mean_value 			= sum / array_size;
	
	return mean_value;
}

//根据数组内的所有非零值来计算总平均值
float 	mean_float( const float array_data[], const int array_size )
{
	int 	index		= 0;
	float 	sum 		= 0;
	float 	mean_value 	= 0;
	int		real_size	= 0;
	
	for( index=0; index<array_size; index++ ){
		if( array_data[index] != 0 ){
			real_size ++;
			sum 	+= array_data[index];
		}
	}
	mean_value 			= sum / real_size;
	
	return mean_value;
}	

//calculate the mean value
short 	mean_short( const short array_data[], const short array_size )
{
	int 	index		= 0;
	float 	sum 		= 0;
	short 	mean_value 	= 0;
	short	number 		= 0;
	
	for( index=0; index<array_size; index++ ){
		if( array_data[index] != 0 ){
			sum 	+= array_data[index];
			number++;
		}
	}
	mean_value 			= sum / number + 0.5;
	
	return mean_value;
}

//calculate the standard deviation
float 	std1( const int array_data[], const int array_size )
{
	int 	index		= 0;
	float 	sum 		= 0;
	float 	square_sum 	= 0;
	float 	std_value 	= 0;
	
	for( index=0; index<array_size; index++ ){
		sum 	+= array_data[index] / array_size;
		square_sum 	+= array_data[index] * array_data[index];
	}
	std_value 	= (square_sum - (sum*sum)/array_size) / (array_size-1);
	
	return std_value;
}


//absolute value
float 	abs_float_resp( float value )
{
	float 	abs_value 	= 0;
	
	abs_value 	= value >= 0 ? value : -value;
	return 	abs_value;
}