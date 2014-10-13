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

CDCWireHitTopology = Belle2.CDCLocalTracking.CDCWireHitTopology

# Lookup
CDCMCHitLookUp = Belle2.CDCLocalTracking.CDCMCHitLookUp

# Rootified data objects
CDCAxialStereoSegmentPair = Belle2.CDCLocalTracking.CDCAxialStereoSegmentPair
CDCRecoSegment2D = Belle2.CDCLocalTracking.CDCRecoSegment2D

# Rootified fitter objects
CDCRiemannFitter = Belle2.CDCLocalTracking.CDCRiemannFitter
CDCAxialStereoFusion = Belle2.CDCLocalTracking.CDCAxialStereoFusion


class MCSegmenterModule(Module):

    """Simple module to generate segment to the DataStore."""

    @staticmethod
    def default_fit_method(segment):
        return CDCRiemannFitter.getFitter().fit(segment)

    def __init__(self, allow_backward=False, fit_method=None):
        """Initalizes the module and sets up the method, with which the segments ought to be fitted."""

        ## Indicates if also segments that are reverse to the actual track should be generated.
        self.allow_backward = allow_backward

        ## Method used to fit the individual segments
        self.fit_method = fit_method
        if not fit_method:
            self.fit_method = self.default_fit_method

        super(MCSegmenterModule, self).__init__()

    def initialize(self):
        """Initializes the segment worker"""

        self.theMCHitLookUp = CDCMCHitLookUp.getInstance()
        self.theWireHitTopology = CDCWireHitTopology.getInstance()

        self.mcSegmentWorker = Belle2.CDCLocalTracking.MCSegmentWorker()
        self.mcSegmentWorker.initialize()

    def event(self):
        """Generates the segments into the DataStore and fits them."""

        self.theMCHitLookUp.fill()
        self.theWireHitTopology.fill()
        self.mcSegmentWorker.generate(self.allow_backward)
        self.fitStoredSegments()

    def fitStoredSegments(self):
        """Executes a fit on each segment generated"""

        segments = Belle2.PyStoreArray('CDCRecoSegment2Ds')
        fit_method = self.fit_method
        for segment in segments:
            fit = fit_method(segment)
            if fit is not None:
                segment.setTrajectory2D(fit)

    def terminate(self):
        """Terminates the segment worker"""

        self.mcSegmentWorker.terminate()
        print 'MCSegmenter terminates'


class MCAxialStereoPairCreatorModule(Module):

    @staticmethod
    def default_fit_method(axialStereoSegmentPair):
        """Default method to fit the generated segment pairs."""

        CDCAxialStereoFusion.reconstructFuseTrajectories(axialStereoSegmentPair,
                True)

    def __init__(
        self,
        create_mc_true_only=True,
        allow_backward=False,
        fit_method=None,
        ):

        ## Limit the segment pair generation that are aligned in a true track
        self.create_mc_true_only = create_mc_true_only

        ## Give a positive decision for segment pairs that are aligned in the opposite direction of a true track.
        self.allow_backward = allow_backward

        ## Method used to fit the individual segments
        self.fit_method = fit_method
        if not fit_method:
            self.fit_method = self.default_fit_method

        super(MCAxialStereoPairCreatorModule, self).__init__()

    def initialize(self):
        self.mcAxialStereoSegmentPairFilter = \
            Belle2.CDCLocalTracking.MCAxialStereoSegmentPairFilter()
        self.mcAxialStereoSegmentPairFilter.initialize()

        CDCAxialStereoSegmentPair.registerStoreArray()
        CDCAxialStereoSegmentPair.registerRelationTo('CDCRecoSegment2Ds')
        CDCRecoSegment2D.registerRelationTo('CDCAxialStereoSegmentPairs')

    def event(self):
        """Generates valid axial stereo segment pairs from the CDCRecoSegment2Ds 
        on the DataStore filtering by MCAxialStereoSegmentPairFilter and output the result to the DataStore."""

        segmentsByISuperLayer = self.getSegmentsByISuperLayer()

        for (iSuperLayer, segmentsForISuperLayer) in \
            enumerate(segmentsByISuperLayer):

            iSuperLayerIn = iSuperLayer - 1
            if iSuperLayerIn >= 0:
                segmentsForISuperLayerIn = segmentsByISuperLayer[iSuperLayerIn]
                self.storePairs(segmentsForISuperLayer,
                                segmentsForISuperLayerIn)

            iSuperLayerOut = iSuperLayer + 1
            if iSuperLayerOut < len(segmentsByISuperLayer):
                segmentsForISuperLayerOut = \
                    segmentsByISuperLayer[iSuperLayerOut]
                self.storePairs(segmentsForISuperLayer,
                                segmentsForISuperLayerOut)

        self.fitStoredPairs()
        self.addRelationsToSegments()

    def getSegmentsByISuperLayer(self):
        """Returns a list of lists of segments where the index of the first list is the superlayer id of the segments contained at that position"""

        segments = Belle2.PyStoreArray('CDCRecoSegment2Ds')

        segmentsByISuperLayer = [[] for i in range(9)]

        for segment in segments:
            iSuperLayer = segment.getISuperLayer()
            segmentsByISuperLayer[iSuperLayer].append(segment)

        return segmentsByISuperLayer

    def storePairs(self, startSegments, endSegments):
        """Generate pairs of segments from the startSegments to the endSegments and copy them to the StoreArray."""

        mcAxialStereoSegmentPairFilter = self.mcAxialStereoSegmentPairFilter
        create_mc_true_only = self.create_mc_true_only
        allow_backward = self.allow_backward
        axialStereoSegmentPair = CDCAxialStereoSegmentPair()

        for (startSegment, endSegment) in itertools.product(startSegments,
                endSegments):
            if startSegment == endSegment:
                continue
            axialStereoSegmentPair.setSegments(startSegment, endSegment)
            axialStereoSegmentPair.clearTrajectory3D()

            mcWeight = \
                mcAxialStereoSegmentPairFilter.isGoodAxialStereoSegmentPair(axialStereoSegmentPair,
                    allow_backward)

            if mcWeight == mcWeight or not create_mc_true_only:
                axialStereoSegmentPair.getAutomatonCell().setCellWeight(mcWeight)
                axialStereoSegmentPair.copyToStoreArray()

    def fitStoredPairs(self):
        """Fits all pairs in the StoreArray with designated fit method."""

        fit_method = self.fit_method
        for axialStereoSegmentPair in \
            CDCAxialStereoSegmentPair.getStoreArray():
            fit_method(axialStereoSegmentPair)

    def addRelationsToSegments(self):
        """Adds relations from start segments to the pairs and from the pairs to the end segments."""

        for axialStereoSegmentPair in \
            CDCAxialStereoSegmentPair.getStoreArray():
            startSegment = axialStereoSegmentPair.getStartSegment()
            endSegment = axialStereoSegmentPair.getEndSegment()

            startSegment.addRelationTo(axialStereoSegmentPair)
            axialStereoSegmentPair.addRelationTo(endSegment)

    def terminate(self):
        self.mcAxialStereoSegmentPairFilter.terminate()


