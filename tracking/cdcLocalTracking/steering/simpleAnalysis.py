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


class ShowNumberOfHits(Module):

    """Small helper module to count the number of hits in tracks"""

    def initialize(self):
        # Initialize singletones from the tracking software
        self.theMCHitLookUp = \
            Belle2.CDCLocalTracking.CDCMCHitLookUp.getInstance()
        self.theWireHitTopology = \
            Belle2.CDCLocalTracking.CDCWireHitTopology.getInstance()

        self.numberOfHits = []

    def event(self):
        """Event method"""

        # Fill extended Monte Carlo information
        theMCHitLookUp = self.theMCHitLookUp
        self.theMCHitLookUp.fill()

        # Unpack the hits from CDCSimHits and attach the geometry to them
        self.theWireHitTopology.fill()

        # Setup  a dictionary to count the number of hits for each particle id
        numberOfHitsByMCTrackId = {}

        for wireHit in vector_iterator(self.theWireHitTopology.getWireHits()):
            # Get the track id of the wire hit
            hit = wireHit.getHit()
            mcTrackIdOfWireHit = theMCHitLookUp.getMCTrackId(hit)

            # Your stuff here
            # ...
            print mcTrackIdOfWireHit

    def terminate(self):
        plt.hist(self.numberOfHits, bins=100)
        plt.show()


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

main.add_module(ShowNumberOfHits())

########## Run paths and print statistics ##########
process(main)
print statistics

