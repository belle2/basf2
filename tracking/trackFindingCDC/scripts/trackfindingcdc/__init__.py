#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from basf2 import *

from ROOT import gSystem
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCWireHit
gSystem.Load('libtracking_trackFindingCDC')
gSystem.Load('libgenfit2')  # for GFTrackCands

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import os
import sys
import itertools
import logging


def get_logger():
    return logging.getLogger(__name__)


class SegmentFitterModule(Module):

    @staticmethod
    def default_fit_method(segment):
        """Default method to fit the generated segments."""

        CDCRiemannFitter = Belle2.TrackFindingCDC.CDCRiemannFitter
        return CDCRiemannFitter.getFitter().fit(segment)

    def __init__(self, fit_method=None):
        """
        fit_method : function
            A function called on each stored segment pair as its only argument to update its fit inplace.
            See default_fit_method for an example. Defaults to None meaning the default_fit_method is used
        """

        # Method used to fit the individual segment pairs
        self.fit_method = fit_method
        if not fit_method:
            self.fit_method = self.default_fit_method

        super(SegmentFitterModule, self).__init__()

    def event(self):
        self.fitStoredSegments()

    def fitStoredSegments(self):
        """Executes a fit on each segment generated"""

        stored_segments = Belle2.PyStoreObj('CDCRecoSegment2DVector')
        wrapped_segments = stored_segments.obj()
        segments = wrapped_segments.get()

        fit_method = self.fit_method
        for segment in segments:
            fit = fit_method(segment)
            if fit is not None:
                segment.setTrajectory2D(fit)


class AxialStereoPairFitterModule(Module):

    @staticmethod
    def default_fit_method(segmentPair):
        """Default method to fit the generated segment pairs."""

        CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion
        CDCAxialStereoFusion.reconstructFuseTrajectories(segmentPair,
                                                         True)

    def __init__(self, fit_method=None):
        """
        fit_method : function
            A function called on each stored segment pair as its only argument to update its fit inplace.
            See default_fit_method for an example. Defaults to None meaning the default_fit_method is used
        """

        # Method used to fit the individual segment pairs
        self.fit_method = fit_method
        if not fit_method:
            self.fit_method = self.default_fit_method

        super(AxialStereoPairFitterModule, self).__init__()

    def event(self):
        self.fitStoredPairs()

    def fitStoredPairs(self):
        """Fits all pairs in the StoreArray with designated fit method."""

        fit_method = self.fit_method

        stored_axial_stereo_segment_pairs = Belle2.PyStoreObj("CDCSegmentPairVector")
        wrapped_axial_stereo_segment_pairs = stored_axial_stereo_segment_pairs.obj()
        axial_stereo_segment_pairs = wrapped_axial_stereo_segment_pairs.get()

        for axial_stereo_segment_pair in axial_stereo_segment_pairs:
            fit_method(axial_stereo_segment_pair)
