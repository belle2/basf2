#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def _has_module(path, module_type):
    "Check if a module of a given type is already in the path"
    result = module_type in (e.type() for e in path.modules())
    if result:
        B2WARNING("add_simulation: Module '%s' already present in path, "
                  "skipping" % module_type)
    return result


def check_simulation(path):
    """Check if the minimum number of modules required for simulation are in
    the path and in the correct order"""
    required = ['Gearbox', 'Geometry', 'FullSim']
    found = []

    # make a list of all required modules in the path
    for module in path.modules():
        module_type = module.type()
        # if required add to list of found modules
        if module_type in required:
            # unless it is already in there
            if module_type in found:
                B2ERROR("Duplicate module in path: %s" % module_type)
            else:
                found.append(module.type())

    if len(required) != len(found):
        #Apparently at least one module is missing
        for r in required:
            if r not in found:
                B2ERROR("No '%s' module found but needed for simulation" % r)
    #We have all modules but do they have the correct order?
    elif required != found:
        B2ERROR("Simulation modules in wrong order. Should be '%s' but is '%s'"
                % (", ".join(required), ", ".join(found)))


def add_simulation(path, components=None, bkgfiles=None, bkgcomponents=None):
    """
    This function adds the standard simulation modules to a path.
    """

    # geometry parameter database
    if not _has_module(path, 'Gearbox'):
        gearbox = register_module('Gearbox')
        path.add_module(gearbox)

    # detector geometry
    if not _has_module(path, 'Geometry'):
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # detector simulation
    if not _has_module(path, 'FullSim'):
        g4sim = register_module('FullSim')
        path.add_module(g4sim)

    check_simulation(path)

    # no checks are performed for BeamBkgMixer and the Digitizers as they are
    # not necessary for running simulation jobs and it should be possible to
    # have them in the path more than once

    # background mixing
    if bkgfiles:
        bkgmixer = register_module('BeamBkgMixer')
        bkgmixer.param('backgroundFiles', bkgfiles)
        if bkgcomponents:
            bkgmixer.param('components', bkgcomponents)
        else:
            if components:
                bkgmixer.param('components', components)
        path.add_module(bkgmixer)

    # PXD digitization
    if components is None or 'PXD' in components:
        pxd_digitizer = register_module('PXDDigitizer')
        path.add_module(pxd_digitizer)
        pxd_clusterizer = register_module('PXDClusterizer')
        path.add_module(pxd_clusterizer)

    # SVD digitization
    if components is None or 'SVD' in components:
        svd_digitizer = register_module('SVDDigitizer')
        path.add_module(svd_digitizer)
        svd_clusterizer = register_module('SVDClusterizer')
        path.add_module(svd_clusterizer)

    # CDC digitization
    if components is None or 'CDC' in components:
        cdc_digitizer = register_module('CDCDigitizer')
        path.add_module(cdc_digitizer)

    # TOP digitization
    if components is None or 'TOP' in components:
        top_digitizer = register_module('TOPDigitizer')
        path.add_module(top_digitizer)

    # ARICH digitization
    if components is None or 'ARICH' in components:
        arich_digitizer = register_module('ARICHDigitizer')
        path.add_module(arich_digitizer)

    # ECL digitization
    if components is None or 'ECL' in components:
        ecl_digitizer = register_module('ECLDigitizer')
        path.add_module(ecl_digitizer)

    # BKLM digitization
    if components is None or 'BKLM' in components:
        bklm_digitizer = register_module('BKLMDigitizer')
        path.add_module(bklm_digitizer)

    # EKLM digitization
    if components is None or 'EKLM' in components:
        eklm_digitizer = register_module('EKLMDigitizer')
        path.add_module(eklm_digitizer)
