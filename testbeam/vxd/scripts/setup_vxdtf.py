#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# This returns a VXDTF module with current 'best' options
# Argument candName can be used to define a non-standard name for the
# output genfit::TrackCand StoreArray
#
# The idea of returning the module instead of adding it to a path is
# that the user can change the settings.


def setup_vxdtf(candName='',
                secSetup=['testBeamMini6GeVSVD-moreThan1500MeV_SVD'],
                filterOverlaps='hopfield'):

    # VXDTF:
    # # parameters:
    qiType = 'circleFit'
    #
    vxdtf = register_module('VXDTF')
    vxdtf.logging.log_level = LogLevel.INFO
    vxdtf.logging.debug_level = 1
    param_vxdtf = {
        'activateBaselineTF': 1,
        'tccMinState': [2],
        'tccMinLayer': [3],
        'standardPdgCode': -11,
        'sectorSetup': secSetup,
        'calcQIType': qiType,
        'killEventForHighOccupancyThreshold': 75,
        'highOccupancyThreshold': 85,
        'cleanOverlappingSet': False,
        'filterOverlappingTCs': filterOverlaps,
        'TESTERexpandedTestingRoutines': False,
        'qiSmear': False,
        'smearSigma': 0.000001,
        'GFTrackCandidatesColName': candName,
        'tuneCutoffs': 4,
        'activateDistanceXY': [False],
        'activateDistanceZ': [True],
        'activateDistance3D': [True],
        'activateAngles3DHioC': [False],
        'activateAnglesXYHioC': [False],
        'activateDeltaSlopeRZHioC': [False],
        'activateDistance2IPHioC': [False],
        'activatePTHioC': [False],
        'activateHelixFitHioC': [False],
        'activateDeltaPtHioC': [False],
        'activateDeltaDistance2IPHioC': [False],
        'activateAngles3D': [True],
        'activateAnglesXY': [False],
        'activateAnglesRZ': [True],
        'activateDeltaSlopeRZ': [False],
        'activateDistance2IP': [False],
        'activatePT': [False],
        'activateHelixFit': [False],
        'activateZigZagXY': [False],
        'activateDeltaPt': [False],
        'activateCircleFit': [True],
        'tuneCircleFit': [0],
        'tuneDistanceZ': [1.2],
        }
    vxdtf.param(param_vxdtf)
    return vxdtf


