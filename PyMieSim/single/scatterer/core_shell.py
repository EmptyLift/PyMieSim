#!/usr/bin/env python
# -*- coding: utf-8 -*-

from PyOptik.material.base_class import BaseMaterial

import numpy
from typing import Optional
from pydantic.dataclasses import dataclass
from pydantic import field_validator
from PyMieSim.units import Quantity, meter, RIU
from PyMieSim.single.scatterer.base import BaseScatterer, config_dict



@dataclass(config=config_dict, kw_only=True)
class CoreShell(BaseScatterer):
    """
    Class representing a core/shell spherical scatterer.

    Parameters
    ----------
    core_diameter : Quantity
        Diameter of the core of the scatterer, in meters.
    shell_width : Quantity
        Width of the shell surrounding the core, in meters.
    core_property : Quantity | BaseMaterial
        Defines either the refractive index (`Quantity`) or material (`BaseMaterial`) of the scatterer's core. Only one can be provided.
    shell_property : Quantity | BaseMaterial
        Defines either the refractive index (`Quantity`) or material (`BaseMaterial`) of the scatterer's shell. Only one can be provided.

    """

    core_diameter: Quantity
    shell_width: Quantity

    core_property: Quantity | BaseMaterial
    shell_property: Quantity | BaseMaterial

    property_names = [
        "size_parameter", "area", "g",
        "Qsca", "Qext", "Qabs", "Qback", "Qratio", "Qpr",
        "Csca", "Cext", "Cabs", "Cback", "Cratio", "Cpr"
    ]

    def __post_init__(self):
        self.core_index, self.core_material = self._assign_index_or_material(self.core_property)

        self.shell_index, self.shell_material = self._assign_index_or_material(self.core_property)

        self.cross_section = numpy.pi * ((self.core_diameter / 2 + self.shell_width)) ** 2

        super().__post_init__()

    def set_binding(self) -> None:
        """
        Bind the C++ scatterer class.
        """
        from PyMieSim.binary.CoreShellInterface import CORESHELL

        self.binding = CORESHELL(
            shell_index=self.shell_index.to_base_units().magnitude,
            core_index=self.core_index.to_base_units().magnitude,
            shell_width=self.shell_width.to_base_units().magnitude,
            core_diameter=self.core_diameter.to_base_units().magnitude,
            medium_index=self.medium_index.to_base_units().magnitude,
            source=self.source.binding
        )

    def an(self, max_order: Optional[int] = 0) -> numpy.ndarray:
        r"""
        Compute :math:`a_n` coefficient.

        If max_order is set to zero, the maximum order output is calculated using the Wiscombe criterion.

        Parameters
        ----------
        max_order : Optional[int])
            The maximum order of the coefficient. Default is 0.

        Returns
        -------
        numpy.ndarray
            Array of :math:`a_n` coefficients.
        """
        return self.binding.an(max_order)

    def bn(self, max_order: Optional[int] = 0) -> numpy.ndarray:
        r"""
        Compute :math:`b_n` coefficient.

        If max_order is set to zero, the maximum order output is calculated using the Wiscombe criterion.

        Parameters
        ----------
        max_order : Optional[int])
            The maximum order of the coefficient. Default is 0.

        Returns
        -------
        numpy.ndarray
            Array of :math:`b_n` coefficients.
        """
        return self.binding.bn(max_order)
