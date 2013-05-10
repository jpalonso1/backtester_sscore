//Juan Pablo Alonso Escobar
//GPU Backtester 1.0

#include "setup.h"
#include "gpu.h"
#include "xlog.h"

using namespace std;


__host__ __device__
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

//	//TEMP: print
//	for (int sym=0;sym<1;sym++){
//		float execPnL=0;
//		for (long i=0;i<(exec.numTrades[sym]);i++){
//			float thisPos=exec.trade[sym].posSize[i];
//			float thisPrice=data[exec.trade[sym].location[i]].d[sym];
//			execPnL+=exec.trade[sym].realPnL[i];
////			testOut<<i<<",price,"<<thisPrice<<",pos,"<<thisPos<<",PnL,"<<
////					exec.trade[sym].realPnL[i]<<",loc,"<<
////					exec.trade[sym].location[i]<<endl;
//		}
//
//	}
}

struct individual_run
{
	//hold a copy of the pointer to data
	bt::stockData* data;
	long dataSize;
    individual_run(bt::stockData* _data,long _dataSize) :
    	data(_data),dataSize(_dataSize) {}

    __host__ __device__
    bt::execution operator()(const bt::parameters& par, const long& Y) const {
    	//to be run every iteration of the backtest
    	bt::execution execTemp;
    	initExec(execTemp);
    	crossingMA(data,dataSize,0,100.0,par.fastMA,par.slowMA,execTemp);
    	aggregateResults(execTemp,data,dataSize);
    	getStats(execTemp,data,dataSize);
    	return execTemp;
	}
};

void runBacktest(thrust::device_vector<bt::stockData>& data,
		thrust::device_vector<bt::parameters>& par, thrust::device_vector<bt::execution>& exec){
	thrust::device_vector<long> Y(100);
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
	const long VEC_SIZE=1000;

	//create vector of parameters to be tested
	thrust::host_vector<bt::parameters> parh(VEC_SIZE);
	setParameters(parh);
    parh[0].fastMA=27;
    thrust::device_vector<bt::parameters> pard=parh;
    thrust::device_vector<bt::stockData> datad=datah;
    cout<<"creating execution..."<<endl;
    thrust::device_vector<bt::execution> exec(VEC_SIZE);
    cout<<"end creating execution..."<<endl;
    XLog logBacktest("Run backtest");
    runBacktest(datad,pard,exec);
    logBacktest.end();

    thrust::host_vector<bt::execution> exech=exec;

    cout<<exech[0].trade[0].location[0]<<endl;
    cout<<exech[0].trade[0].posSize[0]<<endl;
    cout<<"Sum PnL: "<<exech[0].resTotal.PnL<<endl;
    cout<<"Max Draw: "<<exech[0].resTotal.maxDrawdown<<endl;
    logMain.end();
	return 0;
}

void testDemo(){

}
