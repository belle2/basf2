#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file prepares CAF collector outputs from MC simulations for
# PXD gain calibrations. At the moment, gain calibration is only supported
# for phase2. The input for collecting data are mixed background simulations.
#
# basf2 gain_collector_mc.py -n 1000 -- --setnumber=0
#
# In order to compute gains on a 4x6 grid per sensor, we need to execute the
# script x5 on different sets (0,1,2,3,4).
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de


from basf2 import *
from ROOT import Belle2
import math
import os
import shutil
import glob

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Collect data for gain calibration from MC")
    parser.add_argument('--setnumber', default=0, type=int, help='setnumber for bg simulation')
    parser.add_argument('--bg', default='/group/belle2/BGFile/OfficialBKG/15thCampaign/phase2',
                        type=str, help='Path to folder with bg sets for mixing')
    args = parser.parse_args()

    scaleFactor = 1.0
    bg = glob.glob(args.bg + '/set' + str(args.setnumber) + '/*.root')

    # Now let's create a path to run collectors
    main = create_path()

    main.add_module("EventInfoSetter")
    main.add_module('HistoManager', histoFileName='PXDGainCollectorOutput_MC_set{}.root'.format(args.setnumber))
    main.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
    main.add_module("Geometry")
    bkgmixer = register_module('BeamBkgMixer')
    bkgmixer.param('backgroundFiles', bg)
    bkgmixer.param('overallScaleFactor', scaleFactor)
    main.add_module(bkgmixer)
    main.add_module("PXDDigitizer")
    main.add_module("PXDClusterizer")
    main.add_module(
        'PXDGainCollector',
        granularity="run",
        minClusterCharge=8,
        minClusterSize=2,
        maxClusterSize=6,
        collectSimulatedData=True,
        nBinsU=4,
        nBinsV=6)

    # main.add_module('RootOutput', outputFileName='mixed_bg_set{}.root'.format(args.setnumber))
    main.add_module("Progress")

    process(main)
    print(statistics)
