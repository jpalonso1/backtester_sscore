//Juan Pablo Alonso

#ifndef GPU_H_
#define GPU_H_

#include "setup.h"
#include "xlog.h"
#include <fstream>

std::ofstream testOut("testOut.csv");

__host__ __device__
inline void cpyCharCustom(char* source,char* target){
	//custom character copy that works on gpu
	int cnt=0;
	while (source[cnt]!='\0'){
		target[cnt]=source[cnt];
		cnt++;
	}
}

__host__ __device__
inline void crossingMA(bt::stockData* data,long dataSize,long dataEl,
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
			tempSlow+=data[i-j].d[dataEl];
		}
		slow=tempSlow/float(slowMA);
		//get fast MA
		for (int j=0;j<fastMA;j++){
			tempFast+=data[i-j].d[dataEl];
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
			if (exec.numTrades[dataEl]==0)exec.trade[dataEl].posSize[exec.numTrades[dataEl]]=nextTrade*orderSize;
			else exec.trade[dataEl].posSize[exec.numTrades[dataEl]]=2*nextTrade*orderSize;
			exec.trade[dataEl].location[exec.numTrades[dataEl]]=i;
			exec.numTrades[dataEl]++;
			nextTrade=0;
		}
	}
}

__host__ __device__
inline void getStats(bt::execution& exec,bt::stockData* data,long dataSize){

	float netPos[DATA_ELEMENTS];
	long lastExec[DATA_ELEMENTS];
	long tempMaxDraw[DATA_ELEMENTS];
	long tempMaxDrawTotal=0;
	//initialize results
	exec.resTotal.clear();
	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		exec.resInd[sym].clear();
		netPos[sym]=0;
		lastExec[sym]=0;
		tempMaxDraw[sym]=0;
	}

	float totalPnL=0;
	float periodPnL=0;
	//loop through each record
	for (int i=1;i<dataSize;i++){
		for (int sym=0;sym<DATA_ELEMENTS;sym++){
			//get PnL based on difference from previous record
			if (netPos[sym]==0)periodPnL=0;
			else periodPnL=netPos[sym]*(data[i].d[sym]-data[i-1].d[sym]);

			//positions updated at the end of the day if needed
			if (i==exec.trade[sym].location[lastExec[sym]]){
				netPos[sym]+=exec.trade[sym].posSize[lastExec[sym]];
				lastExec[sym]++;
			}
			//updateDrawdown
			tempMaxDraw[sym]-=periodPnL;
			if (tempMaxDraw[sym]<0)tempMaxDraw[sym]=0;
			if (exec.resInd[sym].maxDrawdown<tempMaxDraw[sym])
				exec.resInd[sym].maxDrawdown=tempMaxDraw[sym];
			tempMaxDrawTotal-=periodPnL;
			if (tempMaxDrawTotal<0)tempMaxDrawTotal=0;
			if (exec.resTotal.maxDrawdown<tempMaxDrawTotal)
				exec.resTotal.maxDrawdown=tempMaxDrawTotal;
//			if (sym==0)testOut<<i<<",draw:,"<<exec.resTotal.maxDrawdown<<",periodPnL,"<<
//					periodPnL<<",tempDraw,"<<tempMaxDrawTotal<<endl;
			exec.resInd[sym].PnL+=periodPnL;
			exec.resTotal.PnL+=periodPnL;
		}
//			testOut<<i<<",pos,"<<netPos<<",PnL,"<<periodPnL<<",total,"<<
//					totalPnL<<endl;
	}
//	cout<<"totalPnL "<<exec.resTotal.PnL<<" max draw "<<exec.resTotal.maxDrawdown<<endl;

}

#endif /* GPU_H_ */
