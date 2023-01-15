#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
Script to collect data for training the track merger for VTX upgrade of Belle II.
The output is a folder with extracted training data for each simulated event in
HDF5 format.

Usage:

Set the correct global tag and path to background files:

export BELLE2_VTX_UPGRADE_GT=NameOfUpgradeGT
export BELLE2_VTX_BACKGROUND_DIR=/path/to/overlay/files/

Create training data:

basf2 collect_data.py -n 2000 -- --outputdir=/path/to/folder
"""
import basf2 as b2
from simulation import add_simulation
from vtx import get_upgrade_globaltag, get_upgrade_background_files
import tracking
import argparse
from pathlib import Path
import shutil

rndseed = 13245


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser('collect_data.py')
    add_arg = parser.add_argument
    add_arg('--outputdir', type=str, default="/home/benjamin/vtx_cdc_merger_data")
    return parser.parse_args()


def main():
    """Main function"""

    # Parse the command line
    args = parse_args()

    # Prepare output
    shutil.rmtree(args.outputdir, ignore_errors=True)
    Path(args.outputdir).mkdir(parents=True, exist_ok=True)

    # Use default global tag prepended with upgrade GT to replace PXD+SVD by VTX
    b2.conditions.disable_globaltag_replay()
    b2.conditions.prepend_globaltag(get_upgrade_globaltag())

    # Set log level. Can be overridden with the "-l LEVEL" flag for basf2.
    b2.set_log_level(b2.LogLevel.WARNING)

    # Set random seed
    b2.set_random_seed(rndseed)

    # ---------------------------------------------------------------------------------------
    path = b2.create_path()

    eventinfosetter = path.add_module('EventInfoSetter')
    eventinfosetter.param("expList", [0])

    path.add_module('EventInfoPrinter')

    path.add_module('Progress')

    # --------------------------------------------------------------------------
    # Simulation Settings

    evtgenInput = path.add_module('EvtGenInput')
    evtgenInput.logging.log_level = b2.LogLevel.WARNING

    # --------------------------------------------------------------------------
    # Detector Simulation
    add_simulation(path, bkgfiles=get_upgrade_background_files(), useVTX=True)

    # ---------------------------------------------------------------------------
    # Tracking and data collection
    tracking.add_tracking_reconstruction(path, useVTX=True,
                                         use_vtx_to_cdc_ckf=True, use_mc_vtx_cdc_merger=True)

    for e in path.modules():
        if e.name() == "VTXCDCMergerCollector":
            e.outputdir = args.outputdir

    b2.print_path(path)

    b2.process(path)
    print(b2.statistics)


if __name__ == '__main__':
    main()
