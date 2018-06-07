#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from softwaretrigger.hltdqm import standard_hltdqm


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
    # PXD
    if dqm_environment == "expressreco":
        # does only make sense on ERECO
        if components is None or 'PXD' in components:
            path.add_module('PXDDQMExpressReco')
    # SVD
    if dqm_environment == "expressreco":
        # main DQM only on ERECO
        if components is None or 'SVD' in components:
            path.add_module('SVDDQMExpressReco')
    # VXD (PXD/SVD common)
    if dqm_environment == "expressreco":
        # does only make sense on ERECO
        if components is None or 'SVD' and 'PXD' in components:
            vxddqmExpRecoMin = register_module('VXDDQMExpressReco')

    # CDC
    if components is None or 'CDC' in components:
        cdcdqm = register_module('cdcDQM7')
        path.add_module(cdcdqm)

        cdcdedxdqm = register_module('CDCDedxDQM')
        cdcdedxdqm.param("UsingHadronfiles", True)
        path.add_module(cdcdedxdqm)

    # ECL
    if components is None or 'ECL' in components:
        ecldqm = register_module('ECLDQM')
        ecldqm.param("NHitsUpperThr1", 400)
        ecldqm.param("PedestalMeanUpperThr", 15000)
        ecldqm.param("PedestalRmsUpperThr", 500.)
        path.add_module(ecldqm)
    # TOP
    if components is None or 'TOP' in components:
        topdqm = register_module('TOPDQM')
        path.add_module(topdqm)
    # BKLM
    if components is None or 'BKLM' in components:
        bklmdqm = register_module("BKLMDQM")
        path.add_module(bklmdqm)
    # ECLTRG
    if components is None or 'TRG' in components:
        trgecldqm = register_module('TRGECLDQM')
        path.add_module(trgecldqm)
    # TrackDQM, needs at least one VXD components to be present or will crash otherwise
    if components is None or 'SVD' in components or 'PXD' in components:
        trackDqm = register_module('TrackDQM')
        path.add_module(trackDqm)
    # ARICH
    if dqm_environment == "expressreco":
        if components is None or 'ARICH' in components:
            path.add_module('ARICHDQM')

    standard_hltdqm(path)
