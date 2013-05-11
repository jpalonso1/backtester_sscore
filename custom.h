//Juan Pablo Alonso

#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "setup.h"

static char* dataFile={"AAPLclean.csv"};
static std::ofstream testOut("testOut.csv");

namespace bt{

enum {orderSize=0,fastMA=1,slowMA=2};
enum {s1=0,vol=1};
enum {initEq=0};

long setParameters(thrust::host_vector<parameters>& par);

__device__ __host__
inline void recordTrade(bt::stockData* data,bt::execution& exec,
		long sym,long location,long amount){
	exec.trade[sym].posSize[exec.numTrades[sym]]=amount;
	exec.trade[sym].price[exec.numTrades[sym]]=data[location].d[sym];
	exec.trade[sym].location[exec.numTrades[sym]]=location;
	exec.numTrades[sym]++;
}

__device__ __host__
void crossingMA(bt::stockData* data,long dataSize,long sym,
		float orderSize,long fastMA,long slowMA,bt::execution& exec);

////gets the index for the maximum and minimum values in the returns array
//__device__ __host__
//void updateMaxMin(float *avgReturns, int& max, int& min){
//	max = 0; min = 0;
//	float maxval = avgReturns[0];
//	float minval = avgReturns[0];
//
//	for(int i=1; i<DATA_ELEMENTS-1; i++){
//		if(maxval<avgReturns[i]){
//			max = i;
//			maxval = avgReturns[i];
//		}
//		if(minval>avgReturns[i]){
//			min = i;
//			minval = avgReturns[i];
//		}
//	}
//}

////runs the momentum strategy for the day
//__device__ __host__
//void momentum(bt::stockData * data,bt::execution& exec, bool& isMomentumOpen, int windowsize,
//		int endpos, int& daysopen, int& min,int &max){
//	if(!isMomentumOpen){ //if there isn't already an open position
//		//get average return over the window for each stock
//		float avgReturn[DATA_ELEMENTS];
//		for(int sym=0; sym<DATA_ELEMENTS; sym++)
//		{
//			float sumReturn=0;
//			for (int k=endpos-windowsize; k<endpos; k++){
//				sumReturn+=log(data[k+1].d[sym]/data[k].d[sym]);
//			}
//			avgReturn[sym]=sumReturn/windowsize;
//		}
//
//		updateMaxMin(avgReturn,max,min);
//
//		//check that the max return is positive, min return is negative
//		if((avgReturn[max]>0)&&(avgReturn[min]<0)){
//			//long max stock, short min stock
//			recordTrade(data,exec,max,endpos,100);
//			recordTrade(data,exec,min,endpos,-100);
//
//			isMomentumOpen = true;
//		}
//	}
//	else{
//		if(daysopen<20) //increment count if position has been open for less than a month
//			daysopen++;
//		else{ //close position if open for a month
//			//close position
//			recordTrade(data,exec,max,endpos,-100);
//			recordTrade(data,exec,min,endpos,+100);
//			daysopen = 0;
//			isMomentumOpen = false;
//		}
//	}
//}
//
////Loops through the momentum strategy over the whole time series
//__device__ __host__
//void executeMomentum(bt::stockData * data,bt::execution& exec,long dataSize){
//
//	int windowsize = 6*20; //days of history to look back at
//	int daysopen = 0; //initialize variable to count days a position is open
//	bool isMomentumOpen = false; //initialize boolean to signal whether a momentum position is open
//
//	//find the index for the stock with the min and max return
//	int min;
//	int max;
//
//	//replace 100 with the # of observations
//	for(int endpos=windowsize+1; endpos<dataSize; endpos++)
//	{
//
//		momentum(data,exec, isMomentumOpen, windowsize, endpos, daysopen,min,max);
//	}
//}
//
//This function is called every iteration. DO NOT modify function name or arguments
__device__ __host__
inline void runExecution(bt::stockData* data,long dataSize,
		bt::execution& exec,const bt::parameters& par){
	//modify this line to call custom function:
	crossingMA(data,dataSize,0,100.0,par.lPar[bt::fastMA],par.lPar[bt::slowMA],exec);
//	executeMomentum(data,exec,dataSize);
}

//namespace bt
}

#endif /* CUSTOM_H_ */
