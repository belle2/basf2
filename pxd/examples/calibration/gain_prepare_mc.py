#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file prepares experiment specific mc files needed for the
# gain calibration.
#
# basf2 gain_prepare_mc.py -- --setnumber=0
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de


from basf2 import *
from ROOT import Belle2
import math
import os
import shutil
import glob

reset_database()
use_central_database("Calibration_Offline_Development")


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Collect data for gain calibration from MC")
    parser.add_argument('--setnumber', default=0, type=int, help='setnumber for bg simulation')
    parser.add_argument('--bg', default='/group/belle2/BGFile/OfficialBKG/15thCampaign/phase2',
                        type=str, help='Path to folder with bg sets for mixing')
    parser.add_argument('--runNo', default=3360, type=int, help='Set run number')
    parser.add_argument('--expNo', default=3, type=int, help='Set experiment number')
    args = parser.parse_args()

    scaleFactor = 1.0
    bg = glob.glob(args.bg + '/set' + str(args.setnumber) + '/*.root')

    # Now let's create a path to run collectors
    main = create_path()

    main.add_module("EventInfoSetter", expList=[args.expNo], runList=[args.runNo], evtNumList=[1000])
    main.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
    main.add_module("Geometry")
    bkgmixer = register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    bkgmixer.param('overallScaleFactor', scaleFactor)
    main.add_module(bkgmixer)
    output = main.add_module('RootOutput')
    output.param('outputFileName', 'pxd_mc_set{}.root'.format(args.setnumber))
    output.param('outputFileName', 'beam.{:0>4}.{:0>5}.HLT2.f{:0>5}.root'.format(args.expNo, args.runNo, args.setnumber))
    output.param('branchNames', ['PXDSimHits', 'EventMetaData'])
    main.add_module("Progress")

    process(main)
    print(statistics)
