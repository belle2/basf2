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
# Usage: basf2 runClusterizer.py -i "<path/to/file.root>"
#
# 2020 Belle II Collaboration
#############################################################

import glob
import sys

import basf2 as b2
from basf2 import conditions as b2conditions
from rawdata import add_unpackers
from simulation import add_simulation
from svd import add_svd_reconstruction

alg = int(sys.argv[1])
data = False  # True
unpack = True
if not data:
    unpack = False

# only relevant for data:
run = 5900  # 3239 #5751
fileout = '3sampleClusterizer_exp10run'+str(run)+'.root'

# RAW
files = ['/group/belle2/dataprod/Data/Raw/e0010/r0'+str(run)+'/sub00/physics.0010.0'+str(run)+'.HLT*.root']
# 5751
# 5900
# RAW CT skims
# files = ['/gpfs/fs02/belle2/group/detector/SVD/']
# run = 3239

# old-format cDST
# files=["/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_hadron/cdst/sub00/cdst.physics.0010.04295.HLT1*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_bhabha/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root","/group/belle2/dataprod/Data/release-04-00-02/DB00000523/Unofficial/e0010/4S/r04295/skim/hlt_mumu_2trk/cdst/sub00/cdst.physics.0010.04295.HLT1.*.root"]

bkg = glob.glob('/group/belle2/BGFile/OfficialBKG/early_phase3/prerelease-04-00-00a/overlay/phase31/BGx1/set0/*.root')

# needed for some temporary issues with BKLMDisplacement payload
if data:
    b2conditions.override_globaltags()
    b2conditions.globaltags = ['svd_timeCalibration_test', 'klm_alignment_testing', 'online']
else:
    b2conditions.globaltags = ['svd_timeCalibration_test']

# svd_timeCalibration_test CONTENT:
# exp 0, run 0: no calibration applied: returned time from the database = raw time

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [0])

evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.logging.log_level = b2.LogLevel.INFO

# main main
main = b2.create_path()

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
if data and unpack:
    add_unpackers(main, components=['SVD'])

if not data:
    main.add_module('FullSim')
    add_simulation(main, bkgfiles=None, usePXDDataReduction=False, forceSetPXDDataReduction=True)
#    add_simulation(main, bkgfiles=bkg, usePXDDataReduction=False, forceSetPXDDataReduction=True)

if not data:
    fileout = 'SVD3SampleClusterizer_MCnoBKG_timeAlg'+str(alg)+'.root'

add_svd_reconstruction(main)

for m in main.modules():
    if "SVDSimpleClusterizer" == m.name():
        m.param("timeAlgorithm", alg)

main.add_module('RootOutput', outputFileName=fileout, branchNames=['SVDClusters'])
# Show progress
main.add_module('Progress')

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
