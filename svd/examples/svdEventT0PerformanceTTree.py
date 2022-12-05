#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##################################################################################
#
# Example Script to use SVDEventT0PerformanceTTree and OverlapResidual Modules
# on simulation or real data
#
# Use: basf2 -i <RAWDATAFILE> script -- --fileDir temp/ --fileTag test
#
###################################################################################

import basf2 as b2
from basf2 import conditions as b2conditions
import rawdata as raw
import tracking as trk
import simulation as sim
import glob
# import sys
import argparse
# import os

# b2.set_log_level(b2.LogLevel.DEBUG)
# b2.set_debug_level(40)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--fileDir", default="./",
                    help="Output File Dir")
parser.add_argument("--fileTag", default="test",
                    help="Output File Tag")
parser.add_argument("--isMC", action="store_true",
                    help="Use Simulation")
parser.add_argument("--is3sample", action="store_true",
                    help="Emulate SVD 3 samples")
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")

main = b2.create_path()

b2.set_random_seed(1)

if args.isMC:
    # options for simulation:
    # expList = [1003]
    expList = [0]
    numEvents = 20
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')  # Phase3 background
    bkgFiles = None  # uncomment to remove  background
    simulateJitter = False
    ROIfinding = False
    MCTracking = False
    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param('expList', expList)
    eventinfosetter.param('runList', [0])
    eventinfosetter.param('evtNumList', [numEvents])
    main.add_module(eventinfosetter)
    # main.add_module('EventInfoPrinter')
    main.add_module('EvtGenInput')

    sim.add_simulation(
        main,
        bkgfiles=bkgFiles,
        forceSetPXDDataReduction=True,
        usePXDDataReduction=ROIfinding,
        simulateT0jitter=simulateJitter)
else:
    # setup database
    b2conditions.reset()
    b2conditions.override_globaltags()
    b2conditions.globaltags = ["online"]
    b2conditions.prepend_globaltag("data_reprocessing_prompt")
    b2conditions.prepend_globaltag("patch_main_release-07")

    MCTracking = False

    # input root files
    # main.add_module('RootInput', branchNames=['RawPXDs', 'RawSVDs', 'RawCDCs', 'RawECLs'])
    # raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC', 'ECL'])

    # main.add_module('RootInput', entrySequences=['0:50'])
    main.add_module('RootInput')

    main.add_module("Gearbox")
    main.add_module('Geometry', useDB=True)

if not args.isMC:
    raw.add_unpackers(main)

    if args.is3sample:
        # change ListName
        for moda in main.modules():
            if moda.name() == 'SVDUnpacker':
                moda.param("svdShaperDigitListName", "SVDShaperDigits6Sample")
                moda.param("SVDEventInfo", "SVDEventInfo6Sample")
        # main.add_module("SVDZeroSuppressionEmulator",SNthreshold=5,ShaperDigits="SVDShaperDigitsZS3",ShaperDigitsIN="SVDShaperDigits")

if args.is3sample:
    # emulate 3-sample DAQ for events
    zsemulator = b2.register_module("SVD3SamplesEmulator")
    zsemulator.param("outputSVDShaperDigits", "SVDShaperDigits3SampleAll")
    zsemulator.param("outputSVDEventInfo", "SVDEventInfo")
    zsemulator.param("chooseStartingSample", False)
    zsemulator.param("chooseRelativeShift", True)
    zsemulator.param("relativeShift", 7)
    zsemulator.param("SVDShaperDigits", "SVDShaperDigits6Sample")
    zsemulator.param("SVDEventInfo", "SVDEventInfo6Sample")
    main.add_module(zsemulator)

    zsonline = b2.register_module("SVDZeroSuppressionEmulator")
    zsonline.param("ShaperDigits", "SVDShaperDigits3SampleAll")
    zsonline.param("ShaperDigitsIN", "SVDShaperDigits")
    main.add_module(zsonline)

    # #  clusterizer
    # clusterizer = b2.register_module('SVDClusterizer')
    # clusterizer.set_name("SVDClusterizer_3Sample")
    # clusterizer.param('ShaperDigits', "SVDShaperDigits3Sample")
    # clusterizer.param('Clusters', "SVDClusters3Sample")
    # clusterizer.param('EventInfo', "SVDEventInfo3Sample")
    # main.add_module(clusterizer)

# now do reconstruction:
trk.add_tracking_reconstruction(
    main,
    mcTrackFinding=MCTracking,
    append_full_grid_cdc_eventt0=True,
    trackFitHypotheses=[211])  # ,
#    skipHitPreparerAdding=True)

'''
# skim mu+mu- events:
ma.applyEventCuts("nTracks ==2", path=main)

mySelection = 'pt>1.0 and abs(dz)<0.5 and dr<0.4'
ma.fillParticleList('mu+:DQM', mySelection, path=main)
ma.reconstructDecay('Upsilon(4S):IPDQM -> mu+:DQM mu-:DQM', '10<M<11', path=main)

skimfilter = b2.register_module('SkimFilter')
skimfilter.set_name('SkimFilter_MUMU')
skimfilter.param('particleLists', ['Upsilon(4S):IPDQM'])
main.add_module(skimfilter)
filter_path = b2.create_path()
skimfilter.if_value('=1', filter_path, b2.AfterConditionPath.CONTINUE)
'''

# fill TTrees
outputFileName = str(args.fileDir)+"SVDEventT0PerformanceTTree"
if args.isMC:
    outputFileName += "_MC"
if args.is3sample:
    outputFileName += "_emulated3sample"
outputFileName += "_"+str(args.fileTag)

# main.add_module('SVDEventT0PerformanceTTree', outputFileName=outputFileName+".root")

# # write everything
# main.add_module('OverlapResiduals', ExpertLevel=True)

main.add_module('RootOutput',
                outputFileName=outputFileName+"_RootOutput.root",
                branchNames=['SVDClusters'])

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
