#ifndef		_RESP_FINAL_RESULT_H
#define		_RESP_FINAL_RESULT_H

#include <math.h>
#include <stdlib.h>
//#include <stdbool.h>
#include "EcgResult.h"
#include "assistTools.h"
#include "RespMain.h"

//void RespFinalResult(void);
//With the peaks and valleys method, calculate the respiration cycles
void peakValley(int *Peak_Array, short PeakNum);

//With the cross mean line method, calculate the respiration cycles
float calRespiration( void );

//merge the data if they have the same increase or decrease direction
unsigned char  	merge( float *height, float *duration, int length,
						float *merge_height, float *merge_duration );
#endif