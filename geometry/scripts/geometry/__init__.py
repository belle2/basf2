#!/usr/bin/env python3

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


ALLOWED_COMPONENTS = ['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']


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
    for component in components:
        if not (component in ALLOWED_COMPONENTS):
            basf2.B2FATAL(f'Geometry component {component} is unknown or it cannot be used in standard scripts.')


def is_detector_present(component: str, components: list[str] | None = None) -> bool:
    """
    Check whether a detector component is contained in the list of components AND if it is an allowed component.

    Returns True, if components is None, or if the component is contained in the list of components and if the component is allowed,
    else, returns False.

    Parameters:
    :param component (str): Detector componet
    :param components (list(str)): List of geometry components present in the current setting.
    :returns Bool indicating whether the detector component is contained and valid.
        Returning True if the components list is None.
    """

    if (components is None) or ((component in components) and (component in ALLOWED_COMPONENTS)):
        return True

    basf2.BINFO(f"Component {component} is not present in list of components.")
    return False


def are_components_present(components_to_check: list[str], components: list[str] | None = None) -> bool:
    """
    Check whether all detectors in the list "components_to_check" are present in the list of components.

    Returns True if all detector components in "components_to_check are contained in components, else, returns False.

    Parameters:
    :param components_to_check (list(str)): List of detector componets to be checked
    :param components (list(str)): List of geometry components present in the current setting.
    :returns Bool indicating whether all the detectors are contained
    """

    if components is None:
        return True

    for component in components_to_check:
        if not is_detector_present(component, components):
            return False

    return True


def is_any_detector_present(components_to_check: list[str], components: list[str] | None = None) -> bool:
    """
    Check whether any detector in the list "components_to_check" is present in the list of components.

    Returns True if any detector components in "components_to_check is contained in components, else, returns False.

    Parameters:
    :param components_to_check (list(str)): List of detector componets to be checked
    :param components (list(str)): List of geometry components present in the current setting.
    :returns Bool indicating whether all the detectors are contained
    """

    if components is None:
        return True

    for component in components_to_check:
        if is_detector_present(component, components):
            return True

    return False
