#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_pxd_reconstruction(path, clusterName=None, digitsName=None, usePXDClusterShapes=False):

    if usePXDClusterShapes:
        if 'ActivatePXDClusterPositionEstimator' not in [e.name() for e in path.modules()]:
            shape_activator = register_module('ActivatePXDClusterPositionEstimator')
            shape_activator.set_name('ActivatePXDClusterPositionEstimator')
            path.add_module(shape_activator)

    if 'PXDClusterizer' not in [e.name() for e in path.modules()]:
        clusterizer = register_module('PXDClusterizer')
        clusterizer.set_name('PXDClusterizer')
        if clusterName:
            clusterizer.param('Clusters', clusterName)
        if digitsName:
            clusterizer.param('Digits', digitsName)
        path.add_module(clusterizer)


def add_pxd_simulation(path, digitsName=None):

    digitizer = register_module('PXDDigitizer')
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
