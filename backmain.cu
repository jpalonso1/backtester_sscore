//Juan Pablo Alonso
//GPU Backtester 1.0

#include "setup.h"
#include "gpu.h"
#include <ctime>

using namespace std;

void runBacktest(thrust::device_vector<bt::stockData>& data,
		thrust::device_vector<bt::parameters>& par, thrust::device_vector<bt::result>& res,
		long vecSize,int etf){
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
	thrust::sort(res.begin(),res.end(),return_max());
}

void printOptimal( thrust::host_vector<bt::result> resh,int etf){
    for (int i=0;i<1;i++){
		cout<<etf<<" - Sum PnL: "<<resh[i].PnL[DATA_ELEMENTS];
		cout<<" sharpe: "<<resh[i].sharpe[DATA_ELEMENTS];
		cout<<" Max Draw: "<<resh[i].maxDrawdown[DATA_ELEMENTS]<<endl;
		cout<<" SBE: "<<resh[i].pars.fPar[bt::SBE][etf];
		cout<<" SBC: "<<resh[i].pars.fPar[bt::SBC][etf];
		cout<<" SSE: "<<resh[i].pars.fPar[bt::SSE][etf];
		cout<<" SSC: "<<resh[i].pars.fPar[bt::SSC][etf];
		cout<<" WindowSize: "<<resh[i].pars.lPar[bt::windowSize][etf]<<endl;
    }
}

void copyResult(bt::result& optRes,bt::result& lastRes,int etf){
	optRes.pars.fPar[bt::SBE][etf]=lastRes.pars.fPar[bt::SBE][etf];
	optRes.pars.fPar[bt::SBC][etf]=lastRes.pars.fPar[bt::SBC][etf];
	optRes.pars.fPar[bt::SSE][etf]=lastRes.pars.fPar[bt::SSE][etf];
	optRes.pars.fPar[bt::SSC][etf]=lastRes.pars.fPar[bt::SSC][etf];
	optRes.pars.fPar[bt::windowSize][etf]=lastRes.pars.fPar[bt::windowSize][etf];
}

int main(){
	//get data
	thrust::host_vector<bt::stockData> datah;
	bt::extractRawData(dataFile,datah,true);
	thrust::device_vector<bt::stockData>datad(datah.size());
//	thrust::device_vector<bt::stockData> datad=datah;
	thrust::copy(datah.begin(), datah.end(), datad.begin());

    bt::result optRes;
    int etf;
    for (etf=0;etf<35;etf++){
		//create vector of parameters to be tested
		thrust::host_vector<bt::parameters> parh;
		long VEC_SIZE=setParameters(parh,etf);
		cout<<"Number of simulations: "<<VEC_SIZE<<endl;
		thrust::device_vector<bt::parameters> pard(VEC_SIZE);
		thrust::copy(parh.begin(), parh.end(), pard.begin());
		//    thrust::device_vector<bt::parameters> pard=parh;
		thrust::device_vector<bt::result> resd(VEC_SIZE);
		thrust::host_vector<bt::result> resh(VEC_SIZE);

		setParameters(parh,etf);
		//run the backtesting on gpu
		runBacktest(datad,pard,resd,VEC_SIZE,etf);

		//sort on gpu
		optimizeParameters(resd);
		thrust::copy(resd.begin(), resd.end(), resh.begin());

		//update optimalRes
		copyResult(optRes,resh[0],etf);

		//sample output
		printOptimal(resh,etf);
    }




    clock_t timeEnd=clock();

    cout<<"returned s Scores: "<<optRes.temp<<endl;
    cout<<"returned s Scores: "<<optRes.temp<<endl;
    cout<<"Total Runtime (see README.txt): "<<double(timeEnd)/double(CLOCKS_PER_SEC)<<" seconds"<<endl;

    return 0;
}
