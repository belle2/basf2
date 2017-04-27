#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple example script to reconstruct cosmics events
Usage :
basf2 runRec.py <run>
run: Run number
"""

from basf2 import *
from ROOT import Belle2
import datetime
from tracking import add_cdc_cr_track_finding
from reconstruction import *
from simulation import *
import os.path
import sys
from cdc.cr import *

# Set the global log level
set_log_level(LogLevel.INFO)


def main(run, period, mode):

    main_path = create_path()
    empty_path = create_path()

    inputFilename = '/ghi/fs01/belle2/bdata/users/karim/MC_data/Cosmics_test2/rawdata_run' + run + '.root'
    main_path.add_module('RootInput', inputFileNames=[inputFilename])

    main_path.add_module('Progress')

    # set_cdc_cr_parameters(period)

    # Propagation velocity of the light in the scinti.
    lightPropSpeed = 12.9925

    # Run range.
    run_range = {'normal': [-1, -1]}

    # Size of trigger counter.
    triggerSize = {'normal': [100.0, 8.0, 10.0]}

    # Center position of trigger counter.
    triggerPosition = {'normal': [0.0, 0.0, 0.0]}

    # Normal direction of the trigger plane.
    triggerPlaneDirection = {'normal': [0, 1, 0]}

    # PMT position.
    pmtPosition = {'normal': [0, 0, -50.0]}

    # Global phi rotation.
#    globalPhiRotation = {'normal': 0.0}

    lengthOfCounter = 100.0
    widthOfCounter = 8.0
    triggerPos = []
    normTriggerPlanDirection = []
    readOutPos = []
#    globalPhi = 0.0

    global lengthOfCounter
    global widthOfCounter
    global triggerPos
    global normTriggerPlanDirection
    global readOutPos
#    global globalPhi

    lengthOfCounter = triggerSize[period][0]
    widthOfCounter = triggerSize[period][1]
    triggerPos = triggerPosition[period]
    normTriggerPlanDirection = triggerPlaneDirection[period]
    readOutPos = pmtPosition[period]
#    globalPhi = globalPhiRotation[period]

#    phi = getPhiRotation()

    gearbox = register_module('Gearbox')
    """,
                              fileName="/geometry/GCR_Summer2017.xml",
                              override=[
                                  ("/Global/length", "8.", "m"),
                                  ("/Global/width", "8.", "m"),
                                  ("/Global/height", "8.", "m"),
                              ])
    """
    main_path.add_module(gearbox)

    main_path.add_module('Geometry')

    add_cdc_cr_track_finding(main_path)

    main_path.add_module("SetupGenfitExtrapolation")

    main_path.add_module("PlaneTriggerTrackTimeEstimator",
                         pdgCodeToUseForEstimation=13,
                         triggerPlanePosition=triggerPos,
                         triggerPlaneDirection=normTriggerPlanDirection,
                         useFittedInformation=False)

    main_path.add_module("DAFRecoFitter",
                         probCut=0.00001,
                         pdgCodesToUseForFitting=13,
                         )

    main_path.add_module("PlaneTriggerTrackTimeEstimator",
                         pdgCodeToUseForEstimation=13,
                         triggerPlanePosition=triggerPos,
                         triggerPlaneDirection=normTriggerPlanDirection,
                         useFittedInformation=True,
                         useReadoutPosition=True,
                         readoutPosition=readOutPos,
                         readoutPositionPropagationSpeed=lightPropSpeed
                         )

    main_path.add_module("DAFRecoFitter",
                         # probCut=0.00001,
                         pdgCodesToUseForFitting=13,
                         )

    # Select the tracks for the time extraction.
    #        main_path.add_module("SelectionForTrackTimeExtraction")

    # Extract the time: either with the TrackTimeExtraction or the FullGridTrackTimeExtraction module.
    #        main_path.add_module("FullGridTrackTimeExtraction")

    main_path.add_module("DAFRecoFitter",
                         # probCut=0.00001,
                         pdgCodesToUseForFitting=13,
                         )
    main_path.add_module('TrackCreator',
                         defaultPDGCode=13,
                         useClosestHitToIP=True
                         )

    # Add further reconstruction modules
    # add_posttracking_reconstruction(main_path)

    # I have a problem with the following line ? significantly reduced [mdst] output, why ?
    add_dedx_modules(main_path)
    add_ext_module(main_path)

    add_top_modules(main_path)
    # add_arich_modules(main_path)

    add_ecl_modules(main_path)
    add_ecl_track_matcher_module(main_path)
    add_ecl_eip_module(main_path)
    add_ecl_mc_matcher_module(main_path)

    add_klm_modules(main_path)
    add_klm_mc_matcher_module(main_path)

    add_muid_module(main_path)
    add_pid_module(main_path)

    # Add output
    # add_mdst_output(main_path,mc=True,filename='/ghi/fs01/belle2/bdata/users/karim/MC_data/Cosmics/mdst_run' + run + '.root',
    #                additionalBranches=['ECLDigits','ECLCalDigits','RecoTracks'])

    #
    main_path.add_module('MillepedeCollector', minPValue=0.)
    output = register_module('RootOutput')
    outputFilename = '/ghi/fs01/belle2/bdata/users/karim/MC_data/Cosmics/dst_run' + run + '.root'
    output.param('outputFileName', outputFilename)
    main_path.add_module(output)

    print_path(main_path)
    process(main_path)
    print(statistics)

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('run', help='Run number')
    parser.add_argument('--period', dest='period', default='normal', help='Data period')

    args = parser.parse_args()

    date = datetime.datetime.today()
    print(date.strftime('Start at : %d-%m-%y %H:%M:%S\n'))
    main(args.run, args.period, '')
    date = datetime.datetime.today()
    print(date.strftime('End at : %y-%m-%d %H:%M:%S\n'))
