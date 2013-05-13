//Juan Pablo Alonso

#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "setup.h"
#include "s_sscore.h"

static char* dataFile={"backtest_value_signal_etf.csv"};
static std::ofstream testOut("testOut.csv");

namespace bt{

void recMixerParameters(thrust::host_vector<bt::parameters>& par,parameters tempPar){

}

//void customOptParameters(thrust::host_vector<bt::parameters>& par,parameters tempPar){
//	for (int etf=0;etf<35;etf++){
//		tempPar.lPar[bt::orderSize][etf]=100000;
//		tempPar.fPar[bt::SBE][etf]=-0.5;
//		tempPar.fPar[bt::SBC][etf]=-0.2;
//		tempPar.fPar[bt::SSE][etf]=0.5;
//		tempPar.fPar[bt::SSC][etf]=0.2;
//		tempPar.lPar[bt::windowSize][etf]=10;
//	}
//}

long mixedParameters(thrust::host_vector<bt::parameters>& par,int etf){
	long count=0;
	parameters tempPar;
	tempPar.lPar[bt::orderSize][etf]=100000;
	for (int a=0;a<1;a++){
	tempPar.fPar[bt::SBE][etf]=-0.5-a*0.2;
	for (int b=0;b<1;b++){
	tempPar.fPar[bt::SBC][etf]=-0.2-b*0.1;
	for (int c=0;c<1;c++){
	tempPar.fPar[bt::SSE][etf]=0.5+c*0.2;
	for (int d=0;d<1;d++){
	tempPar.fPar[bt::SSC][etf]=0.2+d*0.1;
	for (int e=0;e<10;e++){
	tempPar.lPar[bt::windowSize][etf]=10+e*5;
	par.push_back(tempPar);
	count++;
	}}}}}
	return count;
}

long setParameters(thrust::host_vector<bt::parameters>& par, int etf){
	return mixedParameters(par, etf);
}

//This function is called in every thread. DO NOT modify function name or arguments
__device__ __host__
inline void runExecution(bt::stockData* data,long dataSize,
		bt::execution& exec,const bt::parameters& par,int etf){

	if (etf==-1){
		for (int etf=0;etf<35;etf++){
			bt::runSScore(data,exec,dataSize,par.lPar[bt::orderSize][etf],
				par.lPar[bt::windowSize][etf],etf+35,etf,
				par.fPar[bt::SBE][etf],par.fPar[bt::SBC][etf],
				par.fPar[bt::SSE][etf],par.fPar[bt::SSC][etf]);
		}
	}
	else
	{
		bt::runSScore(data,exec,dataSize,par.lPar[bt::orderSize][etf],
			par.lPar[bt::windowSize][etf],etf+35,etf,
			par.fPar[bt::SBE][etf],par.fPar[bt::SBC][etf],
			par.fPar[bt::SSE][etf],par.fPar[bt::SSC][etf]);
	}


	//	cout<<data[1618].d[0]<<endl;
//	cout<<data[1618].d[1]<<endl;
//	cout<<data[1618].d[2]<<endl;
//	cout<<data[1618].d[3]<<endl;
}

//namespace bt
}

#endif /* CUSTOM_H_ */
