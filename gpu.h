//Juan Pablo Alonso

#ifndef GPU_H_
#define GPU_H_

#include "setup.h"
#include "custom.h"
#include "xlog.h"
#include <fstream>

std::ofstream testOut("testOut.csv");

__device__ __host__
inline void cpyCharCustom(char* source,char* target){
	//custom character copy that works on gpu
	int cnt=0;
	while (source[cnt]!='\0'){
		target[cnt]=source[cnt];
		cnt++;
	}
}

__device__ __host__
void initExec(bt::execution& exec){
	exec.numTrades[0]=0;
    for (int sym=0;sym<DATA_ELEMENTS;sym++){
    	exec.numTrades[sym]=0;
    }
}

__device__ __host__
void clearResult(bt::result& res){
	res.PnL=0;
	res.sharpe=0;
	res.maxDrawdown=0;
	res.numTransactions=0;
	res.avgDailyProfit=0;
}


__device__ __host__
inline void getStats(bt::execution& exec,bt::stockData* data,long dataSize){

	float netPos[DATA_ELEMENTS];
	long lastExec[DATA_ELEMENTS];
	long tempMaxDraw[DATA_ELEMENTS];
	long tempMaxDrawTotal=0;
	//initialize results
	clearResult(exec.resTotal);
	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		clearResult(exec.resInd[sym]);
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



__device__ __host__
void aggregateResults(bt::execution& exec,bt::stockData* data,long dataSize){
	//find the PnL for each execution IF it is closing an exisiting position
	long thisPos=0;
	long partSize=0;
	for (int sym=0;sym<DATA_ELEMENTS;sym++)
		{
		//when last position was closed or reversed
		long lastClose=0;
		//direction (1 buy, 0 sell)
		int dir;
		//current net positon
		long netPos=exec.trade[sym].posSize[0];
		long thisPos,targetFill;
		float thisPrice;
		long partialFill=0;
		bool closed;

//		//loop through each execution and update results
		for (long i=1;i<=exec.numTrades[sym];i++){
//		for (long i=1;i<min(10,int(exec.numTrades[sym]));i++){
			//set position,price and direction
			thisPos=exec.trade[sym].posSize[i];
			thisPrice=data[exec.trade[sym].location[i]].d[sym];
			if (thisPos>0)dir=1;
			else dir=0;
			//check if closing position (reducing abs net)
			if (thisPos*netPos<0)closed=true;
			else closed=false;
			//update net position
			netPos+=thisPos;

			//Check for position closed, capture profit
			if (closed==true){
				//check if trade exceeds current position (reversal) and adjust
				if (thisPos*netPos>0)thisPos-=netPos;
				targetFill=thisPos;
				float priceSum=0;
				int dirCheck=0;
				for(long j=lastClose;j<exec.numTrades[sym];j++){
					//get checked trade direction
					if (exec.trade[sym].posSize[j]>0)dirCheck=1;
					else dirCheck=0;
					//check direction. if trades do not cancel each other, continue
					if (dirCheck==dir)continue;

					float posCheck=exec.trade[sym].posSize[j]+partialFill;


					float priceCheck=data[exec.trade[sym].location[j]].d[sym];
					//check for enough shares at current trade
					if (abs(posCheck)>abs(targetFill)){
						priceSum+=targetFill*priceCheck;
						partialFill+=targetFill;
						targetFill=0;
						lastClose=j;
					}
					//if check is smaller, take entire position and move on
					else{
						priceSum+=posCheck*priceCheck;
						partialFill=0;
						targetFill+=posCheck;
						lastClose=j+1;
					}
					//check if position has been filled
					if (targetFill==0)break;
				}
				//clear partial if crossed
				if (thisPos*netPos>0)partialFill=0;
				float avgPrice=abs(priceSum/thisPos);
				//update pnl
				exec.trade[sym].realPnL[i]=thisPos*(avgPrice-thisPrice);
			}
			//add a "closer"execution that assumes position is
			//forcefully closed at last date
			if (((i+1)==exec.numTrades[sym]) && (netPos!=0)){
				exec.trade[sym].posSize[exec.numTrades[sym]]=-netPos;
				exec.trade[sym].location[exec.numTrades[sym]]=dataSize-1;
				exec.numTrades[sym]++;
			}
		}
	}

	//TEMP: print
	for (int sym=0;sym<1;sym++){
		float execPnL=0;
		for (long i=0;i<(exec.numTrades[sym]);i++){
			float thisPos=exec.trade[sym].posSize[i];
			float thisPrice=data[exec.trade[sym].location[i]].d[sym];
			execPnL+=exec.trade[sym].realPnL[i];
			testOut<<i<<",price,"<<thisPrice<<",pos,"<<thisPos<<",PnL,"<<
					exec.trade[sym].realPnL[i]<<",loc,"<<
					exec.trade[sym].location[i]<<endl;
		}

	}
}

struct individual_run
{
	//hold a copy of the pointer to data
	bt::stockData* data;
	long dataSize;
    individual_run(bt::stockData* _data,long _dataSize) :
    	data(_data),dataSize(_dataSize) {}

    __device__ __host__
    bt::execution operator()(const bt::parameters& par, const long& Y) const {
    	//to be run every iteration of the backtest
    	bt::execution execTemp;
    	initExec(execTemp);
    	cout<<"in run "<<Y<<endl;
    	cout<<"FM :"<<par.lPar[bt::fastMA]<<" SM: "<<par.lPar[bt::slowMA]<<endl;
    	crossingMA(data,dataSize,0,100.0,par.lPar[bt::fastMA],par.lPar[bt::slowMA],execTemp);
    	aggregateResults(execTemp,data,dataSize);
    	getStats(execTemp,data,dataSize);
    	return execTemp;
	}
};


#endif /* GPU_H_ */
