#include "numpy_interface.cpp"
#include <vector>
#include <cmath>
#include "bessel_subroutine.h"

#define PI (double)3.14159265358979323846264338
typedef std::complex<double> complex128;


std::vector<complex128> get_LP_mode_field(
   std::vector<double> x_coords,
   std::vector<double> y_coords,
   int azimuthal_number,
   int radial_number)
{
   std::vector<complex128> field(x_coords.size());

   // Normalize the coordinates
   double max_norm = 0.0;
   for (size_t i = 0; i < x_coords.size(); ++i) {
      double norm = std::sqrt(x_coords[i] * x_coords[i] + y_coords[i] * y_coords[i]);
      if (norm > max_norm)
         max_norm = norm;

   }

   if (max_norm != 0) {  // Avoid division by zero
      for (size_t i = 0; i < x_coords.size(); ++i) {
         x_coords[i] /= max_norm;
         y_coords[i] /= max_norm;
      }
   }

   double rj0[radial_number], rj1[radial_number], ry0[radial_number], ry1[radial_number];

   bessel_zeros(azimuthal_number, radial_number, rj0, rj1, ry0, ry1);

   for (size_t i = 0; i < x_coords.size(); ++i)
   {
      double x = x_coords[i];
      double y = y_coords[i];

      double r = std::sqrt(x * x + y * y);
      double phi = std::atan2(y, x);

      complex128 radial_part = bessel_J(azimuthal_number, r * rj0[radial_number-1]);

      double azimuthal_part = std::cos(azimuthal_number * phi);

      field[i] = radial_part * azimuthal_part;
   }

   // Normalization to L2 norm of 1
   double norm = 0.0;
   for (const complex128& f : field)
      norm += std::norm(f);

   norm = std::sqrt(norm);

   for (complex128& f : field)
      f /= norm;

   return field;
}

pybind11::array_t<complex128> get_LP_mode_field_py(
   std::vector<double> x_coords,
   std::vector<double> y_coords,
   int azimuthal_number,
   int radial_number)
{
   return vector_to_numpy(
      get_LP_mode_field(x_coords, y_coords, azimuthal_number, radial_number),
      {x_coords.size()}
   );
}



// -