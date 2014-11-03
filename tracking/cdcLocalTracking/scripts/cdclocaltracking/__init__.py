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
import logging


def getLogger():
    return logging.getLogger(__name__)


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


class IfModule(Module):

    """Wrapper module to conditionally execute module and continue with the normal path afterwards.

    There are two ways to specify the condition. 
    One way is to override the condtion(self) method in a subclass. 
    The second way is to give a function as the second argument to the module constructor, which is called
    each event.

    Attributes
    ----------
    module : basf2.Module
         The module executed, if the condition is met.
    condition : function() -> bool, optional
         Function executed at each event to determine, if the given module shall be executed.
         If None call the condition method instead, which can be overridden by subclasses
    """

    def __init__(self, module, condition=None):
        """Initialisation method taking the module instance to 

        Parameters
        ----------
        module : basf2.Module
            The module executed, if the condition is met.
        condition : function() -> bool, optional
            Function executed at each event to determine, if the given module shall be executed.
            If None call the condition method instead, which can be overridden by subclasses.
        """

        super(IfModule, self).__init__()

        if condition is not None:
            ## Condition function called at each event to determine if wrapped module should be executed
            self.condition = condition

        conditional_path = create_path()
        conditional_path.add_module(module)
        self.if_true(conditional_path, AfterConditionPath.CONTINUE)

    def condition(self):
        """Condition method called if not given a condition function during construction.
        
        Can be overridden by a concrete subclass to specify under which condition the wrapped module should be executed.
        It can also be shadowed by a condition function given to the constructor of this module.

        Returns
        -------
        bool
            The indication if the wrapped module should be executed. Always True in the base implementation
        """

        return True

    def event(self):
        """Event method of the module

        Evaluates the condition and sets the return value of this module to trigger the execution of the wrapped module.
        """

        condition_is_met = self.condition()
        self.return_value(bool(condition_is_met))


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


class MCAxialStereoPairCreatorModule(Module):

    """A BASF2 python module that creates segment pairs from the segments stored in the CDCRecoSegment2Ds array on the DataStore.
    
    Generated segment pairs are stored in the DataStore in the CDCAxialStereoSegmentPairs array.
    Their cell weight is set to the value assigned by the return value of the MCAxialStereoSegmentPairFilter.
    Optionally forward backward symmetry can be keep by a parameter, meaning that additionally to true segment pairs also their reversed counter part is accepted as true.
    Segment pairs are fitted after their creation with an adjustable fit method.


    Attributes
    ----------
    See Parameters

    Parameters
    ----------
    create_mc_true_only : bool
        Switch if only true segment pairs shall be written to the DataStore. 
        If False wrong segment pairs are stored as well with weight of NOT_A_CELL (NAN).
        Defaults to True.
    allow_backward : bool
        Switch to accept segment pairs that are reverse to a true segment pair.
        If True reversed segment pairs are store with the weight their true counterpart
        Defaults to False
    filter_method : function
        A function called on each segment pair as its only argumetn that may prevent it from being saved to the DataStore.
        See default_filter_method for an example. Default to None meaning the default_fitler_method is used, accepting all pairs.

    """

    def __init__(
        self,
        create_mc_true_only=True,
        allow_backward=False,
        filter_method=None,
        ):

        ## Limit the segment pair generation that are aligned in a true track
        self.create_mc_true_only = create_mc_true_only

        ## Give a positive decision for segment pairs that are aligned in the opposite direction of a true track.
        self.allow_backward = allow_backward

        ## Method used to prevent segment pairs to be stored in the DataStore
        self.filter_method = filter_method
        if not filter_method:
            self.filter_method = self.default_filter_method

        ## Counter for the signal events that are accepted from the MCAxialStereoSegmentPairFilter
        self.num_signal = 0

        ## Counter for the signal events rejected by the filter_method
        self.num_rejected_signal = 0

        super(MCAxialStereoPairCreatorModule, self).__init__()

    @staticmethod
    def default_filter_method(axialStereoSegmentPair):
        """Default method to prevent segment pairs to be saved to the DataStore. Default implementation does reject any pairs."""

        return True

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
        filter_method = self.filter_method

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
                self.num_signal += 1
                if filter_method(axialStereoSegmentPair):
                    axialStereoSegmentPair.getAutomatonCell().setCellWeight(mcWeight)
                    axialStereoSegmentPair.copyToStoreArray()
                else:
                    if mcWeight == mcWeight:
                        self.num_rejected_signal += 1

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
        logger = getLogger()
        logger.info('Number of pairs accepted by the MCAxialStereoSegmentPairFilter : %s'
                    , self.num_signal)
        logger.info('Number of signal pairs rejected by filter method: %s',
                    self.num_rejected_signal)


class MCAxialStereoPairFitterModule(Module):

    @staticmethod
    def default_fit_method(axialStereoSegmentPair):
        """Default method to fit the generated segment pairs."""

        CDCAxialStereoFusion.reconstructFuseTrajectories(axialStereoSegmentPair,
                True)

    def __init__(self, fit_method=None):
        """
        fit_method : function
            A function called on each stored segment pair as its only argument to update its fit inplace.
            See default_fit_method for an example. Defaults to None meaning the default_fit_method is used
        """

        ## Method used to fit the individual segment pairs
        self.fit_method = fit_method
        if not fit_method:
            self.fit_method = self.default_fit_method

        super(MCAxialStereoPairFitterModule, self).__init__()

    def event(self):
        self.fitStoredPairs()

    def fitStoredPairs(self):
        """Fits all pairs in the StoreArray with designated fit method."""

        fit_method = self.fit_method
        for axialStereoSegmentPair in \
            CDCAxialStereoSegmentPair.getStoreArray():
            fit_method(axialStereoSegmentPair)


