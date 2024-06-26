#include <pybind11/pybind11.h>
#include <pybind11/complex.h> // Ensure complex numbers are supported
#include "cylinder.cpp"

namespace py = pybind11;
using namespace CYLINDER;

PYBIND11_MODULE(CylinderInterface, module) {
    module.doc() = "Lorenz-Mie Theory (LMT) C++ binding module for PyMieSim Python package.";

    py::class_<Scatterer>(module, "CYLINDER")
        .def(py::init<double, double, double, std::complex<double>, double, std::vector<complex128>>(),
             py::arg("wavelength"),
             py::arg("amplitude"),
             py::arg("diameter"),
             py::arg("index"),
             py::arg("medium_index"),
             py::arg("jones_vector"),
             "Constructor for CYLINDER, initializing it with physical and optical properties.")

        .def("get_s1s2", &Scatterer::get_s1s2_py, py::arg("phi"), "Calculates and returns the S1 and S2 scattering parameters for a cylinder.")
        .def("get_fields", &Scatterer::get_unstructured_fields_py, py::arg("phi"), py::arg("theta"), py::arg("r"), "Returns the unstructured electromagnetic fields around the cylinder.")
        .def("get_full_fields", &Scatterer::get_full_structured_fields_py, py::arg("sampling"), py::arg("r"), "Returns the full structured electromagnetic fields around the sphere.")
        .def("a1n", py::overload_cast<>(&Scatterer::get_a1n_py), "Returns the a1n coefficient.")
        .def("b1n", py::overload_cast<>(&Scatterer::get_b1n_py), "Returns the b1n coefficient.")
        .def("a2n", py::overload_cast<>(&Scatterer::get_a2n_py), "Returns the a2n coefficient.")
        .def("b2n", py::overload_cast<>(&Scatterer::get_b2n_py), "Returns the b2n coefficient.")
        .def_property_readonly("Qsca", &Scatterer::get_Qsca, "Scattering efficiency of the cylinder.")
        .def_property_readonly("Qext", &Scatterer::get_Qext, "Extinction efficiency of the cylinder.")
        .def_property_readonly("Qabs", &Scatterer::get_Qabs, "Absorption efficiency of the cylinder.")
        .def_property_readonly("Qback", &Scatterer::get_Qback, "Backscattering efficiency of the cylinder.")
        .def_property_readonly("Qpr", &Scatterer::get_Qpr, "Radiation pressure efficiency of the cylinder.")
        .def_property_readonly("Csca", &Scatterer::get_Csca, "Scattering cross-section of the cylinder.")
        .def_property_readonly("Cext", &Scatterer::get_Cext, "Extinction cross-section of the cylinder.")
        .def_property_readonly("Cabs", &Scatterer::get_Cabs, "Absorption cross-section of the cylinder.")
        .def_property_readonly("Cback", &Scatterer::get_Cback, "Backscattering cross-section of the cylinder.")
        .def_property_readonly("Cpr", &Scatterer::get_Cpr, "Radiation pressure cross-section of the cylinder.")
        .def_property_readonly("g", &Scatterer::get_g, "Asymmetry parameter of the cylinder.")
        .def_readwrite("area", &Scatterer::area, "Physical cross-sectional area of the cylinder.")
        .def_readwrite("size_parameter", &Scatterer::size_parameter, "Size parameter of the cylinder scatterer.");
}
