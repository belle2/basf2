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
    set_random_seed, create_path, statistics, print_path

from basf2 import conditions as b2c
import simulation as si
import tracking as trk
from tracking.harvesting_validation.FlippingMVA.savingFlippingVariables import Saving1stMVAData
from tracking.harvesting_validation.FlippingMVA.savingFlippingVariablesFor2ndMVA import Saving2ndMVAData

"""
import variables.utils as vu
from variables import variables as vm
import os
"""
import argparse
import glob


def arg_parser():

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-n', '--events',
                        default=100,
                        type=int,
                        help='number of events',
                        metavar='EVENTS')
    parser.add_argument('--no_bkg',
                        default=False,
                        action='store_true',
                        help='flag to remove beam background from simulation')
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
    parser.add_argument('--flip_recoTrack',
                        default=False,
                        action='store_true',
                        help='add this option to flip the reco_tracks')
    parser.add_argument('--flip_mva_cut',
                        default=0.5,
                        type=float,
                        help='the mva cut for flipping')
    parser.add_argument('--num',
                        default=1,
                        type=int,
                        help='the index of mva',
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

    # Argument parsing
    args.events
    args.output_file_mva

    # Prepend and/or append the input GTs
    if not (args.prepend_gt == ''):
        for gt in reversed(args.prepend_gt):
            b2c.prepend_globaltag(gt)

    # add (or not) the beam background
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')

    if args.exp == 1003:
        bkgFiles = glob.glob('/group/belle2/dataprod/BGOverlay/early_phase3/release-05-01-15/overlay/phase31/BGx1/set0/*root')

    if args.no_bkg:
        bkgFiles = None

    flip_recoTrack = False
    if args.flip_recoTrack:
        flip_recoTrack = True
    # particle type
    #####################################################
    # Part 1: setup PArticleGun
    #####################################################
    # Part 2: setup other modules
    eventinfosetter = register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [args.events], 'runList': [1], 'expList': [args.exp]})

    #####################################################
    # Part 3: build the path for generation/simulation/reconstruction
    main = create_path()
    main.add_module(eventinfosetter)
    main.add_module("Progress")
    main.add_module("EvtGenInput")
    si.add_simulation(main, bkgfiles=bkgFiles)
    trk.add_tracking_reconstruction(main, flip_recoTrack=flip_recoTrack)

    #####################################################
    outputfile = args.output_file_mva

    if (args.num == 1):
        trackingVali_0 = Saving1stMVAData(
            name="saving1stMVA_BBbar",
            contact="none",
            output_file_name=outputfile)
        main.add_module(trackingVali_0)
    if (args.num == 2):
        trackingVali_0 = Saving2ndMVAData(
            name="saving2ndMVA_BBbar",
            contact="none",
            output_file_name=outputfile)
        main.add_module(trackingVali_0)

    # Process events
    print_path(main)
    process(main)

    # Print call statistics
    print(statistics)
