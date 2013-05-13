//Juan Pablo Alonso

#include "setup.h"
#include <math.h>

#ifndef S_SSCORE_H_
#define S_SSCORE_H_

const int MAX_WINDOW=100;

namespace bt{

struct ols_pair{
    float beta;
    float alpha;
};


struct trade_param{
    double hedge_ratio;
    double s_score;
};


__device__ __host__
float getReturn(bt::stockData* data,int sym,long position){
	return (data[position].d[sym]/data[position-1].d[sym])-1.0;
}
__device__ __host__
ols_pair ols_regression(float x_var[], float y_var[], int win_size){

    float sum_x(0), sum_y(0);
    float sum_x_square(0), sum_cross(0);

    for(int i = 0; i < win_size; i++){
        sum_x += x_var[i];
        sum_y += y_var[i];
    }

    float x_mean = sum_x / win_size;
    float y_mean = sum_y / win_size;

    for(int i = 0; i < win_size; i++){
        sum_x_square += powf(x_var[i] - x_mean, 2);
        sum_cross += (x_var[i] - x_mean) * (y_var[i] - y_mean);
    }

    float x_variance = sum_x_square / (win_size - 1);
    float xy_cov = sum_cross / (win_size - 1);

    ols_pair ols_result;
    ols_result.beta = xy_cov / x_variance;
    ols_result.alpha = (y_mean - x_mean * ols_result.beta);

    return ols_result;
}


__device__ __host__
trade_param comp_s_score(float x_var[], float y_var[], int win_size){

    // This is the final output, which contains the hedge_ratio and s-score.
    trade_param trade_param_output;

    // First OLS regression on the pair returns.
    ols_pair ols_return = ols_regression(x_var, y_var, win_size);
    trade_param_output.hedge_ratio = ols_return.beta;

    // Find the residues of the regression result of the returns.
    float cum_error[MAX_WINDOW];
    float resid[MAX_WINDOW-1], resid_lag[MAX_WINDOW-1];

    cum_error[0] = y_var[0] - (ols_return.alpha + ols_return.beta*x_var[0]);

    for(int i = 1; i < win_size; i++){
        cum_error[i] = cum_error[i-1] + y_var[i] - (ols_return.alpha + ols_return.beta*x_var[i]);
    }

    for(int i = 0; i < win_size; i++){
        if(i != 0) { resid[i- 1] = cum_error[i]; }
        if(i != win_size - 1){ resid_lag[i] = cum_error[i]; }
    }

    // OLS regression on the residues - AR(1) model.
    ols_pair ols_resid = ols_regression(resid_lag, resid, win_size-1);

    float resid_stdev;
    float sum_squares(0);
    float sum_resid(0);

    for(int i = 0; i < win_size-1; i++){
        sum_resid = resid[i] - resid_lag[i]*ols_resid.beta - ols_resid.alpha;
        sum_squares += powf(sum_resid, 2.0);
    }

    resid_stdev = sqrtf((sum_squares/(win_size-1.0) - powf(sum_resid/(win_size-1), 2.0)));

    // Compute s_score with drift according to the paper [Marco Avellaneda, Jeong-Hyun Lee].
    float k = - logf(ols_resid.beta) * 252;
    float m = ols_resid.alpha / (1-ols_resid.beta);
    float sigma_eq = resid_stdev / sqrtf(1.0-ols_resid.beta*ols_resid.beta);
    float s_score = - m / sigma_eq;
    float s_score_with_drift = s_score - ols_return.alpha / (k*sigma_eq);

    trade_param_output.s_score = s_score_with_drift;

    return trade_param_output;
}

//int main(){
//    float x_var[] = {60, 61, 62, 63, 65};
//    float y_var[] = {3.1, 3.6, 3.8, 4, 4.1};
//
//    trade_param trade_param_output = comp_s_score(x_var, y_var, 5);
//    cout << fixed << setprecision(9) << trade_param_output.hedge_ratio << endl << trade_param_output.s_score << endl;
//}

__device__ __host__
void runSScore(bt::stockData* data,bt::execution& exec,
		long dataSize,long orderSize,long win_size,
		int sym, int etf,float SBE,float SBC,float SSE,float SSC){
	trade_param tempS;
	//x is etf, y is sym
	float x_var[MAX_WINDOW],y_var[MAX_WINDOW];
	long winStart=0;
	//1 is long sym short etf, -1 opposite
	int currentPosition=0;
	long symShares=0;
	long etfShares=0;

	for (long i=win_size+1;i<dataSize;i++){
		winStart=i-win_size;
		for (long j=0;j<win_size;j++){
			x_var[j]=(data[winStart+j].d[etf]/data[winStart+j-1].d[etf])-1.0;
			y_var[j]=(data[winStart+j].d[sym]/data[winStart+j-1].d[sym])-1.0;
		}

		tempS=comp_s_score(x_var,y_var,win_size);
		float sScore=tempS.s_score;

		if (currentPosition==0){
			//buy sym sell etf
			if (sScore<SBE){
				symShares=recordTrade(data,exec,sym,i,orderSize);
				etfShares=recordTrade(data,exec,etf,i,-orderSize*tempS.hedge_ratio);
				currentPosition=1;
			}
			//but etf sell sym
			else if(sScore>SSE){
				symShares=recordTrade(data,exec,sym,i,-orderSize);
				etfShares=recordTrade(data,exec,etf,i,orderSize*tempS.hedge_ratio);
				currentPosition=-1;
			}
		}
		//check for close
		else{
			if (sScore>SBC && currentPosition==1){
				closePosition(data,exec,sym,i);
				closePosition(data,exec,etf,i);
				currentPosition=0;
			}
			else if (sScore<SSC && currentPosition==-1){
				closePosition(data,exec,sym,i);
				closePosition(data,exec,etf,i);
				currentPosition=0;
			}
		}
	}
}


}
#endif /* S_SSCORE_H_ */
