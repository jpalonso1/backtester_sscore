//Juan Pablo Alonso
#ifndef S_MOMENTUM_H_
#define S_MOMENTUM_H_

#include "setup.h"
#include "gpu.h"

namespace bt{

//gets the index for the maximum and minimum values in the returns array
__device__ __host__
void updateMaxMin(float *avgReturns, int& max, int& min){
	max = 0; min = 0;
	float maxval = avgReturns[0];
	float minval = avgReturns[0];

	for(int i=1; i<DATA_ELEMENTS; i++){
		if(maxval<avgReturns[i]){
			max = i;
			maxval = avgReturns[i];
		}
		if(minval>avgReturns[i]){
			min = i;
			minval = avgReturns[i];
		}
	}
}


//runs the momentum strategy for the day
__device__ __host__
void momentum(bt::stockData * data,bt::execution& exec, bool& isMomentumOpen, int windowsize,
		int endpos, int& daysopen, int& min,int &max,const bt::parameters& par,
		int &lastMax, int &lastMin){
	if(!isMomentumOpen){ //if there isn't already an open position
		//get average return over the window for each stock
		float avgReturn[DATA_ELEMENTS];
		for(int sym=0; sym<DATA_ELEMENTS; sym++)
		{
			float sumReturn=0;
			for (int k=endpos-windowsize; k<endpos; k++){
				sumReturn+=log(data[k+1].d[sym]/data[k].d[sym]);
			}
			avgReturn[sym]=sumReturn/windowsize;
		}

		updateMaxMin(avgReturn,max,min);

		//check that the max return is positive, min return is negative
		if((avgReturn[max]>0)&&(avgReturn[min]<0)){
			//long max stock, short min stock
			recordTrade(data,exec,max,endpos,par.lPar[bt::orderSize]);
			recordTrade(data,exec,min,endpos,-par.lPar[bt::orderSize]);
			lastMin=min;lastMax=max;
			isMomentumOpen = true;
		}
	}
	else{
		if(daysopen<20) //increment count if position has been open for less than a month
			daysopen++;
		else{ //close position if open for a month
			//close position
//			forceClose();
			closePosition(data,exec,lastMax,endpos);
			closePosition(data,exec,lastMin,endpos);
			daysopen = 0;
			isMomentumOpen = false;
		}
	}
}

//Loops through the momentum strategy over the whole time series
__device__ __host__
void executeMomentum(bt::stockData * data,bt::execution& exec,
		long dataSize,const bt::parameters& par){

	int windowsize = 6*20; //days of history to look back at
	int daysopen = 0; //initialize variable to count days a position is open
	bool isMomentumOpen = false; //initialize boolean to signal whether a momentum position is open

	//find the index for the stock with the min and max return
	int min,max;
	int lastMin,lastMax;
	//replace 100 with the # of observations
	for(int endpos=windowsize+1; endpos<dataSize; endpos++)	{
		momentum(data,exec, isMomentumOpen, windowsize, endpos, daysopen,min,max,par,lastMin,lastMax);
	}
}

//namespace bt
}
#endif /* S_MOMENTUM_H_ */
