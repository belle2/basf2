#!/usr/bin/env python
# -*- coding: utf-8 -*-

######### Imports #########

import sys
import os
import optparse

from basf2 import *
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libframework')  # for PyStoreArray
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCWire and so on
gSystem.Load('libgenfit2')  # for GFTrackCands

gSystem.Load('libdataobjects')

import simulation

import math
import matplotlib.pyplot as plt

usage = """
Usage:
%prog input_file_name
"""

parser = optparse.OptionParser(usage=usage)

##### Define options ##########

(options, args) = parser.parse_args()

if args:
    input_file_name = args[0]
else:
    input_file_name = 'mc_gun.root'


########## Custom modules ##########

def vector_iterator(v):
    return (v.at(i) for i in xrange(v.size()))


class AnalyseFacetFiltering(Module):

    """Small helper module to generate all facets, then apply a filter to it and analyse the outcome."""

    def initialize(self):
        # Initialize singletones from the tracking software
        self.theMCHitLookUp = \
            Belle2.CDCLocalTracking.CDCMCHitLookUp.getInstance()
        self.theWireHitTopology = \
            Belle2.CDCLocalTracking.CDCWireHitTopology.getInstance()

        # Prepare a filter based on Monte Carlo information
        self.mcFacetFilter = Belle2.CDCLocalTracking.MCFacetFilter()
        self.mcFacetFilter.initialize()

        # Create the worker that creates the facets
        self.allFacetWorker = Belle2.CDCLocalTracking.AllFacetWorker()
        self.allFacetWorker.initialize()

        # Initalize some counters to be extended
        self.nFacets = 0
        self.nAcceptedFacets = 0
        self.nSignalFacets = 0
        self.nAcceptedSignalFacets = 0

    def isSignalFacet(self, facet):
        mcWeight = self.mcFacetFilter.isGoodFacet(facet)
        if mcWeight == mcWeight:  # Test for nan
            return True
        else:
            return False

    def isGoodFacet(self, facet):
        # To be refined
        startRLInfo = facet.getStartRLInfo()
        right = 1
        left = -1

        return True

    def event(self):
        """Event method"""

        # Fill extended Monte Carlo information
        theMCHitLookUp = self.theMCHitLookUp
        theMCHitLookUp.fill()

        # Unpack the hits from CDCSimHits and attach the geometry to them
        self.theWireHitTopology.fill()

        # Generate all facets
        self.allFacetWorker.generate()

        print '#Facets', self.allFacetWorker.getRecoFacets().size()

        for facet in vector_iterator(self.allFacetWorker.getRecoFacets()):
            self.nFacets += 1

            isSignal = self.isSignalFacet(facet)
            accepted = self.isGoodFacet(facet)
            if accepted:
                self.nAcceptedFacets += 1

            if isSignal:
                self.nSignalFacets += 1

            if isSignal and accepted:
                self.nAcceptedSignalFacets += 1

    def terminate(self):
        print '#Facets', self.nFacets
        print 'Efficiency', 1.0 * self.nAcceptedSignalFacets \
            / self.nSignalFacets
        print 'Purity', 1.0 * self.nAcceptedSignalFacets / self.nAcceptedFacets

        nBackgroundFacets = self.nFacets - self.nSignalFacets
        nRejectedBackgroundFacets = self.nFacets - self.nSignalFacets \
            - self.nAcceptedFacets + self.nAcceptedSignalFacets
        print 'Background rejection', 1.0 * nRejectedBackgroundFacets \
            / nBackgroundFacets


########## Register modules ##########

rootInputModule = register_module('RootInput')
rootInputModule.param({'inputFileName': input_file_name})

gearboxModule = register_module('Gearbox')
geometryModule = register_module('Geometry')

########## Create paths and add modules ##########

main = create_path()
main.add_module(rootInputModule)
main.add_module(gearboxModule)
main.add_module(geometryModule)

main.add_module(AnalyseFacetFiltering())

########## Run paths and print statistics ##########
process(main)
print statistics

