#include "findIntervals.h"

#include <vector>

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <assert.h>
using namespace std;

void partitionGaussian(const vector<Param> &params) {


}

// Maximas can be found by using the mean as initial guesses. In the case of overlapping components
// They should converge to the same point. Unlike minimas, they don't have pathological zero derivatives
vector<double> findPeriodicMaxima(const vector<Param> &params, double period, int images) {
    vector<double> maximas;
    for(int k=0; k<params.size(); k++) {
        double xn_old = params[k].u;
        bool found = false;
        const double delta = 1e-4;
        int iteration = 0;
        while(!found) {
            iteration++;
            if(iteration >= 1e6)
                throw(std::runtime_error("findPeriodicMaxima: maximum iteration count reached!"));
            
            double xn_new = xn_old + delta*periodicGaussianMixtureDx(params, xn_old, period, images);
            if(fabs(xn_new - xn_old) < 1e-8)
                found = true;

            xn_old = xn_new;
        }


        bool skip = false;
        for(int i=0; i<maximas.size(); i++)
            if(fabs(xn_old-maximas[i]) < 1e-7)
                skip = true;
        if(!skip) 
            maximas.push_back(xn_old);
    }
    return maximas;
}

// Algorithm 1.

// 1. Find all the maximas
// 2. Sort the maximas according to their x value
// 3. Find minimas by taking adjacent pair of maximas and doing either
//    bisection or gradient descent to converge at a minima
vector<double> findPeriodicMinima(const vector<Param> &params, double period, int images) {

    double ak = -1.2;
    double bk = 0.5;
    double mk;
    double my;
    do {
        assert(periodicGaussianMixtureDx(params, ak, period, images) < 0);
        assert(periodicGaussianMixtureDx(params, bk, period, images) > 0);
        mk = (ak+bk)/2.0;
        my = periodicGaussianMixtureDx(params, mk, period, images);
        if(my < 0)
            ak = mk;
        else
            bk = mk;
       cout << ak << " " << mk << " " << bk << endl;
    } while(fabs(my) > 1e-10);


    return vector<double>(0);
}

void partitionPeriodicGaussian(const vector<Param> &params, double period, int images) {
    ofstream mixture("mixture.dat");
    for(double xn = -period/2; xn < period/2; xn += 0.01) {
        mixture << xn << " " << periodicGaussianMixture(params, xn, period, images) << endl;
    }
    ofstream mixtureDx("mixtureDx.dat");
    for(double xn = -period/2; xn < period/2; xn += 0.01) {
        mixtureDx << xn << " " << periodicGaussianMixtureDx(params, xn, period, images) << endl;
    }
}
