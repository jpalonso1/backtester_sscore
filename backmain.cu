//Juan Pablo Alonso Escobar
//GPU Backtester 1.0

#include "setup.h"
#include "gpu.h"
#include "xlog.h"
#include "custom.h"

using namespace std;

void runBacktest(thrust::device_vector<bt::stockData>& data,
		thrust::device_vector<bt::parameters>& par, thrust::device_vector<bt::execution>& exec,
		long vecSize){
	thrust::device_vector<long> Y(vecSize);
    thrust::sequence(Y.begin(),Y.end());
    //wrap data in device pointer
    bt::stockData* dataPtr=thrust::raw_pointer_cast(&data[0]);
    //transform the vector using the specified function
//    cout<<"running transform"<<endl;
    thrust::transform(par.begin(), par.end(), Y.begin(), exec.begin(),
			individual_run(dataPtr,data.size()));

}


int main(){
	cout<<"starting"<<endl;
	XLog logMain("Main process");
	thrust::host_vector<bt::stockData> datah;
	XLog logExtract("Extracting data");
	bt::extractRawData("AAPLclean.csv",datah,true);
	logExtract.log("Lines: ",datah.size());
	logExtract.end();

	//create vector of parameters to be tested
	thrust::host_vector<bt::parameters> parh;
	long VEC_SIZE=setParameters(parh);;
    cout<<"Vector Size: "<<VEC_SIZE<<endl;

    thrust::device_vector<bt::parameters> pard=parh;
    thrust::device_vector<bt::stockData> datad=datah;
    thrust::device_vector<bt::execution> exec(VEC_SIZE);

    XLog logBacktest("Run backtest");
    logBacktest.start();
    runBacktest(datad,pard,exec,VEC_SIZE);
    logBacktest.end();

    thrust::host_vector<bt::execution> exech=exec;

    cout<<exech[0].trade[0].location[0]<<endl;
    cout<<exech[0].trade[0].posSize[0]<<endl;
    cout<<"Parameters vec size: "<<VEC_SIZE<<endl;
    cout<<"Sum PnL: "<<exech[0].resTotal.PnL<<endl;
    cout<<"Max Draw: "<<exech[0].resTotal.maxDrawdown<<endl;
    logMain.end();
	return 0;
}
