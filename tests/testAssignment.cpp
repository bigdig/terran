#include <math.h>
#include <vector>
#include <iostream>

#include <EMGaussian.h>
#include <EMPeriodicGaussian.h>
#include <PartitionPeriodicGaussian.h>
#include <MathFunctions.h>

#include "util.h"

using namespace std;

// A three-cluster dataset looking something like this:

// PI
// |       *
// |      ***
// |     **0**
// |      ***     *
// |       *     ***
// |            **2**
// |       *     ***
// |      ***     *
// |     **1**
// |      ***
// |       *
//-PI-------------------PI


void testEasyCase2D() {

    vector<vector<double> > dataset;
    // setup 0th cluster;
    {
        double u1 = -PI/2;
        double u2 =  PI/2;
        double s = 0.3;
        double period = 2*PI;
        for(int i=0; i < 2000; i++) {
            vector<double> point(2);
            point[0] = periodicGaussianSample(u1, s, period);
            point[1] = periodicGaussianSample(u2, s, period);
            dataset.push_back(point);
        }
    }

    // setup 1st cluster
    {
        double u1 = -PI/2;
        double u2 = -PI/2;
        double s = 0.3;
        double period = 2*PI;
        for(int i=0; i < 2000; i++) {
            vector<double> point(2);
            point[0] = periodicGaussianSample(u1, s, period);
            point[1] = periodicGaussianSample(u2, s, period);
            dataset.push_back(point);
        }
    }

    // setup 2nd cluster
    {
        double u1 = PI/4;
        double u2 = 0;
        double s = 0.3;
        double period = 2*PI;
        for(int i=0; i < 2000; i++) {
            vector<double> point(2);
            point[0] = periodicGaussianSample(u1, s, period);
            point[1] = periodicGaussianSample(u2, s, period);
            dataset.push_back(point);
        }
    }

    // First dimension
    vector<vector<double> > intervals;
    {
        vector<double> d0;
        for(int i=0; i < dataset.size(); i++) {
            d0.push_back(dataset[i][0]);
        }

        vector<Param> params;
        {
            Param p;
            p.p = 0.5;
            p.u = -0.3;
            p.s = 0.3;
            params.push_back(p);
        }
        {
            Param p;
            p.p = 0.5;
            p.u = 0.3;
            p.s = 0.3;
            params.push_back(p);
        }
        EMPeriodicGaussian em(d0, params, 2*PI, 10);
        em.run();
        vector<Param> p = em.getParams();
        PartitionPeriodicGaussian ppg(p, 2*PI, 10);
        intervals.push_back(ppg.partition(0.05));
        cout << p[0].p << " " << p[0].u << " " << p[0].s << endl;
        cout << p[1].p << " " << p[1].u << " " << p[1].s << endl;
    }
    {
        vector<double> d1;
        for(int i=0; i < dataset.size(); i++) {
            d1.push_back(dataset[i][1]);
        }

        vector<Param> params;
        {
            Param p;
            p.p = 0.33;
            p.u = -0.3;
            p.s = 0.3;
            params.push_back(p);
        }
        {
            Param p;
            p.p = 0.33;
            p.u = 0;
            p.s = 0.3;
            params.push_back(p);
        }
        {
            Param p;
            p.p = 0.33;
            p.u = 0.3;
            p.s = 0.3;
            params.push_back(p);
        }
        EMPeriodicGaussian em(d1, params, 2*PI, 10);
        em.run();
        vector<Param> p = em.getParams();
        PartitionPeriodicGaussian ppg(p, 2*PI, 10);
        intervals.push_back(ppg.partition(0.05));
        cout << p[0].p << " " << p[0].u << " " << p[0].s << endl;
        cout << p[1].p << " " << p[1].u << " " << p[1].s << endl;
        cout << p[2].p << " " << p[2].u << " " << p[2].s << endl;
    }

    // Partitions
    for(int i=0; i<intervals.size(); i++) {
        for(int j=0; j<intervals[j].size(); j++) {
            cout << intervals[i][j] << " ";
        }
        cout << endl;
    }

    // uncomment if visualization is desired, you can 
    // plot the output using xmgrace directly using:
    // ./testAssignment > data.txt; xmgrace data.txt
    /*
    for(int i=0; i < dataset.size(); i++) {
        cout << dataset[i][0] << " " << dataset[i][1] << endl;
    }
    */

}

int main() {
    testEasyCase2D();

}
