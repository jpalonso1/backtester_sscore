//Juan Pablo Alonso

#include "custom.h"

namespace bt{

__device__ __host__
void crossingMA(bt::stockData* data,long dataSize,long sym,
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


long setParameters(thrust::host_vector<bt::parameters>& par){
	long count=0;
	//DES:defines the parameters to be tested
	for (int i=0;i<10000;i++){
		bt::parameters tempPar;
		tempPar.lPar[orderSize]=1000;
		tempPar.fPar[initEq]=100000;
		for (int j=0;j<1;j++){
			tempPar.lPar[fastMA]=20+j;
			for (int k=0;k<1;k++){
				tempPar.lPar[slowMA]=60+k;
				par.push_back(tempPar);
				count++;
			}
		}
	}
	return count;
}

//n bt
}





