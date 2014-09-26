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

belle2_local_dir = os.environ['BELLE2_LOCAL_DIR']
additional_python_module_path = os.path.join(belle2_local_dir, 'tracking',
        'cdcLocalTracking', 'python_code')
sys.path.append(additional_python_module_path)

import cdcdisplay
import tools

sys.path.pop()


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


