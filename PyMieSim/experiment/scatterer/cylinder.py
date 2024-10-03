#!/usr/bin/env python
# -*- coding: utf-8 -*-

from pydantic.dataclasses import dataclass
from pydantic import field_validator
from typing import List, Optional

import numpy
from PyMieSim.binary.SetsInterface import CppCylinderSet
from PyOptik.base_class import BaseMaterial
from PyMieSim.units import Quantity, meter
from PyMieSim.experiment.scatterer.base_class import BaseScatterer, config_dict


@dataclass(config=config_dict, kw_only=True)
class Cylinder(BaseScatterer):
    """
    Represents a cylindrical scatterer configuration for PyMieSim simulations.

    Attributes:
        source (PyMieSim.experiment.source.base.BaseSource): Light source configuration for the simulation.
        diameter (Quantity): Diameter(s) of the cylinder in meters.
        property (List[BaseMaterial] | List[Quantity]): Refractive index or indices of the spherical scatterers themselves.
        medium_property (List[BaseMaterial] | List[Quantity]): BaseMaterial(s) defining the medium, used if `medium_index` is not provided.
    """
    diameter: Quantity
    property: List[BaseMaterial] | List[Quantity]

    available_measure_list = [
        'Qsca', 'Qext', 'Qabs', 'Csca', 'Cext', 'Cabs', 'a11',
        'a21', 'a12', 'a22', 'a13', 'a23', 'b11', 'b21', 'b12',
        'b22', 'b13', 'b23', 'coupling',
    ]

    @field_validator('diameter', 'medium_index', 'medium_material', 'index', 'material', mode='before')
    def validate_array(cls, value):
        """Ensure that arrays are properly converted to numpy arrays."""
        if not isinstance(value, numpy.ndarray):
            value = numpy.atleast_1d(value)

        return value

    @field_validator('diameter', mode='before')
    def validate_length_quantity(cls, value):
        """
        Ensures that diameter is Quantity objects with length units."""
        if not isinstance(value, Quantity):
            raise ValueError(f"{value} must be a Quantity with meters units.")

        if not value.check(meter):
            raise ValueError(f"{value} must have length units (meters).")

        return numpy.atleast_1d(value)

    def __post_init__(self) -> None:
        """
        Constructs the keyword arguments necessary for the C++ binding interface, specifically tailored for spherical scatterers.
        This includes processing material indices and organizing them into a structured dictionary for simulation interaction.

        This method automatically configures the `binding_kwargs` attribute with appropriately formatted values.
        """
        self.mapping = {}

        self.binding_kwargs = dict(diameter=self.diameter)

        self._assign_index_or_material(element='', property=self.property)
        self._assign_index_or_material(element='medium_', property=self.medium_property)

        binding_kwargs = {
            k: v.to_base_units().magnitude if isinstance(v, Quantity) else v for k, v in self.binding_kwargs.items()
        }

        self.binding = CppCylinderSet(**binding_kwargs)

# -