#include "EM.h"
#include "MathFunctions.h"
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>

#ifdef _WIN32
#define isinf(x) !_finite(x)
#endif

namespace Terran {

using namespace std;

EM::EM(const std::vector<double> &data) : 
    data_(data),
    pikn_(data.size(), std::vector<double>(0)),
    maxSteps_(200),
    tolerance_(0.1) {
    if(data_.size() == 0)
        throw(std::runtime_error("Cannot initialize EM with empty dataset"));

}

EM::EM(const std::vector<double> &data, const std::vector<Param> &params) : 
    data_(data),
    params_(params),
    pikn_(data.size(), std::vector<double>(params.size(),0)),
    maxSteps_(200),
    tolerance_(0.1) {

    if(data_.size() == 0)
        throw(std::runtime_error("Cannot initialize EM with empty dataset"));

    setParameters(params);
}

EM::~EM() {

}

void EM::setParameters(const std::vector<Param> &input) {
    double psum = 0;
    for(int i=0; i<input.size(); i++) {
        psum += input[i].p;
        if(input[i].p <= 0)
            throw(std::runtime_error("Cannot have p <= 0 in parameters"));
        if(input[i].p > 1)
            throw(std::runtime_error("Cannot have p > 1 in parameters"));
        if(input[i].s <= 0)
            throw(std::runtime_error("Cannot have s <= 0 in parameters"));
    }
    if(psum > 1.0000001) {
        throw(std::runtime_error("Initial probabilities sum to greater than 1"));
    }
    params_ = input;
    vector<vector<double> > temp(data_.size(), std::vector<double>(params_.size(),0));
    pikn_ = temp; 
}

std::vector<Param> EM::getParams() const {
    return params_;
}

int EM::getDataSize() const {
    return data_.size();
}

double EM::getLikelihood() const {
    double lambda = 0;
    for(int n=0; n<data_.size(); n++) {
        double sum = 0;
        for(int k=0; k<params_.size(); k++) {
            sum += qkn(k,n);
        }
        lambda += log(sum);
    }
    return lambda;
}

void EM::setMaxSteps(int maxSteps) {
    maxSteps_ = maxSteps;
}

int EM::getMaxSteps() const {
    return maxSteps_;
}

void EM::setTolerance(double tol) {
    tolerance_ = tol;
}

double EM::getTolerance() const {
    return tolerance_;
}

bool EM::run() {
    if(params_.size() == 0) {
        throw(std::runtime_error("EM::run(), parameters are not set"));
    }
    int steps = 0;
    double likelihood = getLikelihood();
    double likelihoodOld;
    // keep an old copy of params
    vector<Param> paramsOld;

    do {
        likelihoodOld = likelihood;
        paramsOld = params_;
        EStep();
        MStep();   
        steps++;
        likelihood = getLikelihood(); 
        // if the likelihood increased, then we revert back to the old params right before
        // we took the step and break;
        // (the likelihood may increase due to convergence/numerical issues, and is
        // an indication of convergence)

        // likelihood decreases normally if a convergence criterion has been reached
        if(likelihood < likelihoodOld) {
            params_ = paramsOld;
            break; 
        }
    // Stop EM if:
    // a. likelihood reaches the specified tolerance
    // b. maxmimum number of steps reached
    } while(likelihood - likelihoodOld > tolerance_ && steps < maxSteps_);

    return (steps < maxSteps_);
}

bool EM::cleanParameters() {
    vector<Param> cleanParams;
    bool changed = false;
    
	// a point n is owned by a component k if it has the highest value
	vector<int> count(params_.size(), 0);

	for(int n=0; n < data_.size(); n++) {
		double best_v = 0;
		double best_k = 0;
		for(int k=0; k < params_.size(); k++) {
			double uk = params_[k].u;
			double sk = params_[k].s;
			double pk = params_[k].p;
			
			// todo - gaussian value not available to generic class, need to subclass
			double value = pk*gaussian(uk,sk,data_[n]);
			if( value > best_v) {
				best_v = value;
				best_k = k;
			}
		}
		count[best_k] += 1;
	}
	
	cout << params_ << endl;

	for(int i=0; i < count.size(); i++) {
		cout << i << ": " << count[i] << endl;
	}


	// given two components, if both the mean and std dev are very similar, then merge using
	// p_n = p1 + p2;
	// u_n = (u1+u2)/2;
	// s_n = sqrt(s1*s2);

	for(int i=0; i < params_.size(); i++) {
        bool keep = true;
		// even ownership implies each component owns approximately N/K of the points.
		if(count[i] < 0.1*(data_.size()/params_.size())) {
			keep = false;
		} else if(params_[i].s < 0.01*domainLength()) {
            keep = false;
        }
        if(keep) {
            cleanParams.push_back(params_[i]);
        } else {
            changed = true;
        }
   } 
    params_ = cleanParams;
    return changed;
}

bool EM::simpleRun(unsigned int numParams) {
    
    if(numParams > data_.size()) {
        throw(std::runtime_error("EM::simpleRun(), numParams > number of data points"));
    }

    // initialize parameters by sampling from the data
    vector<double> randomSample = data_;
    random_shuffle(randomSample.begin(), randomSample.end());
    randomSample.resize(numParams);
    params_.resize(numParams);

    for(int i=0; i < randomSample.size(); i++) {
        params_[i].p = (double) 1 / randomSample.size();
        params_[i].u = randomSample[i];
        params_[i].s = 0.1*domainLength();
    }
    
    // initialize pikn
    for(int i=0; i < data_.size(); i++) {
        pikn_[i].resize(params_.size(), 0);
    }

    int steps = 0;
    double likelihood = getLikelihood();
    double likelihoodOld;

	cout << params_ << endl;

    do {
        vector<Param> paramsOld = params_;

        likelihoodOld = likelihood;
        EStep();
        MStep();   
        steps++;
        likelihood = getLikelihood(); 

        if(steps >= maxSteps_) {
            break;
        }

        if(cleanParameters()) {
            continue;
        }

        if(likelihood - likelihoodOld < tolerance_) {
            if(likelihood < likelihoodOld)
                params_ = paramsOld;
            break;
        }

    // Stop EM if:
    // a. likelihood reaches the specified tolerance
    // b. maximimum number of steps reached
    } while(true);

    return steps < maxSteps_;
}

void EM::EStep() {
    for(int n=0; n<data_.size(); n++) {
        double sum = 0;
        for(int k=0; k<params_.size(); k++) {
            sum += qkn(k,n);
        }

        for(int k=0; k<params_.size(); k++) {
            if(sum > 1e-7)
                pikn_[n][k] = qkn(k,n)/sum;
            else
                pikn_[n][k] = 0;
        }
    }
    testIntegrity();
}

void EM::testIntegrity() const {
    double sum = 0;
    for(int k=0; k<params_.size(); k++) {
        for(int n=0; n<data_.size(); n++) {
            sum += pikn_[n][k];   
        }
        if(fabs(sum-1.0) < 1e-7)
            throw(std::runtime_error("pikn no longer sums to 1"));
    }
};

}
