#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import math


def add_pxd_unpacker(path):
    pxdunpacker = b2.register_module('PXDUnpacker')
    path.add_module(pxdunpacker)

    pxderrorcheck = b2.register_module('PXDPostErrorChecker')
    path.add_module(pxderrorcheck)

    pxdhitsorter = b2.register_module('PXDRawHitSorter')
    path.add_module(pxdhitsorter)
    if 'ActivatePXDPixelMasker' not in [e.name() for e in path.modules()]:
        path.add_module('ActivatePXDPixelMasker')


def add_pxd_packer(path):
    pxdpacker = b2.register_module('PXDPacker')
    pxdpacker.param('dhe_to_dhc', [
        [
            0,
            2,
            4,
            34,
            36,
            38,
        ],
        [
            1,
            6,
            8,
            40,
            42,
            44,
        ],
        [
            2,
            10,
            12,
            46,
            48,
            50,
        ],
        [
            3,
            14,
            16,
            52,
            54,
            56,
        ],
        [
            4,
            3,
            5,
            35,
            37,
            39,
        ],
        [
            5,
            7,
            9,
            41,
            43,
            45,
        ],
        [
            6,
            11,
            13,
            47,
            49,
            51,
        ],
        [
            7,
            15,
            17,
            53,
            55,
            57,
        ],
    ])

    path.add_module(pxdpacker)


def add_pxd_reconstruction(path, clusterName=None, digitsName=None, usePXDClusterShapes=False):

    # register EventTrackingInfo
    if 'RegisterEventLevelTrackingInfo' not in path:
        path.add_module('RegisterEventLevelTrackingInfo')

    if usePXDClusterShapes:
        if 'ActivatePXDClusterPositionEstimator' not in [e.name() for e in path.modules()]:
            shape_activator = b2.register_module('ActivatePXDClusterPositionEstimator')
            shape_activator.set_name('ActivatePXDClusterPositionEstimator')
            path.add_module(shape_activator)

    if 'PXDClusterizer' not in [e.name() for e in path.modules()]:
        clusterizer = b2.register_module('PXDClusterizer')
        clusterizer.set_name('PXDClusterizer')
        if clusterName:
            clusterizer.param('Clusters', clusterName)
        if digitsName:
            clusterizer.param('Digits', digitsName)
        path.add_module(clusterizer)

    path.add_module('PXDTrackingEventLevelMdstInfoFiller')


def add_pxd_simulation(path, digitsName=None, activatePixelMasks=True, activateGainCorrection=True):

    if activatePixelMasks and 'ActivatePXDPixelMasker' not in [e.name() for e in path.modules()]:
        path.add_module('ActivatePXDPixelMasker')
    if activateGainCorrection and 'ActivatePXDGainCalibrator' not in [e.name() for e in path.modules()]:
        path.add_module('ActivatePXDGainCalibrator')

    digitizer = b2.register_module('PXDDigitizer')
    if digitsName:
        digitizer.param('Digits', digitsName)
    path.add_module(digitizer)


def add_pxd_fullframe(path, min_ladders=(1, 1), max_ladders=(8, 12)):
    modules = []
    for layer in [1, 2]:
        min_ladder = min_ladders[layer - 1]
        max_ladder = max_ladders[layer - 1]
        for ladder in range(min_ladder, max_ladder + 1):
            for sensor in [1, 2]:
                modules.append((layer, ladder, sensor))

    for (layer, ladder, sensor) in modules:
        path.add_module('ROIGenerator', ROIListName='ROIs', nROIs=1, TrigDivider=1,
                        Layer=layer, Ladder=ladder, Sensor=sensor,
                        MinU=0, MaxU=249, MinV=0, MaxV=767)


def add_pxd_fullframe_phase3_early(path):
    add_pxd_fullframe(path, min_ladders=(1, 4), max_ladders=(8, 5))


def add_pxd_percentframe(path, min_ladders=(1, 1), max_ladders=(8, 12), fraction=0.1, random_position=False):
    modules = []
    for layer in [1, 2]:
        min_ladder = min_ladders[layer - 1]
        max_ladder = max_ladders[layer - 1]
        for ladder in range(min_ladder, max_ladder + 1):
            for sensor in [1, 2]:
                modules.append((layer, ladder, sensor))

    # Center ROI and make them a bit more realistic, enlarge in z ;-)
    # Random position not supported yet -> need change in module code
    s = math.sqrt(fraction)
    MinU = max(0, int(250 / 2 * (1 - 0.5 * s)))
    MaxU = 249 - MinU
    MinV = max(0, int(768 / 2 * (1 - 2.0 * s)))
    MaxV = 767 - MinV

    for (layer, ladder, sensor) in modules:
        path.add_module('ROIGenerator', ROIListName='ROIs', nROIs=1, TrigDivider=1,
                        Layer=layer, Ladder=ladder, Sensor=sensor,
                        MinU=MinU, MaxU=MaxU, MinV=MinV, MaxV=MaxV, Random=random_position)


def add_pxd_percentframe_phase3_early(path, fraction=0.1, random_position=False):
    add_pxd_percentframe(path, min_ladders=(1, 4), max_ladders=(8, 5), fraction=fraction, random_position=random_position)


def add_roi_payload_assembler(path, ignore_hlt_decision):
    path.add_module('ROIPayloadAssembler',
                    ROIListName='ROIs', ROIpayloadName='ROIpayload',
                    SendAllDownscaler=0, SendROIsDownscaler=1,
                    AcceptAll=ignore_hlt_decision, NoRejectFlag=False)


def add_roi_finder(path):
    """
    Add the PXDDataReduction module to preserve the tracking informaiton for ROI calculation
    :param path: The path to which the module should be added
    :param calcROIs: True: turn on the ROI calculation, False: turn off
    """

    path.add_module('PXDROIFinder', recoTrackListName='RecoTracks',
                    PXDInterceptListName='PXDIntercepts', ROIListName='ROIs',
                    tolerancePhi=0.15, toleranceZ=0.5,
                    sigmaSystU=0.02, sigmaSystV=0.02,
                    numSigmaTotU=10, numSigmaTotV=10,
                    maxWidthU=0.5, maxWidthV=0.5)
