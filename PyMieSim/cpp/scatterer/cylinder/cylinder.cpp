#include "scatterer/cylinder/cylinder.h"

// ---------------------- Constructors ---------------------------------------
Cylinder::Cylinder(double diameter, complex128 refractive_index, double medium_refractive_index, const BaseSource &source, size_t max_order) :
BaseScatterer(max_order, source, medium_refractive_index), diameter(diameter), refractive_index(refractive_index)
{
    this->compute_cross_section();
    this->compute_size_parameter();
    this->max_order = (max_order == 0) ? this->get_wiscombe_criterion(this->size_parameter) : max_order;
    this->compute_an_bn(this->max_order);
}

// ---------------------- Methods ---------------------------------------

void Cylinder::compute_size_parameter() {
    this->size_parameter = source.wavenumber * this->diameter / 2;
    this->size_parameter_squared = pow(this->size_parameter, 2);
}

void Cylinder::compute_cross_section() {
    this->cross_section = this->diameter;
}

double Cylinder::get_Qsca() const {
    complex128 Qsca1=0, Qsca2=0;

    for(size_t order = 1; order < max_order; order++)
    {
        Qsca1 +=  pow( std::abs(this->a1n[order]), 2 ) + pow( std::abs(this->b1n[order]), 2 ) ;
        Qsca2 +=  pow( std::abs(this->a2n[order]), 2 ) + pow( std::abs(this->b2n[order]), 2 ) ;
    }

    Qsca1 =  2. / size_parameter * ( 2.0 * Qsca1 + pow( std::abs(this->b1n[0]), 2 ) );
    Qsca2 =  2. / size_parameter * ( 2.0 * Qsca2 + pow( std::abs(this->a2n[0]), 2 ) );

    return process_polarization(Qsca1, Qsca2);
}

double Cylinder::get_Qext() const {
    complex128 Qext1 = 0, Qext2 = 0;

    for(size_t it = 1; it < max_order; ++it){
        Qext1 += this->b1n[it];
        Qext2 += this->a2n[it];
    }

    Qext1 = 2. / size_parameter * std::real( this->b1n[0] + 2.0 * Qext1 );
    Qext2 = 2. / size_parameter * std::real( this->a1n[0] + 2.0 * Qext2 );

    return this->process_polarization(Qext1, Qext2);
}

double Cylinder::get_g() const {
    return this->get_g_with_fields(1000);
}

double Cylinder::process_polarization(const complex128 value_0, const complex128 value_1) const {
    return std::abs( value_1 ) * pow(std::abs(source.jones_vector[0]), 2) + std::abs( value_0 ) * pow(std::abs(source.jones_vector[1]), 2);
}

void Cylinder::compute_an_bn(const size_t max_order) {
    // Resize vectors to hold Mie coefficients for the specified maximum order
    this->a1n.resize(max_order);
    this->b1n.resize(max_order);
    this->a2n.resize(max_order);
    this->b2n.resize(max_order);

    // Compute the arguments used in the Bessel and Hankel function calculations
    complex128
        m = this->refractive_index / this->medium_refractive_index, // Relative refractive index
        mx = m * size_parameter; // Scaled size parameter for internal calculations

    // Precompute Bessel and Hankel functions up to max_order
    std::vector<complex128>
        J_z(max_order + 1),
        J_z_p(max_order + 1),
        J_x(max_order + 1),
        J_x_p(max_order + 1),
        H_x(max_order + 1),
        H_x_p(max_order + 1);

    for (size_t order = 0; order < max_order + 1; ++order){
        J_z[order] = compute_Jn(order, mx);
        J_z_p[order] = compute_Jn_p(order, mx);
        J_x[order] = compute_Jn(order, size_parameter);
        J_x_p[order] = compute_Jn_p(order, size_parameter);
        H_x[order] = compute_H1(order, size_parameter);
        H_x_p[order] = compute_H1_p(order, size_parameter);
    }

    // Compute Mie coefficients a1n, a2n, b1n, b2n for each order
    for (size_t order = 0; order < max_order; order++){
        complex128 numerator_a = m * J_z[order] * J_x_p[order] - J_z_p[order] * J_x[order];
        complex128 denominator_a = m * J_z[order] * H_x_p[order] - J_z_p[order] * H_x[order];
        this->a1n[order] = 0.0 ;
        this->a2n[order] = numerator_a / denominator_a;

        complex128 numerator_b = J_z[order] * J_x_p[order] - m * J_z_p[order] * J_x[order];
        complex128 denominator_b = J_z[order] * H_x_p[order] - m * J_z_p[order] * H_x[order];
        this->b1n[order] = numerator_b / denominator_b;
        this->b2n[order] = 0.0 ;
    }
}

std::tuple<std::vector<complex128>, std::vector<complex128>>
Cylinder::compute_s1s2(const std::vector<double> &phi) const{
    std::vector<complex128> T1(phi.size()), T2(phi.size());

    for (size_t i = 0; i < phi.size(); i++){
        T1[i] = this->b1n[0];
        T2[i] = this->a2n[0];
        for (size_t order = 1; order < max_order ; order++){
            T1[i] += 2.0 * this->b1n[order] * cos(order * (PI - (phi[i] + PI/2.0) ) );
            T2[i] += 2.0 * this->a2n[order] * cos(order * (PI - (phi[i] + PI/2.0) ) );
        }
    }

    return std::make_tuple(std::move(T1), std::move(T2));
}


std::vector<complex128>
Cylinder::compute_dn(double nmx, complex128 z) const { //Page 205 of BH
    std::vector<complex128> Dn(nmx, 0.0);

    for (double n = nmx - 1; n > 0; n--)
        Dn[n-1] = n/z - ( 1. / (Dn[n] + n/z) );

    return Dn;
}