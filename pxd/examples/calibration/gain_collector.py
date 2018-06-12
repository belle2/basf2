#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file steers the collectors for collecting data for PXD gain
# calibration
#
# To create collector output for simulations
#
# basf2 gain_collector.py -n 1000
#
# To Create collector output from real data
#
# basf2 gain_collector.py -i "path to data"
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
    parser = argparse.ArgumentParser(description="Collect data for gain calibration")
    parser.add_argument('--flag', default="MC", type=str, help='set either to MC or DATA')
    args = parser.parse_args()

    # Now let's create a path to run collectors
    main = create_path()

    if not args.flag == "DATA":
        reset_database()
        use_central_database("Calibration_Offline_Development")

        rootinput = main.add_module("RootInput")
        histoman = main.add_module('HistoManager', histoFileName='PXDGainCollectorOutput_Data.root')
        gearbox = main.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
        geometry = main.add_module("Geometry", components=['PXD'])
        main.add_module("ActivatePXDPixelMasker")
        main.add_module("PXDUnpacker")
        main.add_module("PXDRawHitSorter")
        main.add_module("PXDClusterizer")
        main.add_module('PXDGainCollector', granularity="run", minClusterCharge=7, minClusterSize=1, collectSimulatedData=False)
        main.add_module("Progress")
    else:
        scaleFactor = 1.0
        bg = glob.glob('/home/benjamin/BG/phase3.mixing/set0' + '/*.root')

        main.add_module("EventInfoSetter")
        main.add_module('HistoManager', histoFileName='PXDGainCollectorOutput_MC.root')
        main.add_module("Gearbox")
        main.add_module("Geometry")
        bkgmixer = register_module('BeamBkgMixer')
        bkgmixer.param("components", ["PXD", ])
        bkgmixer.param('backgroundFiles', bg)
        bkgmixer.param('overallScaleFactor', scaleFactor)
        main.add_module(bkgmixer)
        main.add_module("PXDDigitizer")
        main.add_module("PXDClusterizer")
        main.add_module('PXDGainCollector', granularity="run", minClusterCharge=7, minClusterSize=1, collectSimulatedData=True)
        main.add_module("Progress")

    process(main)
    print(statistics)
