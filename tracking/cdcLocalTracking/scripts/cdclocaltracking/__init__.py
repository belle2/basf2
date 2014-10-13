#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

from ROOT import gSystem
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCWireHit
gSystem.Load('libgenfit2')  # for GFTrackCands

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import os
import sys
import itertools

import tools


class MCSegmenterModule(Module):

    """Simple module to generate segment to the DataStore."""

    def __init__(self,
                 fit_method=Belle2.CDCLocalTracking.CDCRiemannFitter.getFitter().fit):
        """Initalizes the module and sets up the method, with which the segments ought to be fitted."""

        ## Method used to fit the individual segments
        self.fit_method = fit_method
        super(MCSegmenterModule, self).__init__()

    def initialize(self):
        """Initializes the segment worker"""

        self.theMCHitLookUp = \
            Belle2.CDCLocalTracking.CDCMCHitLookUp.getInstance()
        self.theWireHitTopology = \
            Belle2.CDCLocalTracking.CDCWireHitTopology.getInstance()

        self.mcSegmentWorker = Belle2.CDCLocalTracking.MCSegmentWorker()
        self.mcSegmentWorker.initialize()

    def event(self):
        """Generates the segments into the DataStore and fits them."""

        self.theMCHitLookUp.fill()
        self.theWireHitTopology.fill()
        self.mcSegmentWorker.generate()
        self.fit()

    def terminate(self):
        """Terminates the segment worker"""

        self.mcSegmentWorker.terminate()

    def fit(self):
        """Executes a fit on each segment generated"""

        segments = Belle2.PyStoreArray('CDCRecoSegment2Ds')
        fit_method = self.fit_method
        for segment in segments:
            fit = fit_method(segment)
            segment.setTrajectory2D(fit)


class MCAxialStereoPairCreator:

    @staticmethod
    def default_fit_method(axialStereoSegmentPair):
        Belle2.CDCLocalTracking.CDCAxialStereoFusion.reconstructFuseTrajectories(axialStereoSegmentPair,
                True)

    def __init__(self, create_mc_true_only=True, fit_method=None):
        self.create_mc_true_only = create_mc_true_only
        self.fit_method = \
            (fit_method if fit_method else self.default_fit_method)

        self.axialStereoSegmentPairs = []

    def initialize(self):
        self.mcAxialStereoSegmentPairFilter = \
            Belle2.CDCLocalTracking.MCAxialStereoSegmentPairFilter()
        self.mcAxialStereoSegmentPairFilter.initialize()

    def event(self):
        """Generates valid axial stereo segment pairs from the CDCRecoSegment2Ds on the DataStore filtering by MCAxialStereoSegmentPairFilter and return them as a list."""

        self.axialStereoSegmentPairs = []
        axialStereoSegmentPairs = self.axialStereoSegmentPairs

        segments = Belle2.PyStoreArray('CDCRecoSegment2Ds')
        mcAxialStereoSegmentPairFilter = self.mcAxialStereoSegmentPairFilter
        create_mc_true_only = self.create_mc_true_only

        axialStereoSegmentPair = \
            Belle2.CDCLocalTracking.CDCAxialStereoSegmentPair()
        segmentsByISuperLayer = [[] for i in range(9)]

        for segment in segments:
            iSuperLayer = segment.getISuperLayer()
            segmentsByISuperLayer[iSuperLayer].append(segment)

        for (iSuperLayer, segmentsForISuperLayer) in \
            enumerate(segmentsByISuperLayer):

            iSuperLayerIn = iSuperLayer - 1
            if iSuperLayerIn >= 0:
                segmentsForISuperLayerIn = segmentsByISuperLayer[iSuperLayerIn]
                for (segment, segmentIn) in \
                    itertools.product(segmentsForISuperLayer,
                                      segmentsForISuperLayerIn):

                    axialStereoSegmentPair.setSegments(segment, segmentIn)
                    axialStereoSegmentPair.clearTrajectory3D()
                    mcWeight = \
                        mcAxialStereoSegmentPairFilter.isGoodAxialStereoSegmentPair(axialStereoSegmentPair)

                    if mcWeight == mcWeight or not create_mc_true_only:
                        axialStereoSegmentPair.getAutomatonCell().setCellWeight(mcWeight)
                        axialStereoSegmentPairs.append(axialStereoSegmentPair)
                        axialStereoSegmentPair = \
                            Belle2.CDCLocalTracking.CDCAxialStereoSegmentPair()

            iSuperLayerOut = iSuperLayer + 1
            if iSuperLayerOut < len(segmentsByISuperLayer):
                segmentsForISuperLayerOut = \
                    segmentsByISuperLayer[iSuperLayerOut]
                for (segment, segmentOut) in \
                    itertools.product(segmentsForISuperLayer,
                                      segmentsForISuperLayerOut):

                    axialStereoSegmentPair.setSegments(segment, segmentOut)
                    axialStereoSegmentPair.clearTrajectory3D()
                    mcWeight = \
                        mcAxialStereoSegmentPairFilter.isGoodAxialStereoSegmentPair(axialStereoSegmentPair)

                    if mcWeight == mcWeight or not create_mc_true_only:
                        axialStereoSegmentPair.getAutomatonCell().setCellWeight(mcWeight)
                        axialStereoSegmentPairs.append(axialStereoSegmentPair)
                        axialStereoSegmentPair = \
                            Belle2.CDCLocalTracking.CDCAxialStereoSegmentPair()

        ## Properly fit the result
        fit_method = self.fit_method
        for axialStereoSegmentPair in axialStereoSegmentPairs:
            fit_method(axialStereoSegmentPair)

        return axialStereoSegmentPairs

    def terminate(self):
        self.mcAxialStereoSegmentPairFilter.terminate()


