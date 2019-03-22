#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
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
            path.add_module('PXDDAQDQM', histogramDirectoryName='PXDDAQ')
            path.add_module('PXDDQMExpressReco', histogramDirectoryName='PXDER')
            path.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEFF')
        # SVD
        if components is None or 'SVD' in components:
            # SVD DATA FORMAT
            svdunpackerdqm = register_module('SVDUnpackerDQM')
            path.add_module(svdunpackerdqm)
            # ZeroSuppression Emulator
            path.add_module(
                'SVDZeroSuppressionEmulator',
                SNthreshold=5,
                ShaperDigits='SVDShaperDigits',
                ShaperDigitsIN='SVDShaperDigitsZS5',
                FADCmode=True)
            svddqm = register_module('SVDDQMExpressReco')
            svddqm.param('offlineZSShaperDigits', 'SVDShaperDigitsZS5')
            path.add_module(svddqm)
        # VXD (PXD/SVD common)
        if components is None or 'PXD' in components or 'SVD' in components:
            vxddqm = register_module('VXDDQMExpressReco')
            path.add_module(vxddqm)

    if dqm_environment == "hlt":
        # HLT
        path.add_module("SoftwareTriggerHLTDQM")
        path.add_module("StatisticsTimingHLTDQM")

        # SVD DATA FORMAT
        if components is None or 'SVD' in components:
            svdunpackerdqm = register_module('SVDUnpackerDQM')
            path.add_module(svdunpackerdqm)

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
        path.add_module(ecldqm)
        ecldqmext = register_module('ECLDQMEXTENDED')
        path.add_module(ecldqmext)
    # TOP
    if components is None or 'TOP' in components:
        topdqm = register_module('TOPDQM')
        path.add_module(topdqm)
    # KLM
    if components is None or 'BKLM' or 'EKLM' in components:
        klmdqm = register_module("KLMDQM")
        path.add_module(klmdqm)
    # TRG
    if components is None or 'TRG' in components:
        # TRGECL
        trgecldqm = register_module('TRGECLDQM')
        path.add_module(trgecldqm)
        # TRGGDL
        trggdldqm = register_module('TRGGDLDQM')
        path.add_module(trggdldqm)
        # TRGCDCTSF
        nmod_tsf = [0, 1, 2, 3, 4, 5, 6]
        for mod_tsf in nmod_tsf:
            path.add_module('TRGCDCTSFDQM', TSFMOD=mod_tsf)
        # TRGCDC3D
        nmod_t3d = [0, 1, 2, 3]
        for mod_t3d in nmod_t3d:
            path.add_module('TRGCDCT3DConverter',
                            hitCollectionName='FirmCDCTriggerSegmentHits' + str(mod_t3d),
                            addTSToDatastore=True,
                            EventTimeName='FirmBinnedEventT0' + str(mod_t3d),
                            addEventTimeToDatastore=True,
                            inputCollectionName='FirmTRGCDC2DFinderTracks' + str(mod_t3d),
                            add2DFinderToDatastore=True,
                            outputCollectionName='FirmTRGCDC3DFitterTracks' + str(mod_t3d),
                            add3DToDatastore=True,
                            fit3DWithTSIM=0,
                            firmwareResultCollectionName='TRGCDCT3DUnpackerStore' + str(mod_t3d),
                            isVerbose=0)
            path.add_module('TRGCDCT3DDQM', T3DMOD=mod_t3d)

    # TrackDQM, needs at least one VXD components to be present or will crash otherwise
    if components is None or 'SVD' in components or 'PXD' in components:
        trackDqm = register_module('TrackDQM')
        path.add_module(trackDqm)
    # ARICH
    if components is None or 'ARICH' in components:
        path.add_module('ARICHDQM')
    # PhysicsObjectsDQM
    add_analysis_dqm(path)
    # DAQ Monitor
    path.add_module('DAQMonitor')
