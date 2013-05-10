//Juan Pablo Alonso

#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "setup.h"

static char* dataFile={"AAPLclean.csv"};

namespace bt{

enum {orderSize=0,fastMA=1,slowMA=2};
enum {s1=0,vol=1};
enum {initEq=0};

long setParameters(thrust::host_vector<parameters>& par);

__device__ __host__
inline void crossingMA(bt::stockData* data,long dataSize,long sym,
		float orderSize,long fastMA,long slowMA,bt::execution& exec){
	//tracks last direction. fast>slow =1, fast<slow =-1
	int lastCross=0;
	//moving averages of current point
	float slow=0,fast=0;
	float tempSlow=0,tempFast=0;
	//check if trade needs to happen (+1 buy, -1 sell)
	int nextTrade=0;

	for (int i=slowMA-1;i<dataSize;i++){
		tempSlow=0;tempFast=0;
		//get slow MA
		for (int j=0;j<slowMA;j++){
			tempSlow+=data[i-j].d[sym];
		}
		slow=tempSlow/float(slowMA);
		//get fast MA
		for (int j=0;j<fastMA;j++){
			tempFast+=data[i-j].d[sym];
		}
		fast=tempFast/float(fastMA);

		if (fast>slow){
			//crossed up, buy
			if (lastCross==-1)nextTrade=1;
			lastCross=1;
		}

		else if(fast<slow){
			//crossed down, sell
			if (lastCross==1)nextTrade=-1;
			lastCross=-1;
		}

		//record the trade if required
		if (nextTrade!=0)
		{
			if (exec.numTrades[sym]==0)exec.trade[sym].posSize[exec.numTrades[sym]]=nextTrade*orderSize;
			else exec.trade[sym].posSize[exec.numTrades[sym]]=2*nextTrade*orderSize;
			exec.trade[sym].location[exec.numTrades[sym]]=i;
			exec.numTrades[sym]++;
			nextTrade=0;
		}
	}
}

//This function is called every iteration. DO NOT modify function name or arguments
__device__ __host__
inline void runExecution(bt::stockData* data,long dataSize,
		bt::execution& exec,const bt::parameters& par){
	//modify this line to call custom function:
	crossingMA(data,dataSize,0,100.0,par.lPar[bt::fastMA],par.lPar[bt::slowMA],exec);
}

//namespace bt
}

#endif /* CUSTOM_H_ */
