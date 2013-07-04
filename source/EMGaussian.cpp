#include "EMGaussian.h"
#include "MathFunctions.h"

EMGaussian::EMGaussian(const std::vector<double> &data, const std::vector<Param> &params) : 
    EM(data, params) {

}

EMGaussian::~EMGaussian() { 

}

void EMGaussian::MStep() {
    for(int k=0; k<params_.size(); k++) {
        // Compute new means
        double numeratorSum = 0;
        double denominatorSum = 0;
        for(int n=0; n<data_.size(); n++) {
            numeratorSum += pikn_[n][k]*data_[n];
        }
        params_[k].u = numeratorSum / denominatorSum;

        numeratorSum = 0;
        // Compute new standard deviations

        for(int n=0; n<data_.size(); n++) {
            double dx = data_[n]-params_[k].u;
            numeratorSum += pikn_[n][k]*dx*dx;
        }
        params_[k].s = sqrt(numeratorSum / denominatorSum);

        params_[k].p = denominatorSum / data_.size(); 
    }
}

double EMGaussian::qkn(int k, int n) const {
   double pk = params_[k].p;
   double uk = params_[k].u;
   double sk = params_[k].s;    
   double xn = data_[n];
   return pk * gaussian(uk, sk, xn);
}
