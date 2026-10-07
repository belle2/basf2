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


ALLOWED_COMPONENTS = ['VTX', 'PXD', 'SVD', 'STL', 'FTL', 'rCDC', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']

# Pairs of component sets that are mutually exclusive alternatives for the
# same subsystem (e.g. VTX replaces PXD+SVD): a components list may not
# contain a member of both sides of a pair at the same time.
_MUTUALLY_EXCLUSIVE_COMPONENT_GROUPS = [({'VTX'}, {'PXD', 'SVD'}), ({'STL', 'FTL', 'rCDC'}, {'CDC'})]

# Experiment-number ranges (half-open: [exp_min, exp_max)) mapped to their
# canonical detector-component set. A new detector generation should be added
# here as a new range, rather than as a new boolean flag threaded through
# add_simulation()/add_reconstruction() and friends. See `get_components()`.
_COMPONENT_SETS_BY_EXPERIMENT = [
    (0,    2000, ['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']),  # standard Belle II
    (2003, 2004, ['VTX', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']),  # upgrade scenario
    (2004, 2005, ['VTX', 'STL', 'FTL', 'rCDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']),  # upgrade scenario
    (2005, 2006, ['VTX', 'rCDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']),  # upgrade scenario
    (2006, 2007, ['VTX', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM', 'TRG']),  # upgrade scenario
]


def check_components(components):
    """
    Check list of geometry components. This function is used by the standard
    scripts, for example `simulation.add_simulation()`. Thus, only the detector
    components corresponding to subdetectors are allowed. In addition, TRG is
    included.  Trigger is not a geometry component, but it is used as an
    additional component in raw-data and DQM scripts.

    If there is a unsupported component in the list, or if it contains more
    than one component from the same mutually-exclusive group (e.g. both
    'VTX' and 'SVD'), the function will raise a FATAL error and is guaranteed
    to not return.

    Parameters:
        components (list(str)): List of geometry components.
    """

    if components is None:
        return
    for component in components:
        if not (component in ALLOWED_COMPONENTS):
            basf2.B2FATAL(f'Geometry component {component} is unknown or it cannot be used in standard scripts.')
    components_set = set(components)
    for side_a, side_b in _MUTUALLY_EXCLUSIVE_COMPONENT_GROUPS:
        present_a = side_a & components_set
        present_b = side_b & components_set
        if present_a and present_b:
            basf2.B2FATAL(f'Mutually exclusive geometry components requested together: '
                          f'{sorted(present_a)} and {sorted(present_b)}')


def get_components(exp_number):
    """
    Return the canonical list of geometry components for a given experiment
    number, e.g. `get_components(0)` returns the standard PXD/SVD-based
    Belle II components, while `get_components(2003)` returns the VTX-based
    upgrade components.

    This is intended to be used together with `add_simulation()` and
    `add_reconstruction()`, e.g.::

        main.add_module('EventInfoSetter', expList=[2003], ...)
        add_simulation(main, components=get_components(2003), useVTX=True)
        add_reconstruction(main, components=get_components(2003), useVTX=True)

    Parameters:
        exp_number (int): Experiment number.

    Returns:
        list(str): List of geometry components registered for this experiment number.
    """

    for exp_min, exp_max, components in _COMPONENT_SETS_BY_EXPERIMENT:
        if exp_min <= exp_number < exp_max:
            return list(components)
    basf2.B2FATAL(f'No default component set registered for experiment number {exp_number}.')


def is_detector_present(component: str, components: list[str] | None = None) -> bool:
    """
    Check whether a detector component is contained in the list of components AND if it is an allowed component,
    or if components is None.

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

    return False


def are_detectors_present(components_to_check: list[str], components: list[str] | None = None) -> bool:
    """
    Check whether all detectors in the list "components_to_check" are present in the list of components,
    or if components is None.

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
    Check whether any detector in the list "components_to_check" is present in the list of components,
    or if components is None.

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
