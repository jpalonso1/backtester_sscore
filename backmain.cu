//Juan Pablo Alonso
//GPU Backtester 1.0

#include "setup.h"
#include "gpu.h"
#include <ctime>

using namespace std;

void runBacktest(thrust::device_vector<bt::stockData>& data,
		thrust::device_vector<bt::parameters>& par, thrust::device_vector<bt::result>& res,
		long vecSize,int etf,bool totalRun=false){
	//create "dummy" vector sequence. Only used to track position
	thrust::device_vector<long> Y(vecSize);
    thrust::sequence(Y.begin(),Y.end());
    //wrap data in device pointer
    bt::stockData* dataPtr=thrust::raw_pointer_cast(&data[0]);
    //transform the vector using the specified function
    thrust::transform(par.begin(), par.end(), Y.begin(), res.begin(),
			individual_run(dataPtr,data.size(),etf));

}

void optimizeParameters(thrust::device_vector<bt::result>& res){
	thrust::sort(res.begin(),res.end(),retdraw_max());
}

void printOptimal(bt::result resh,int etf){
	cout<<etf<<" - Sum PnL: "<<resh.PnL[DATA_ELEMENTS];
//	cout<<" sharpe: "<<resh.sharpe[DATA_ELEMENTS];
	cout<<" Max Draw: "<<resh.maxDrawdown[DATA_ELEMENTS]<<endl;
	cout<<" SBE: "<<resh.pars.fPar[bt::SBE][etf];
	cout<<" SBC: "<<resh.pars.fPar[bt::SBC][etf];
	cout<<" SSE: "<<resh.pars.fPar[bt::SSE][etf];
	cout<<" SSC: "<<resh.pars.fPar[bt::SSC][etf];
	cout<<" WindowSize: "<<resh.pars.lPar[bt::windowSize][etf]<<endl;
}

void printParameter(bt::parameters pars){
	parametersOut<<"sym,orderSize,SBE,SBC,SSE,SSC,windowSize"<<endl;
	for (int etf=0;etf<35;etf++){
		parametersOut<<etf<<","<<pars.lPar[bt::orderSize][etf]
		<<","<<pars.fPar[bt::SBE][etf]
		<<","<<pars.fPar[bt::SBC][etf]
		<<","<<pars.fPar[bt::SSE][etf]
		<<","<<pars.fPar[bt::SSC][etf]
		<<","<<pars.lPar[bt::windowSize][etf]<<endl;

	}
}

void copyResult(bt::result& optRes,bt::result& lastRes,int etf){
	optRes.pars.lPar[bt::orderSize][etf]=lastRes.pars.lPar[bt::orderSize][etf];
	optRes.pars.fPar[bt::SBE][etf]=lastRes.pars.fPar[bt::SBE][etf];
	optRes.pars.fPar[bt::SBC][etf]=lastRes.pars.fPar[bt::SBC][etf];
	optRes.pars.fPar[bt::SSE][etf]=lastRes.pars.fPar[bt::SSE][etf];
	optRes.pars.fPar[bt::SSC][etf]=lastRes.pars.fPar[bt::SSC][etf];
	optRes.pars.lPar[bt::windowSize][etf]=lastRes.pars.lPar[bt::windowSize][etf];
}

int main(){
	//get data
	thrust::host_vector<bt::stockData> datah;
	bt::extractRawData(iSample,datah,true);
	thrust::device_vector<bt::stockData>datad(datah.size());
//	thrust::device_vector<bt::	stockData> datad=datah;
	thrust::copy(datah.begin(), datah.end(), datad.begin());

	long simCount=0;
	long VEC_SIZE;
    bt::result optRes;
    int etf;
    for (etf=0;etf<35;etf++){
		//create vector of parameters to be tested
		thrust::host_vector<bt::parameters> parh;
		VEC_SIZE=setParameters(parh,etf);
		cout<<"Number of simulations: "<<simCount<<endl;
		simCount+=VEC_SIZE;
		thrust::device_vector<bt::parameters> pard(VEC_SIZE);
		thrust::copy(parh.begin(), parh.end(), pard.begin());
		//    thrust::device_vector<bt::parameters> pard=parh;
		thrust::device_vector<bt::result> resd(VEC_SIZE);
		thrust::host_vector<bt::result> resh(VEC_SIZE);

		//run the backtesting on gpu
		runBacktest(datad,pard,resd,VEC_SIZE,etf);

		//sort on gpu
		optimizeParameters(resd);
		thrust::copy(resd.begin(), resd.end(), resh.begin());

		//update optimalRes
		copyResult(optRes,resh[0],etf);

		//sample output
		printOptimal(resh[0],etf);
    }

	thrust::host_vector<bt::stockData> dataho;
	bt::extractRawData(iSample,dataho,true);
	thrust::device_vector<bt::stockData>datado(dataho.size());
//	thrust::device_vector<bt::	stockData> datad=datah;
	thrust::copy(dataho.begin(), dataho.end(), datado.begin());

    for (etf=0;etf<1;etf++){
		//create vector of parameters to be tested
		thrust::host_vector<bt::parameters> parh;
		simCount+=VEC_SIZE;
		cout<<"Number of simulations: "<<simCount<<endl;
		thrust::device_vector<bt::parameters> pard(1);

		parh.push_back(optRes.pars);

		thrust::copy(parh.begin(), parh.end(), pard.begin());
		//    thrust::device_vector<bt::parameters> pard=parh;
		thrust::device_vector<bt::result> resd(1);
		thrust::host_vector<bt::result> resh(1);


		//run the backtesting on gpu
		runBacktest(datado,pard,resd,1,-1);

		//sort on gpu
		optimizeParameters(resd);
		thrust::copy(resd.begin(), resd.end(), resh.begin());

		//update optimalRes
		copyResult(optRes,resh[0],etf);

		//sample output
		printOptimal(resh[0],etf);
    }

    printParameter(optRes.pars);
    clock_t timeEnd=clock();

    cout<<"Total Runtime: "<<double(timeEnd)/double(CLOCKS_PER_SEC)<<" seconds"<<endl;

    return 0;
}
