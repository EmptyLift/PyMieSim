#pragma once

#include <vector>
#include <complex>
#include "utils/base_mesh.h"

typedef std::complex<double> complex128;

class FullSteradian : public BaseMesh
{
    public:
        double dTheta, dPhi;

        FullSteradian(const size_t sampling, const double radius);

        template<typename dtype> dtype get_integral(std::vector<dtype>& vector) const;

        template<typename dtype> dtype get_cos_integral(const std::vector<dtype>& vector) const;

        double get_integral() const;

};

// --
