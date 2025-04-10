#pragma once

#include <vector>
#include "source/source.h"
#include "utils/numpy_interface.h"
#include "fibonacci/fibonacci.h"
#include "full_mesh/full_mesh.h"

typedef std::complex<double> complex128;


class BaseScatterer {
public:
    size_t max_order;
    BaseSource source;
    double size_parameter;
    double size_parameter_squared;
    double area;
    double medium_refractive_index;
    std::vector<size_t> indices;

    BaseScatterer() = default;
    virtual ~BaseScatterer() = default;

    virtual std::tuple<std::vector<complex128>, std::vector<complex128>> compute_s1s2(const std::vector<double> &phi) const = 0;
    virtual double get_Qsca() const = 0;
    virtual double get_Qext() const = 0;
    virtual double get_g() const = 0;
    virtual void compute_size_parameter() = 0;
    virtual void compute_area() = 0;

    double get_Qabs() const {return get_Qext() - get_Qsca();};
    double get_Csca() const {return get_Qsca() * area;};
    double get_Cext() const {return get_Qext() * area;};
    double get_Cabs() const {return get_Qabs() * area;};
    double get_Qpr() const {return get_Qext() - get_g() * get_Qsca();};
    double get_Cpr() const {return get_Qpr() * area;};


    BaseScatterer(const size_t max_order, const BaseSource &source, const double medium_refractive_index)
     : max_order(max_order), source(source), medium_refractive_index(medium_refractive_index)
    {}

    std::vector<double> get_prefactor() const {
        std::vector<double> output;

        output.reserve(max_order);

        for (size_t m = 0; m < max_order ; ++m)
            output[m] = (double) ( 2 * (m + 1) + 1 ) / ( (m + 1) * ( (m + 1) + 1 ) );

        return output;
    }

    complex128 compute_propagator(const double &radius) const
    {
        return source.amplitude / (source.wavenumber * radius) * exp(-complex128(0, 1) * source.wavenumber * radius);
    }

    size_t get_wiscombe_criterion(const double size_parameter) const {
        return static_cast<size_t>(2 + size_parameter + 4 * std::cbrt(size_parameter)) + 16;
    }

    std::tuple<std::vector<complex128>, std::vector<complex128>>
    compute_structured_fields(const std::vector<complex128>& S1, const std::vector<complex128>& S2, const std::vector<double>& theta, const double& radius = 1.) const {
        std::vector<complex128> phi_field, theta_field;

        size_t full_size = theta.size() * S1.size();

        phi_field.reserve(full_size);
        theta_field.reserve(full_size);

        complex128 propagator = this->compute_propagator(radius);

        for (unsigned int p=0; p < S1.size(); p++ )
            for (unsigned int t=0; t < theta.size(); t++ )
            {
                complex128 phi_point_field = propagator * S1[p] * (source.jones_vector[0] * cos(theta[t]) + source.jones_vector[1] * sin(theta[t]));
                complex128 thetea_point_field = propagator * S2[p] * (source.jones_vector[0] * sin(theta[t]) - source.jones_vector[1] * cos(theta[t]));

                phi_field.push_back(phi_point_field);
                theta_field.push_back(thetea_point_field);
            }

        return std::make_tuple(phi_field, theta_field);
    }

    std::tuple<std::vector<complex128>, std::vector<complex128>>
    compute_structured_fields(const std::vector<double>& phi, const std::vector<double>& theta, const double radius) const {
        auto [S1, S2] = this->compute_s1s2(phi);

        return this->compute_structured_fields(S1, S2, theta, radius);
    }

    std::tuple<std::vector<complex128>, std::vector<complex128>>
    compute_unstructured_fields(const std::vector<double>& phi, const std::vector<double>& theta, const double radius) const
    {
        auto [S1, S2] = this->compute_s1s2(phi);

        std::vector<complex128> phi_field, theta_field;

        size_t full_size = theta.size();

        phi_field.reserve(full_size);
        theta_field.reserve(full_size);

        complex128 propagator = this->compute_propagator(radius);

        for (unsigned int idx=0; idx < full_size; idx++)
        {
            complex128 phi_field_point = propagator * S1[idx] * (source.jones_vector[0] * cos(theta[idx]) + source.jones_vector[1] * sin(theta[idx])),
            theta_field_point = propagator * S2[idx] * (source.jones_vector[0] * sin(theta[idx]) - source.jones_vector[1] * cos(theta[idx]));

            phi_field.push_back(phi_field_point);
            theta_field.push_back(theta_field_point);
        }

        return std::make_tuple(phi_field, theta_field);
    }

    std::tuple<std::vector<complex128>, std::vector<complex128>>
    compute_unstructured_fields(const FibonacciMesh& fibonacci_mesh, const double radius) const
    {
        return this->compute_unstructured_fields(
            fibonacci_mesh.spherical_coordinates.phi,
            fibonacci_mesh.spherical_coordinates.theta,
            radius
        );
    }

    std::tuple<std::vector<complex128>, std::vector<complex128>, std::vector<double>, std::vector<double>>
    compute_full_structured_fields(const size_t& sampling, const double& radius) const
    {
        FullSteradian full_mesh = FullSteradian(sampling, radius);

        auto [S1, S2] = this->compute_s1s2(full_mesh.spherical_coordinates.phi);

        auto [phi_field, theta_field] = this->compute_structured_fields(S1, S2, full_mesh.spherical_coordinates.theta, radius);

        return std::make_tuple(
            std::move(phi_field),
            std::move(theta_field),
            std::move(full_mesh.spherical_coordinates.phi),
            std::move(full_mesh.spherical_coordinates.theta)
        );
    }

    std::tuple<py::array_t<complex128>, py::array_t<complex128>>
    get_unstructured_fields_py(const std::vector<double>& phi, const std::vector<double>& theta, const double radius) const
    {
        auto [theta_field, phi_field] = this->compute_unstructured_fields(phi, theta, radius);

        return std::make_tuple(
            _vector_to_numpy(phi_field, {phi_field.size()}),
            _vector_to_numpy(theta_field, {theta_field.size()})
        );
    }

    double get_g_with_fields(size_t sampling) const {
        auto [SPF, fibonacci_mesh] = this->compute_full_structured_spf(sampling);

        double
            norm = abs(fibonacci_mesh.get_integral(SPF)),
            expected_cos = abs(fibonacci_mesh.get_cos_integral(SPF));

        return expected_cos/norm;
    }

    //--------------------------------------------------------PYTHON-------------------
    std::tuple<py::array_t<complex128>, py::array_t<complex128>>
    get_s1s2_py(const std::vector<double> &phi) const
    {
        auto [S1, S2] = this->compute_s1s2(phi);

        return std::make_tuple(
            _vector_to_numpy(S1, {S1.size()}),
            _vector_to_numpy(S2, {S2.size()})
        );
    }

    std::tuple<py::array_t<complex128>, py::array_t<complex128>, py::array_t<double>, py::array_t<double>>
    get_full_structured_fields_py(size_t &sampling, double& distance) const {
        auto [phi_field, theta_field, theta, phi] = this->compute_full_structured_fields(sampling, distance);

        py::array_t<complex128>
            phi_field_py = _vector_to_numpy(phi_field, {sampling, sampling}),
            theta_field_py = _vector_to_numpy(theta_field, {sampling, sampling});

        py::array_t<double>
            theta_py = _vector_to_numpy(theta, {theta.size()}),
            phi_py = _vector_to_numpy(phi, {phi.size()});

        phi_field_py = phi_field_py.attr("transpose")();
        theta_field_py = theta_field_py.attr("transpose")();

        return std::make_tuple(phi_field_py, theta_field_py, phi_py, theta_py);
    }

    std::tuple<std::vector<complex128>, FullSteradian>
    compute_full_structured_spf(const size_t sampling, const double radius = 1.0) const
    {
        FullSteradian full_mesh = FullSteradian(sampling, radius);

        auto [phi_field, theta_field] = this->compute_structured_fields(
            full_mesh.spherical_coordinates.phi,
            full_mesh.spherical_coordinates.theta,
            radius
        );

        square_vector(phi_field);
        square_vector(theta_field);

        std::vector<complex128> spf = add_vectors(phi_field, theta_field);

        return std::make_tuple(std::move(spf), std::move(full_mesh));
    }

    template <class T> std::vector<T> add_vectors(std::vector<T>& vector0, std::vector<T>& vector1) const
    {
        std::vector<T> output_vector;
        output_vector.reserve( vector0.size() );

        for (size_t iter=0; iter<vector0.size(); iter++)
            output_vector.push_back( vector0[iter] + vector1[iter] );

        return output_vector;
    }

    template <class T> void square_vector(std::vector<T>& vector) const
    {
        for (size_t iter=0; iter<vector.size(); iter++)
            vector[iter] = pow(abs(vector[iter]), 2);
    }

};