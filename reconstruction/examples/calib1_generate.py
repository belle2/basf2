#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# 1 - generate
#
# Generate a sample of electron tracks for dE/dx calibration
# using the BabayagaNLO generator (ee(gamma) - large angle)
#
# Usage: basf2 1_generate.py
#
# Input: None
# Output: B2Electrons.root
#
# Contributors: Jake Bennett
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# for curve parameters (until added to GT)
use_local_database("localdb/database.txt", "localdb")

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=10000)

# add the BABAYAGA.NLO module
babayaganlo = register_module('BabayagaNLOInput')
main.add_module(babayaganlo)

# detector simulation and reconstruction
add_simulation(main)
add_reconstruction(main)

# write the results to file
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', 'B2Electrons.root')
rootoutput.param('branchNames', ['CDCDedxTracks', 'Tracks', 'TrackFitResults'])
main.add_module(rootoutput)

# add a progress bar
main.add_module('ProgressBar')

# generate events
process(main)

# show call statistics
print(statistics)
