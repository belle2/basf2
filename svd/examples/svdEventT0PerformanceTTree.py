#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##################################################################################
#
# Example Script to use SVDEventT0PerformanceTTree on simulation or real data
#
# Use: basf2 -i <RAWDATAFILE> script -- --fileDir temp/ --fileTag test [...]
#
###################################################################################

import basf2 as b2
from basf2 import conditions as b2conditions
import rawdata as raw
import tracking as trk
import simulation as sim
import glob
import argparse

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--fileDir", default="./",
                    help="Output File Dir")
parser.add_argument("--fileTag", default="test",
                    help="Output File Tag")
parser.add_argument("--isMC", action="store_true",
                    help="Use Simulation")
parser.add_argument("--is3sample", action="store_true",
                    help="Emulate SVD 3 samples")
parser.add_argument("--RootOutput", action="store_true",
                    help="Store svd clusters before reconstruction to root file")
parser.add_argument("--noReco", action="store_true",
                    help="Do not perform the reconstruction")
parser.add_argument("--test", action="store_true",
                    help="Test with small numbers of events")
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
    main.add_module('EvtGenInput')

    sim.add_simulation(
        main,
        bkgfiles=bkgFiles,
        forceSetPXDDataReduction=True,
        usePXDDataReduction=ROIfinding,
        simulateT0jitter=simulateJitter)

    if args.is3sample:
        for m in main.modules():
            if m.name() == "SVDEventInfoSetter":
                m.param("daqMode", 1)
                m.param("relativeShift", 7)
                m.param("useDB", False)

else:
    # setup database
    b2conditions.reset()
    b2conditions.override_globaltags()
    b2conditions.globaltags = ["online"]
    b2conditions.prepend_globaltag("data_reprocessing_prompt")
    b2conditions.prepend_globaltag("patch_main_release-07")

    MCTracking = False

    if args.test:
        main.add_module('RootInput', entrySequences=['0:100'])
    else:
        main.add_module('RootInput')

    main.add_module("Gearbox")
    main.add_module('Geometry', useDB=True)

    raw.add_unpackers(main)

    if args.is3sample:

        # change ListName
        for moda in main.modules():
            if moda.name() == 'SVDUnpacker':
                moda.param("svdShaperDigitListName", "SVDShaperDigits6Sample")
                moda.param("SVDEventInfo", "SVDEventInfo6Sample")

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


if args.noReco:
    #  clusterizer
    main.add_module('SVDClusterizer')
else:
    # now do reconstruction:
    # clusterizer added by default
    trk.add_tracking_reconstruction(
        main,
        mcTrackFinding=MCTracking,
        append_full_grid_cdc_eventt0=True,
        trackFitHypotheses=[211])  # ,
    #    skipHitPreparerAdding=True)


# fill TTrees
outputFileName = str(args.fileDir)+"SVDEventT0PerformanceTTree"
if args.isMC:
    outputFileName += "_MC"
if args.is3sample:
    outputFileName += "_emulated3sample"

if not args.noReco:
    recoFileName = outputFileName + "_" + str(args.fileTag) + ".root"
    main.add_module('SVDEventT0PerformanceTTree', outputFileName=recoFileName)

if args.RootOutput:
    rootOutFileName = outputFileName + "_RootOutput_" + str(args.fileTag) + ".root"
    main.add_module('RootOutput',
                    outputFileName=rootOutFileName,
                    branchNames=['SVDClusters'])

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
