//Juan Pablo Alonso

#ifndef SETUP_H_
#define SETUP_H_

#include <iostream>
#include <fstream>
#include <cstdio>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/transform.h>
#include <thrust/sequence.h>
#include <thrust/sort.h>

using std::cout;
using std::endl;

const int SYM_COUNT=35;
const int DATA_ELEMENTS=35*2;
const int LONG_PARAMETERS=5;
const int FLOAT_PARAMETERS=5;
const int YEAR_PERIODS=252;
const int MAX_ORDERS=2000;
//only "neutral" strategies implemented for sharpe
const float BENCHMARK=0;

namespace bt{

//long parameters
//enum {atrlen=0,fastMA=1,slowMA=2,orderSize=3};
enum {orderSize=1,windowSize=2};
//float parameters
//enum {cutoff=0};
enum {SBE=0,SBC=1,SSE=2,SSC=3};

struct parameters{
	//DES:holds floating (fPar) and long (lPar)
	//parameters for EACH iteration
	float fPar[FLOAT_PARAMETERS][SYM_COUNT];
	long lPar[LONG_PARAMETERS][SYM_COUNT];
};

struct stockData{
	//DES: hold each line of the stock data.
	//date as string
	char date[20];
	//each of the optional elements: prices of multiple stocks,
	//volumes, custom indicators, etc.
	float d[DATA_ELEMENTS];
};

struct trade{
	//negative for short/sell
	long posSize[MAX_ORDERS];
	//"time" (vector element location) of execution relative to data
	long location[MAX_ORDERS];
	float price[MAX_ORDERS];

	//pnl for closing trades
	float realPnL[MAX_ORDERS];
};

struct result{
	float PnL[DATA_ELEMENTS+1];
	float sharpe[DATA_ELEMENTS+1];
	float maxDrawdown[DATA_ELEMENTS+1];
	float numTransactions[DATA_ELEMENTS+1];
	float avgDailyProfit[DATA_ELEMENTS+1];
	parameters pars;
};

struct execution{
	trade trade[DATA_ELEMENTS];
	result result;
	char symbol[DATA_ELEMENTS][20];
	long numTrades[DATA_ELEMENTS];
//	execution();
};

void extractRawData(char* filename,thrust::host_vector<bt::stockData>& data,bool header=false);

__device__ __host__
inline long recordTrade(bt::stockData* data,bt::execution& exec,
		long sym,long location,long amount,bool fixedShares=false){
	//get number of shares (rounded down) to get amount
	long adjAmount=amount/data[location].d[sym];
	if (fixedShares==false)exec.trade[sym].posSize[exec.numTrades[sym]]=adjAmount;
	else exec.trade[sym].posSize[exec.numTrades[sym]]=amount;
	exec.trade[sym].price[exec.numTrades[sym]]=data[location].d[sym];
	exec.trade[sym].location[exec.numTrades[sym]]=location;
	exec.numTrades[sym]++;
//	if (exec.numTrades[sym]==MAX_ORDERS)cout<<"WARNING: max orders reached"<<endl;
	return exec.trade[sym].posSize[exec.numTrades[sym]];
}

__device__ __host__
inline long closePosition(bt::stockData* data,bt::execution& exec,
		int sym,long location){
	long closeAmount=-exec.trade[sym].posSize[exec.numTrades[sym]-1];
	return recordTrade(data,exec,sym,location,closeAmount,true);
}
}//namespace bt

#endif /* SETUP_H_ */
