//Juan Pablo Alonso

#include "custom.h"

namespace bt{

long setParameters(thrust::host_vector<bt::parameters>& par){
	long count=0;
	//DES:defines the parameters to be tested
	for (int i=0;i<1000;i++){
		bt::parameters tempPar;
		tempPar.lPar[fastMA]=20;
		tempPar.lPar[slowMA]=60;
		tempPar.lPar[orderSize]=1000;
		tempPar.fPar[initEq]=100000;
		par.push_back(tempPar);
		count++;
	}
	return count;
}

//n bt
}





