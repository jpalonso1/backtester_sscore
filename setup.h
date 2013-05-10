//Juan Pablo Alonso Escobar
#ifndef SETUP_H_
#define SETUP_H_

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/transform.h>
#include <thrust/sequence.h>
#include <thrust/sort.h>

using std::cout;
using std::endl;

const int DATA_ELEMENTS=3;
const int LONG_PARAMETERS=5;
const int FLOAT_PARAMETERS=5;

namespace bt{

struct parameters{
	float fPar[FLOAT_PARAMETERS];
	long lPar[LONG_PARAMETERS];
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
	long posSize[100];
	//"time" (vector element location) of execution relative to data
	long location[100];
	//pnl for closing trades
	float realPnL[100];
};

struct result{
	float PnL;
	float sharpe;
	float maxDrawdown;
	float numTransactions;
	float avgDailyProfit;
};

struct execution{
	trade trade[DATA_ELEMENTS];
	result resInd[DATA_ELEMENTS];
	result resTotal;
	char symbol[DATA_ELEMENTS][20];
	long numTrades[DATA_ELEMENTS];
//	execution();
};

void extractRawData(char* filename,thrust::host_vector<bt::stockData>& data,bool header=false);

}//namespace bt

#endif /* SETUP_H_ */
