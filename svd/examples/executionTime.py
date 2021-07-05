#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
import basf2 as b2
from svd.executionTime_utils import SVDExtraEventStatisticsModule
import simulation as simu
import tracking as trk

import glob

tag = "test"
if len(sys.argv) == 2:
    tag = sys.argv[1]

# EXAMPLE OF EXECUTION TIME MEASUREMENT
main = b2.create_path()

b2.set_random_seed(1)

# set the exp/run event informations
eventinfosetter = b2.register_module('EventInfoSetter')
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
    b2.B2ERROR('No files found in ', bkgDir)
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

b2.process(main)

print(b2.statistics)
