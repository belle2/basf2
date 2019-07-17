#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2


def check_components(components):
    """
    Check list of geometry components. This function is used by the standard
    scripts, for example add_simulation(). Thus, only the detector components
    corresponding to subdetectors are allowed. In addition, TRG is included.
    Trigger is not a geometry component, but it is used as an additional
    component in raw-data and DQM scripts.

    :param components: List of geometry components.
    """

    if (components is None):
        return
    component_list = ['PXD', 'SVD', 'ECL', 'TOP', 'ARICH', 'BKLM', 'EKLM', 'TRG']
    for (component in components):
        if (not (component in component_list)):
            basf2.B2FATAL('Geometry component %s is unknown or it cannot be used in standard scripts.' % (component))
