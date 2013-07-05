#ifndef EM_PERIODIC_GAUSSIAN_H
#define EM_PERIODIC_GAUSSIAN_H

#include "EM.h"
#include "MathFunctions.h"

// Expectation Maximization of Periodic Gaussian Mixture Models
class EMPeriodicGaussian : public EM {
    public:
        // period and images are initialized by default
        EMPeriodicGaussian(const std::vector<double> &data, const std::vector<Param> &params, double period = 2*PI, int images = 30);
        ~EMPeriodicGaussian();

        void MStep();
    private:
        double qkn(int k, int n) const; 

        // Simplified versions of the lambda derivative in order to find roots
        double dlds(double sk, int k) const;
        double dldu(double uk, int k) const;

        double period_;
        int images_;
};
#endif