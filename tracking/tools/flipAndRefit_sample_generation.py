#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import register_module, process, \
    set_random_seed, create_path, statistics, print_path, B2FATAL

from basf2 import conditions as b2c
import simulation as si
import tracking as trk
from tracking.FlippingMVA.savingFlippingVariables import Saving1stMVAData
from tracking.FlippingMVA.savingFlippingVariablesFor2ndMVA import Saving2ndMVAData
from tracking.path_utils import add_prune_tracks
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
                        help='number of events',
                        metavar='EVENTS')
    parser.add_argument('--use_background',
                        default=True,
                        action='store_true',
                        help='flag to include background in the simulation')
    parser.add_argument('--exp',
                        default=1003,
                        type=int,
                        help='experiment number you want to simulate',
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
                        help='output name')
    parser.add_argument('--num',
                        default=1,
                        type=int,
                        help='the index of track-flipping MVA variables to be savd. (1 or 2)',
                        metavar='NUMS')
    parser.add_argument('--ranseed',
                        default=0,
                        type=int,
                        help='random seed',
                        metavar='NUMS')
    return parser


if __name__ == "__main__":

    #####################################################
    # Part 0: general setup
    args = arg_parser().parse_args()

    # Setting the random seed for particle generation
    set_random_seed(args.ranseed)

    # Prepend and/or append the input GTs
    if not (args.prepend_gt == ''):
        for gt in reversed(args.prepend_gt):
            b2c.prepend_globaltag(gt)

    # add (or not) the beam background
    bkgFiles = None
    if args.use_background:
        bkgFiles = get_background_files()

    #####################################################
    # Part 1: setup the EventInfoSetter
    eventinfosetter = register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [args.events], 'runList': [1], 'expList': [args.exp]})

    main = create_path()
    main.add_module(eventinfosetter)
    main.add_module("Progress")

    #####################################################
    # Part 2: setup other modules
    main.add_module("EvtGenInput")

    #####################################################
    # Part 3: build the path for simulation/reconstruction

    flip_recoTrack = True

    if (args.num == 1):
        flip_recoTrack = False
    if (args.num == 2):
        flip_recoTrack = True

    si.add_simulation(main, bkgfiles=bkgFiles)
    trk.add_prefilter_tracking_reconstruction(main)

    # V0 finding
    reco_tracks = "RecoTracks"
    main.add_module('V0Finder', RecoTracks=reco_tracks, v0FitterMode=1)

    main.add_module("FlipQuality", recoTracksStoreArrayName=reco_tracks,
                    identifier='TRKTrackFlipAndRefit_MVA1_weightfile',
                    indexOfFlippingMVA=1).set_name("FlipQuality_1stMVA")
    if args.num == 2:
        reco_tracks_flipped = "RecoTracks_flipped"
        main.add_module("RecoTracksReverter", inputStoreArrayName=reco_tracks,
                        outputStoreArrayName=reco_tracks_flipped)
        main.add_module("DAFRecoFitter", recoTracksStoreArrayName=reco_tracks_flipped).set_name("Combined_DAFRecoFitter_flipped")
        main.add_module("IPTrackTimeEstimator",
                        recoTracksStoreArrayName=reco_tracks_flipped, useFittedInformation=False)
        main.add_module("TrackCreator", trackColName="Tracks_flipped",
                        trackFitResultColName="TrackFitResults_flipped",
                        recoTrackColName=reco_tracks_flipped,
                        pdgCodes=[211, 321, 2212]).set_name("TrackCreator_flipped")
        main.add_module("FlipQuality", recoTracksStoreArrayName=reco_tracks,
                        identifier='TRKTrackFlipAndRefit_MVA2_weightfile',
                        indexOfFlippingMVA=2).set_name("FlipQuality_2ndMVA")

    main.add_module('TrackTimeEstimator')
    # prune
    add_prune_tracks(main, reco_tracks=reco_tracks)
    main.add_module("PruneRecoHits")

    #####################################################
    outputfile = args.output_file_mva

    if (args.num == 1):
        trackingVali_0 = Saving1stMVAData(
            name="saving1stMVA_BBbar",
            contact="none",
            output_file_name=outputfile)
        main.add_module(trackingVali_0)
    elif (args.num == 2):
        trackingVali_0 = Saving2ndMVAData(
            name="saving2ndMVA_BBbar",
            contact="none",
            output_file_name=outputfile)
        main.add_module(trackingVali_0)
    else:
        B2FATAL("no data saving module added ...")

    # Process events
    print_path(main)
    process(main)

    # Print call statistics
    print(statistics)
