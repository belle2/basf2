#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


# This returns a VXDTF module with current 'best' options
# Argument candName can be used to define a non-standard name for the
# output genfit::TrackCand StoreArray
#
# The idea of returning the module instead of adding it to a path is
# that the user can change the settings.

def setup_vxdtf(
    candName='',
    secSetup=['TB4GeVNoMagnetNoAlignedSource2014May21SVD-moreThan1500MeV_SVD'
              ],
    filterOverlaps='hopfield',
    baseLineTF=0,
    ):

    # VXDTF:
    ## parameters:
    qiType = 'circleFit'
    #
    vxdtf = register_module('VXDTF')
    # vxdtf.logging.log_level = LogLevel.INFO
    vxdtf.logging.log_level = LogLevel.DEBUG
    vxdtf.logging.debug_level = 5
    param_vxdtf = {
        'activateBaselineTF': baseLineTF,
        'tccMinState': [2],
        'tccMinLayer': [3],
        'standardPdgCode': -11,
        'artificialMomentum': 5.,
        'sectorSetup': secSetup,
        'calcQIType': qiType,
        'killEventForHighOccupancyThreshold': 100,
        'highOccupancyThreshold': 1200,
        'cleanOverlappingSet': False,
        'filterOverlappingTCs': filterOverlaps,
        'TESTERexpandedTestingRoutines': False,
        'qiSmear': False,
        'smearSigma': 0.000001,
        'GFTrackCandidatesColName': candName,
        'tuneCutoffs': 0.51,
        'activateDistanceXY': [False],
        'activateDistance3D': [True],
        'activateDistanceZ': [False],
        'activateSlopeRZ': [False],
        'activateNormedDistance3D': [False],
        'activateAngles3D': [True],
        'activateAnglesXY': [False],
        'activateAnglesRZ': [False],
        'activateDeltaSlopeRZ': [False],
        'activateDistance2IP': [False],
        'activatePT': [False],
        'activateHelixParameterFit': [False],
        'activateAngles3DHioC': [True],
        'activateAnglesXYHioC': [True],
        'activateAnglesRZHioC': [False],
        'activateDeltaSlopeRZHioC': [False],
        'activateDistance2IPHioC': [False],
        'activatePTHioC': [False],
        'activateHelixParameterFitHioC': [False],
        'activateDeltaPtHioC': [False],
        'activateDeltaDistance2IPHioC': [False],
        'activateZigZagXY': [False],
        'activateZigZagRZ': [False],
        'activateDeltaPt': [False],
        'activateCircleFit': [True],
        'tuneCircleFit': [0],
        }
        # 'tuneDistanceZ': [1.2],
    vxdtf.param(param_vxdtf)
    return vxdtf


def setup_vxdtf1T(
    candName='',
    secSetup=['TB4GeVFullMagnetNoAlignedSource2014May21SVD-moreThan1500MeV_SVD'
              ],
    filterOverlaps='hopfield',
    baseLineTF=0,
    ):

    # VXDTF:
    ## parameters:
    qiType = 'circleFit'
    #
    vxdtf = register_module('VXDTF')
    # vxdtf.logging.log_level = LogLevel.INFO
    vxdtf.logging.log_level = LogLevel.DEBUG
    vxdtf.logging.debug_level = 5
    param_vxdtf = {  #   'artificialMomentum': 5.,
        'activateBaselineTF': baseLineTF,
        'tccMinState': [2],
        'tccMinLayer': [3],
        'standardPdgCode': -11,
        'sectorSetup': secSetup,
        'calcQIType': qiType,
        'killEventForHighOccupancyThreshold': 125,
        'highOccupancyThreshold': 1200,
        'cleanOverlappingSet': False,
        'filterOverlappingTCs': filterOverlaps,
        'TESTERexpandedTestingRoutines': False,
        'qiSmear': False,
        'smearSigma': 0.000001,
        'GFTrackCandidatesColName': candName,
        'tuneCutoffs': 0.3,
        'activateDistanceXY': [False],
        'activateDistanceZ': [True],
        'activateDistance3D': [True],
        'activateAngles3DHioC': [False],
        'activateAnglesXYHioC': [False],
        'activateDeltaSlopeRZHioC': [False],
        'activateDistance2IPHioC': [False],
        'activatePTHioC': [False],
        'activateHelixParameterFitHioC': [False],
        'activateDeltaPtHioC': [False],
        'activateDeltaDistance2IPHioC': [False],
        'activateAngles3D': [True],
        'activateAnglesXY': [False],
        'activateAnglesRZ': [True],
        'activateDeltaSlopeRZ': [False],
        'activateDistance2IP': [False],
        'activatePT': [False],
        'activateHelixParameterFit': [False],
        'activateZigZagXY': [False],
        'activateDeltaPt': [False],
        'activateCircleFit': [True],
        'tuneCircleFit': [0],
        }
   #     'tuneDistanceZ': [1.2],
    vxdtf.param(param_vxdtf)
    return vxdtf


