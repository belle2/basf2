#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import process, set_random_seed, create_path, statistics, print_path, B2FATAL

from basf2 import conditions as b2c
from simulation import add_simulation
from tracking import add_prefilter_tracking_reconstruction
from tracking.FlippingMVA.savingFlippingVariables import Saving1stMVAData
from tracking.FlippingMVA.savingFlippingVariablesFor2ndMVA import Saving2ndMVAData
from background import get_background_files
import argparse

"""
generating BBbar MC samples for training and testing the track-flipping MVAs
"""


def arg_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-n', '--events',
                        default=1000,
                        type=int,
                        help='Number of events to be processed. Default is 1000.',
                        metavar='EVENTS')
    parser.add_argument('--no_bkg',
                        default=False,
                        action='store_true',
                        help='Flag to include background in the simulation. Default is False')
    parser.add_argument('--exp',
                        default=0,
                        type=int,
                        help='Experiment number you want to simulate. Default is 0.',
                        metavar='EXP')
    parser.add_argument('--prepend_gt',
                        default='',
                        type=str,
                        nargs='*',
                        help='Global Tags to be prepended',
                        metavar='GTs')
    parser.add_argument('--output_file_mva',
                        default='',
                        type=str,
                        help='Output file name. Default is \'\'.')
    parser.add_argument('--num',
                        default=1,
                        type=int,
                        help='The index of track-flipping MVA variables to be saved. (1 or 2). Default is 1.',
                        metavar='NUMS')
    parser.add_argument('--randseed',
                        default=0,
                        type=int,
                        help='Random seed value. Default is 0.',
                        metavar='NUMS')
    return parser


if __name__ == "__main__":

    #####################################################
    # Part 0: general setup parsing the input and checking the arguments
    args = arg_parser().parse_args()

    if args.events < 0:
        B2FATAL("Number of events to process must be larger than 0. Terminating here.")

    if args.num not in [1, 2]:
        B2FATAL("Argument num must be either 1 or 2. Terminating here.")
    training_mva_number = args.num

    if args.exp not in [0, 1003]:
        B2FATAL("Argument exp must be either 0 or 1003. Terminating here.")

    if args.output_file_mva == '':
        B2FATAL("Empty output file name. Terminating here.")

    outputfile = args.output_file_mva
    if not outputfile.endswith(".root"):
        outputfile + ".root"

    # Setting the random seed for particle generation
    set_random_seed(args.randseed)

    # Prepend and/or append the input GTs
    if not (args.prepend_gt == ''):
        for gt in reversed(args.prepend_gt):
            b2c.prepend_globaltag(gt)

    # add (or not) the beam background
    bkgFiles = get_background_files()
    if args.no_bkg:
        bkgFiles = None

    #####################################################
    # Part 1: setup basf2 simulation and run it
    main = create_path()
    main.add_module('EventInfoSetter', evtNumList=[args.events], runList=[0], expList=[args.exp])
    main.add_module("EvtGenInput")

    add_simulation(main, bkgfiles=bkgFiles)

    #####################################################
    # Step 2: run the tracking reconstruction

    add_prefilter_tracking_reconstruction(main)

    main.add_module('TrackTimeEstimator')

    # Save data to train the first MVA. After that, we're done
    if (training_mva_number == 1 or training_mva_number == 2):
        main.add_module("FlipQuality", recoTracksStoreArrayName="RecoTracks",
                        identifier='TRKTrackFlipAndRefit_MVA1_weightfile',
                        indexOfFlippingMVA=1).set_name("FlipQuality_1stMVA")

    # elif (training_mva_number == 2):
        reco_tracks_flipped = "RecoTracks_flipped"
        main.add_module("RecoTracksReverter", inputStoreArrayName="RecoTracks",
                        outputStoreArrayName=reco_tracks_flipped)
        main.add_module("DAFRecoFitter", recoTracksStoreArrayName=reco_tracks_flipped).set_name("Combined_DAFRecoFitter_flipped")
        main.add_module("IPTrackTimeEstimator",
                        recoTracksStoreArrayName=reco_tracks_flipped, useFittedInformation=False)
        main.add_module("TrackCreator", trackColName="Tracks_flipped",
                        trackFitResultColName="TrackFitResults_flipped",
                        recoTrackColName=reco_tracks_flipped,
                        pdgCodes=[211, 321, 2212]).set_name("TrackCreator_flipped")
        main.add_module("FlipQuality", recoTracksStoreArrayName="RecoTracks",
                        identifier='TRKTrackFlipAndRefit_MVA2_weightfile',
                        indexOfFlippingMVA=2).set_name("FlipQuality_2ndMVA")

        if training_mva_number == 1:
            saveFirstMVAData = Saving1stMVAData(
                name="saving1stMVA_BBbar",
                output_file_name=outputfile)
            main.add_module(saveFirstMVAData)
        if training_mva_number == 2:
            saveSecondMVAData = Saving2ndMVAData(
                name="saving2ndMVA_BBbar",
                output_file_name=outputfile)
            main.add_module(saveSecondMVAData)

    # Process events
    print_path(main)

    main.add_module("Progress")
    process(main)

    # Print call statistics
    print(statistics)
