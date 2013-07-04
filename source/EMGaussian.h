#ifndef EM_GAUSSIAN_H
#define EM_GAUSSIAN_H

#include "EM.h"

// Canonical Expectation Maximization of Gaussian Mixture Models
class EMGaussian : EM {
    public:
        EMGaussian(const std::vector<double> &data, const std::vector<Param> &params);
        ~EMGaussian();

        void MStep();

    private:
        double qkn(int k, int n) const;

};

#endif
