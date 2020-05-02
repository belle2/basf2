#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce SVD DQM plots
#
# Author:  Giulia Casarosa
#
# Usage: basf2 runEventT0DQM.py -i "<path/to/file.root>"
#
# 2020 Belle II Collaboration
#############################################################

import os
import glob

from basf2 import *
from basf2 import conditions as b2conditions
from svd import add_svd_reconstruction
from tracking import add_tracking_reconstruction
from rawdata import add_unpackers
from simulation import add_simulation

data = True
run = 5751

# RAW
files = ['/group/belle2/dataprod/Data/Raw/e0010/r0'+str(run)+'/sub00/physics.0010.0'+str(run)+'.HLT*.root']
# 5751 (1% media)
# 5900

# old-format cDST
# files=["/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_hadron/cdst/sub00/cdst.physics.0010.04295.HLT1*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_bhabha/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_mumu_2trk/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root"]

bkg = glob.glob('/group/belle2/BGFile/OfficialBKG/early_phase3/prerelease-04-00-00a/overlay/phase31/BGx1/set0/*.root')
# needed for some temporary issues with BKLMDisplacement payload
if data:
    b2conditions.override_globaltags()
    b2conditions.globaltags = ['klm_alignment_testing', 'online']

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [1003])
eventinfosetter.param('runList', [1])

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO


# main main
main = create_path()


# read input rootfile
# -> can be overwritten with the -i option
if data:
    main.add_module("RootInput", inputFileNames=files)
else:
    main.add_module(eventinfosetter)
    main.add_module(evtgeninput)

# nee to know SVD geometry to create histograms
main.add_module('Gearbox')
main.add_module('Geometry')

# if using RAW data you need to unpack them
if data:
    add_unpackers(main, components=['SVD'])

if not data:
    main.add_module('FullSim')
    add_simulation(main, bkgfiles=bkg)

# add offline ZS
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)

main.add_module(
    'SVDOccupancyAnalysis',
    outputFileName='SVDOccupancyAnalysis_exp10run'+str(run)+'.root',
    ShaperDigitsName='SVDShaperDigitsZS5',
    skipHLTRejectedEvents=True)

# == Show progress
main.add_module('Progress')

print_path(main)

# Process events
process(main)

print(statistics)