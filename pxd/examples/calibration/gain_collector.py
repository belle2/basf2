#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file steers the collectors for collecting data for PXD gain
# calibration
#
# Execute as: basf2 gain_collector.py -i input.root
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
from ROOT import Belle2
import math
import os
import shutil


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Collect data for gain calibration")
    args = parser.parse_args()

    # Now let's create a path to run collectors
    main = create_path()

    rootinput = main.add_module("RootInput")
    histoman = main.add_module('HistoManager', histoFileName='PXDGainCollectorOutput.root')
    gearbox = main.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
    geometry = main.add_module("Geometry", components=['PXD'])
    main.add_module("ActivatePXDPixelMasker")
    main.add_module("PXDUnpacker")
    main.add_module("PXDRawHitSorter")
    main.add_module("PXDClusterizer")
    main.add_module('PXDGainCollector', granularity="run", minClusterCharge=7, minClusterSize=1)
    main.add_module("Progress")

    process(main)
    print(statistics)
