//Juan Pablo Alonso

#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "setup.h"
#include "s_momentum.h"
#include "s_sscore.h"

static char* dataFile={"backtest_value_signal_etf.csv"};
static std::ofstream testOut("testOut.csv");

namespace bt{

void recMixerParameters(thrust::host_vector<bt::parameters>& par,parameters tempPar){

}

long mixedParameters(thrust::host_vector<bt::parameters>& par){
	long count=0;
	parameters tempPar;
	tempPar.lPar[bt::orderSize]=100000;
	for (int a=0;a<1;a++){
	tempPar.fPar[bt::SBE]=-0.5-a*0.2;
	for (int b=0;b<1;b++){
	tempPar.fPar[bt::SBC]=-0.2-b*0.1;
	for (int c=0;c<1;c++){
	tempPar.fPar[bt::SSE]=0.5+c*0.2;
	for (int d=0;d<1;d++){
	tempPar.fPar[bt::SSC]=0.2+d*0.1;
	for (int e=0;e<10;e++){
	tempPar.lPar[bt::windowSize]=10+e*5;
	par.push_back(tempPar);
	count++;
	}}}}}
	return count;
}

long setParameters(thrust::host_vector<bt::parameters>& par){
	return mixedParameters(par);
}

//This function is called in every thread. DO NOT modify function name or arguments
__device__ __host__
inline void runExecution(bt::stockData* data,long dataSize,
		bt::execution& exec,const bt::parameters& par){
	//modify this line to call custom function:
//	for (int sym=0;sym<DATA_ELEMENTS;sym++){
//		crossingMA(data,exec,dataSize,sym,par.lPar[bt::orderSize],
//				par.lPar[bt::fastMA],par.lPar[bt::slowMA],
//				par.lPar[bt::atrlen],par.fPar[bt::cutoff]);
//	}
//	executeMomentum(data,exec,dataSize,par);
	int symCount=35;
	int etfAdd=symCount;
	for (int etf=0;etf<symCount;etf++){
		int sym=etf+etfAdd;
		bt::runSScore(data,exec,dataSize,par.lPar[bt::orderSize],
			par.lPar[bt::windowSize],sym,etf,
			par.fPar[bt::SBE],par.fPar[bt::SBC],
			par.fPar[bt::SSE],par.fPar[bt::SSC]);
	}

	//	cout<<data[1618].d[0]<<endl;
//	cout<<data[1618].d[1]<<endl;
//	cout<<data[1618].d[2]<<endl;
//	cout<<data[1618].d[3]<<endl;
}

//namespace bt
}

#endif /* CUSTOM_H_ */
