#ifndef ASSIST_TOOLS_RESPIRATION
#define ASSIST_TOOLS_RESPIRATION

//calculate the minimum of the input array
int 	min( const int array_data[], const int array_size );

//calculate the minimum of the input array
float 	min_float( const float array_data[], const int array_size );

//calculate the maximum of the input array
int 	max( const int array_data[], const int array_size );

//calculate the maximum of the input array
float 	max_float( const float array_data[], const int array_size );

//calculate the mean value
float 	mean( const int array_data[], const int array_size );

//calculate the mean value
float 	mean_float( const float array_data[], const int array_size );

//calculate the mean value
short 	mean_short( const short array_data[], const short array_size );

//calculate the standard deviation
float 	std1( const int array_data[], const int array_size );

//absolute value
float 	abs_float_resp( float value );

#endif