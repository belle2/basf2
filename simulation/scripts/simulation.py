#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_simulation(path, components=None, bkgfiles=None):
    """
    This function adds the standard simulation modules to a path.
    """

    # geometry parameter database
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)

    # detector geometry
    geometry = register_module('Geometry')
    if components:
        geometry.param('Components', components)
    path.add_module(geometry)

    # detector simulation
    g4sim = register_module('FullSim')
    path.add_module(g4sim)

    # background mixing
    if bkgfiles:
        bkgmixer = register_module('MixBkg')
        bkgmixer.param('BackgroundFiles', bkgfiles)
        path.add_module(bkgmixer)

    # PXD simulation
    if components == None or 'PXD' in components:
        pxd_digitizer = register_module('PXDDigitizer')
        path.add_module(pxd_digitizer)
        pxd_clusterizer = register_module('PXDClusterizer')
        path.add_module(pxd_clusterizer)

    # SVD simulation
    if components == None or 'SVD' in components:
        svd_digitizer = register_module('SVDDigitizer')
        path.add_module(svd_digitizer)
        svd_clusterizer = register_module('SVDClusterizer')
        path.add_module(svd_clusterizer)

    # CDC simulation
    if components == None or 'CDC' in components:
        cdc_digitizer = register_module('CDCDigitizer')
        path.add_module(cdc_digitizer)

    # TOP simulation
    if components == None or 'TOP' in components:
        top_digitizer = register_module('TOPDigitizer')
        path.add_module(top_digitizer)

    # ARICH simulation
    if components == None or 'ARICH' in components:
        arich_digitizer = register_module('ARICHDigitizer')
        path.add_module(arich_digitizer)

    # ECL simulation
    if components == None or 'ECL' in components:
        ecl_digitizer = register_module('ECLDigitizer')
        path.add_module(ecl_digitizer)

    # BKLM simulation
    if components == None or 'BKLM' in components:
        bklm_digitizer = register_module('BKLMDigitizer')
        path.add_module(bklm_digitizer)

    # EKLM simulation
    if components == None or 'EKLM' in components:
        eklm_digitizer = register_module('EKLMDigitizer')
        path.add_module(eklm_digitizer)


