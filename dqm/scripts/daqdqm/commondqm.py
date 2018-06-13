#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from softwaretrigger.hltdqm import standard_hltdqm
from analysisDQM import add_analysis_dqm


def add_common_dqm(path, components=None, dqm_environment="expressreco"):
    """
    This function adds DQMs which are common for Cosmic runs and Collion runs

    @param components: A list of the detector components which are available in this
                       run of basf2
    @param dqm_environment: The environment the DQM modules are running in
                            "expressreco" (default) if running on the ExpressReco system
                            "hlt" if running on the HLT online reconstructon nodes
                            If running on the hlt, you may want to output less or other DQM plots
                            due to the limited bandwith of the HLT nodes.
    """

    if dqm_environment == "expressreco":
        # PXD (not useful on HLT)
        if components is None or 'PXD' in components:
            pxddqm = register_module('PXDDQMExpressReco')
            path.add_module(pxddqm)
        # SVD
        if components is None or 'SVD' in components:
            svddqm = register_module('SVDDQMExpressReco')
            path.add_module(svddqm)
        # VXD (PXD/SVD common)
        if components is None or 'PXD' in components or 'SVD' in components:
            vxddqm = register_module('VXDDQMExpressReco')
            path.add_module(vxddqm)

    if dqm_environment == "hlt":
        # HLT
        standard_hltdqm(path)

    # CDC
    if components is None or 'CDC' in components:
        cdcdqm = register_module('cdcDQM7')
        path.add_module(cdcdqm)

    # ECL
    if components is None or 'ECL' in components:
        ecldqm = register_module('ECLDQM')
        path.add_module(ecldqm)
    # TOP
    if components is None or 'TOP' in components:
        topdqm = register_module('TOPDQM')
        path.add_module(topdqm)
    # BKLM
    if components is None or 'BKLM' in components:
        bklmdqm = register_module("BKLMDQM")
        path.add_module(bklmdqm)
    # EKLM
    if components is None or 'EKLM' in components:
        eklmdqm = register_module('EKLMDQM')
        path.add_module(eklmdqm)
    # ECLTRG
    if components is None or 'TRG' in components:
        trgecldqm = register_module('TRGECLDQM')
        path.add_module(trgecldqm)
    # TrackDQM, needs at least one VXD components to be present or will crash otherwise
    if components is None or 'SVD' in components or 'PXD' in components:
        trackDqm = register_module('TrackDQM')
        path.add_module(trackDqm)
    # ARICH
    if components is None or 'ARICH' in components:
        path.add_module('ARICHDQM')
    # PhysicsObjectsDQM
    add_analysis_dqm(path)
