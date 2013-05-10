//Juan Pablo Alonso Escobar
#include "setup.h"

namespace bt{

void setParameters(thrust::host_vector<parameters>& par){
	for (int i=0;i<par.size();i++){
		par[i].fastMA=20;
		par[i].slowMA=60;
		par[i].orderSize=1000;
		par[i].initEq=100000;
	}
}

void extractRawData(char* filename,thrust::host_vector<bt::stockData>& data,bool header){
	std::ifstream input(filename);
	std::string line;
	char date[20];
	char d[DATA_ELEMENTS][20];
	//looks for comma placements and line length
	int c[DATA_ELEMENTS],len;
	//ignore first line if header exists
	if (header)getline(input,line);
	bt::stockData tempData;
	//loop through each line and assign to vector
	while (!input.eof()){
		getline(input,line);

		c[0]=line.find(',',0);
		//get date
		if (c[0]>0){
			line.copy(date,c[0]);
			date[c[0]]='\0';
			strcpy(tempData.date,date);
		}
		else{
			cout<<"Last line parsed"<<endl;
			break;
		}

		for (int i=0;i<DATA_ELEMENTS;i++){
			//find location of next comma
			c[i+1]=line.find(',',c[i]+1);
			//get value between commas if comma exists
			if (c[i+1]>0){
				line.copy(d[i],c[i+1]-c[i]-1,c[i]+1);
				d[i][c[i+1]-c[i]-1]='\0';

			}
			//if there are no more commas, get last value and exit loop
			else {
				line.copy(d[i],len-c[i],c[i]+1);
				tempData.d[i]=atof(d[i]);
				break;
			}
			//transform to float and copy data to vector
			tempData.d[i]=atof(d[i]);
		}
		data.push_back(tempData);
	}
}

execution::execution() {
	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		numTrades[sym]=0;
	}
}

void result::clear() {
	PnL=0;
	sharpe=0;
	maxDrawdown=0;
	numTransactions=0;
	avgDailyProfit=0;
}

//namespace bt
}



