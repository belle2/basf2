#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file creates the Belle II detector geometry and perfoms the
# simulation and standard reconstruction. Finally the MillepedeCollector
# is used to re-fit tracks with General Broken Lines
#
# It also uses display to demonstrate its capability to visualize tracks
# fitted with GBL (done as byproduct of MillepedeCollector module)
#
##############################################################################
##

import os
from basf2 import *
import simulation
import reconstruction
from ROOT import Belle2

# register necessary modules
main = create_path()

main.add_module('RootInput')
# main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('SetupGenfitExtrapolation')
main.add_module(
    'MillepedeCollector',
    components=[],
    tracks=['RecoTracks'],
    particles=['mu+:bbmu'],
    vertices=[],
    primaryVertices=['Z0:mumu'])
main.add_module('Display', showRecoTracks=True)
# main.add_module('RootOutput')
process(main)
print(statistics)
