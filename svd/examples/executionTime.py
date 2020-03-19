#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce SVD DQM plots
#
# Author:  Giulia Casarosa
#
# Usage: basf2 executionTime.py [tag-string]
#        tag-string is a tring to identify the output file
#
# 2020 Belle II Collaboration
#############################################################

import sys
import os
from basf2 import *
from svd.executionTime_utils import SVDExtraEventStatisticsModule
import simulation as simu
import tracking as trk

import glob

tag = "test"
if len(sys.argv) == 2:
    tag = sys.argv[1]

# EXAMPLE OF EXECUTION TIME MEASUREMENT
main = create_path()

set_random_seed(1)

# set the exp/run event informations
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10])
main.add_module(eventinfosetter)

# generate signal
main.add_module('EvtGenInput')

# add default simulation
bkgDir = '/group/belle2/BGFile/OfficialBKG/early_phase3/prerelease-04-00-00a/overlay/phase31/BGx1/set0/*.root'
bg = glob.glob(bkgDir)
if len(bg) == 0:
    B2ERROR('No files found in ', bkgDir)
    sys.exit()
simu.add_simulation(main, bkgfiles=bg, usePXDDataReduction=False, forceSetPXDDataReduction=True)

# add default tracking reconstruction
trk.add_tracking_reconstruction(main)

# add offline ZS for ExecutionTime module
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)

main.add_module(SVDExtraEventStatisticsModule("SVDExecutionTime_"+str(tag)+".root"))

main.add_module('Progress')

process(main)

print(statistics)
