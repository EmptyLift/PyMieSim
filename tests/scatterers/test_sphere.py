import pytest
from PyMieSim.single.scatterer import Sphere
from PyMieSim.single.source import Gaussian
from PyMieSim.single.detector import Photodiode
from PyOptik import UsualMaterial

# Define the core configurations for testing, now separated 'id' for clarity in tests
core_configs = [
    {'config': {'material': UsualMaterial.BK7}, 'id': 'BK7'},
    {'config': {'material': UsualMaterial.Silver}, 'id': 'Silver'},
    {'config': {'index': 1.4}, 'id': 'Index 1.4'}
]

methods = ["an", "bn"]

attributes = [
    "size_parameter", "area", "Qsca", "Qext", "Qback", "Qratio",
    "Qpr", "Csca", "Cext", "Cback", "Cratio", "Cpr",
]

plottings = [
    "get_far_field", "get_stokes", "get_spf", "get_s1s2",
]


@pytest.mark.parametrize('core_config', core_configs, ids=[config['id'] for config in core_configs])
@pytest.mark.parametrize('method', methods)
def test_sphere_method(method, core_config):
    source = Gaussian(
        wavelength=750e-9,
        polarization_value=0,
        polarization_type='linear',
        optical_power=1,
        NA=0.3
    )
    # Pass only the actual configuration dictionary to the constructor
    scatterer = Sphere(
        diameter=100e-9,
        source=source,
        medium_index=1.0,
        **core_config['config']
    )
    _ = getattr(scatterer, method)()


@pytest.mark.parametrize('core_config', core_configs, ids=[config['id'] for config in core_configs])
@pytest.mark.parametrize('attribute', attributes)
def test_sphere_attribute(attribute, core_config):
    source = Gaussian(
        wavelength=750e-9,
        polarization_value=0,
        polarization_type='linear',
        optical_power=1,
        NA=0.3
    )
    scatterer = Sphere(
        diameter=100e-9,
        source=source,
        medium_index=1.0,
        **core_config['config']
    )
    _ = getattr(scatterer, attribute)


@pytest.mark.parametrize('core_config', core_configs, ids=[config['id'] for config in core_configs])
def test_sphere_coupling(core_config):
    detector = Photodiode(
        NA=0.2,
        gamma_offset=0,
        phi_offset=0,
    )
    source = Gaussian(
        wavelength=750e-9,
        polarization_value=0,
        polarization_type='linear',
        optical_power=1,
        NA=0.3
    )
    scatterer = Sphere(
        diameter=100e-9,
        source=source,
        medium_index=1.0,
        **core_config['config']
    )
    _ = detector.coupling(scatterer)


@pytest.mark.parametrize('core_config', core_configs, ids=[config['id'] for config in core_configs])
@pytest.mark.parametrize('plotting', plottings)
def test_sphere_plottings(plotting, core_config):
    source = Gaussian(
        wavelength=750e-9,
        polarization_value=0,
        polarization_type='linear',
        optical_power=1,
        NA=0.3
    )
    scatterer = Sphere(
        diameter=100e-9,
        source=source,
        medium_index=1.0,
        **core_config['config']
    )
    data = getattr(scatterer, plotting)()
    assert data is not None, "Plotting data should not be None"
