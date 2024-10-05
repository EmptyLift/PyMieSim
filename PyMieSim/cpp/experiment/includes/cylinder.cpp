#pragma once

#include "single/includes/cylinder.cpp"

template<typename dtype, typename Function>
pybind11::array_t<dtype> Experiment::get_cylinder_data(Function function) const
{
    using namespace CYLINDER;

    std::vector<size_t> array_shape = concatenate_vector(sourceSet.shape, cylinderSet.shape);

    size_t full_size = get_vector_sigma(array_shape);

    std::vector<dtype> output_array;
    output_array.reserve(full_size);

    #pragma omp parallel for collapse(7)
    for (size_t wl=0; wl<array_shape[0]; ++wl)
    for (size_t jv=0; jv<array_shape[1]; ++jv)
    for (size_t na=0; na<array_shape[2]; ++na)
    for (size_t op=0; op<array_shape[3]; ++op)
    for (size_t sd=0; sd<array_shape[4]; ++sd)
    for (size_t si=0; si<array_shape[5]; ++si)
    for (size_t mi=0; mi<array_shape[6]; ++mi)
    {
        SOURCE::Gaussian source = sourceSet.to_object(wl, jv, na, op);

        CYLINDER::Scatterer scatterer = cylinderSet.to_object(sd, si, wl, mi, source);

        output_array.emplace_back((scatterer.*function)());
    }

    return vector_to_numpy(output_array, array_shape);
}

pybind11::array_t<double> Experiment::get_cylinder_coupling() const
{
    using namespace CYLINDER;

    std::vector<size_t> array_shape = concatenate_vector(
        sourceSet.shape,
        cylinderSet.shape,
        detectorSet.shape
    );

    size_t full_size = get_vector_sigma(array_shape);

    std::vector<double> output_array;
    output_array.reserve(full_size);

    #pragma omp parallel for collapse(14)
    for (size_t wl=0; wl<array_shape[0]; ++wl)
    for (size_t jv=0; jv<array_shape[1]; ++jv)
    for (size_t na_=0; na_<array_shape[2]; ++na_)
    for (size_t op=0; op<array_shape[3]; ++op)
    for (size_t sd=0; sd<array_shape[4]; ++sd)
    for (size_t si=0; si<array_shape[5]; ++si)
    for (size_t mi=0; mi<array_shape[6]; ++mi)
    for (size_t mn=0; mn<array_shape[7]; ++mn)
    for (size_t fs=0; fs<array_shape[8]; ++fs)
    for (size_t ra=0; ra<array_shape[9]; ++ra)
    for (size_t na=0; na<array_shape[10]; ++na)
    for (size_t po=0; po<array_shape[11]; ++po)
    for (size_t go=0; go<array_shape[12]; ++go)
    for (size_t pf=0; pf<array_shape[13]; ++pf)
    {
        SOURCE::Gaussian source = sourceSet.to_object(wl, jv, na_, op);

        DETECTOR::Detector detector = detectorSet.to_object(mn, fs, ra, na, po, go, pf);

        CYLINDER::Scatterer scatterer = cylinderSet.to_object(sd, si, wl, mi, source);

        output_array.emplace_back(detector.get_coupling(scatterer));
    }

    return vector_to_numpy(output_array, array_shape);
}