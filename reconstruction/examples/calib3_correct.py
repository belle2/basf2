#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# 3 - correct
#
# Apply the calibration constants to the sample generated
# with calib1_generate.py.
#
# Usage: basf2 calib3_correct.py
#
# Input: B2Electrons.root (from calib1_generate.py)
# Output: NewB2Electrons.root
#
# Contributors: Jake Bennett
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from ROOT import Belle2

# main path
main = create_path()

# input file from calib1_generate.py
main.add_module('RootInput', inputFileName='B2Electrons.root')

# apply corrections
main.add_module('DedxCorrection')

# write the results to file
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', 'NewB2Electrons.root')
rootoutput.param('branchNames', ['CDCDedxTracks', 'Tracks', 'TrackFitResults'])
main.add_module(rootoutput)

# add a progress bar
main.add_module('ProgressBar')

# generate events
process(main)

# show call statistics
print(statistics)
