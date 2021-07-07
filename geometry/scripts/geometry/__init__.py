#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Functions related to building or using the simulation/reconstruction geometry
"""

import basf2


def check_components(components):
    """
    Check list of geometry components. This function is used by the standard
    scripts, for example `simulation.add_simulation()`. Thus, only the detector
    components corresponding to subdetectors are allowed. In addition, TRG is
    included.  Trigger is not a geometry component, but it is used as an
    additional component in raw-data and DQM scripts.

    If there is a unsupported component in the list the function will raise a
    FATAL error and is guaranteed to not return.

    Parameters:
        components (list(str)): List of geometry components.
    """

    if components is None:
        return
    allowed_components = ['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']
    for component in components:
        if not (component in allowed_components):
            basf2.B2FATAL('Geometry component %s is unknown or it cannot be used in standard scripts.' % (component))
