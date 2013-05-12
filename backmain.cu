//Juan Pablo Alonso Escobar
//GPU Backtester 1.0

#include "setup.h"
#include "gpu.h"
#include "xlog.h"

using namespace std;

void runBacktest(thrust::device_vector<bt::stockData>& data,
		thrust::device_vector<bt::parameters>& par, thrust::device_vector<bt::result>& res,
		long vecSize){
	//create "dummy" vector sequence. Only used to track position
	thrust::device_vector<long> Y(vecSize);
    thrust::sequence(Y.begin(),Y.end());
    //wrap data in device pointer
    bt::stockData* dataPtr=thrust::raw_pointer_cast(&data[0]);
    //transform the vector using the specified function
    thrust::transform(par.begin(), par.end(), Y.begin(), res.begin(),
			individual_run(dataPtr,data.size()));
    //optimize results.
//    thrust::sort(Y.begin(), Y.end(),custom_sort(dataPtr,data.size()));
}

void optimizeParameters(thrust::device_vector<bt::result>& res){
	thrust::sort(res.begin(),res.end(),return_max());
}

int main(){
	cout<<"starting"<<endl;
	XLog logMain("Main process");
	thrust::host_vector<bt::stockData> datah;
	XLog logExtract("Extracting data");
	bt::extractRawData(dataFile,datah,true);
	logExtract.log("Lines: ",datah.size());
	logExtract.end();

	//create vector of parameters to be tested
	thrust::host_vector<bt::parameters> parh;
	long VEC_SIZE=setParameters(parh);
    cout<<"Vector Size: "<<VEC_SIZE<<endl;

    thrust::device_vector<bt::parameters> pard=parh;
    thrust::device_vector<bt::stockData> datad=datah;
//    thrust::device_vector<bt::execution> exec(VEC_SIZE);
    thrust::device_vector<bt::result> res(VEC_SIZE);

    XLog logBacktest("Run backtest");
    logBacktest.start();
    runBacktest(datad
    		,pard,res,VEC_SIZE);
    logBacktest.end();


    XLog logSort("Sorting");
    logSort.start();
    optimizeParameters(res);
    logSort.end();

    thrust::host_vector<bt::result> resh=res;

//    cout<<exech[0].trade[0].location[0]<<endl;
//    cout<<exech[0].trade[0].posSize[0]<<endl;
    cout<<"Parameters vec size: "<<VEC_SIZE<<endl;
    for (int i=0;i<10;i++){
		cout<<i<<"Sum PnL: "<<resh[i].PnL[DATA_ELEMENTS];
		cout<<" sharpe: "<<resh[i].sharpe[DATA_ELEMENTS];
		cout<<" avgdailyProf: "<<resh[i].avgDailyProfit[DATA_ELEMENTS];
		cout<<" Max Draw: "<<resh[i].maxDrawdown[DATA_ELEMENTS]<<endl;
    }

    for (int sym=0;sym<DATA_ELEMENTS;sym++){
		cout<<"Single PnL: "<<resh[0].PnL[sym]<<endl;
    }
    logMain.end();
	return 0;
}
