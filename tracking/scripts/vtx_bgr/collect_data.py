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
Script to collect data for training the bg remover expert for VTX upgrade of Belle II.
Outputs a root file with default name train.root with training or testing data.

Usage:

Set the correct global tag and path to background files:

export BELLE2_VTX_UPGRADE_GT=NameOfUpgradeGT
export BELLE2_VTX_BACKGROUND_DIR=/path/to/overlay/files/

Create training data:

basf2 collect_data.py -n 6000 -- --output=train.root

Create test data:

basf2 collect_data.py -n 4000 -- --output=test.root
"""


import basf2 as b2

from simulation import add_simulation
from vtx import get_upgrade_globaltag, get_upgrade_background_files
from tracking.path_utils import add_vtx_track_finding_vxdtf2, add_hit_preparation_modules
from vtx_bgr.path_utils import add_vtx_bg_collector
import argparse


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser('collect_data.py')
    add_arg = parser.add_argument
    add_arg('--output', type=str, default="train.root")
    return parser.parse_args()


def main():
    """Main function"""

    # Parse the command line
    args = parse_args()

    # Use default global tag prepended with upgrade GT to replace PXD+SVD by VTX
    b2.conditions.disable_globaltag_replay()
    b2.conditions.prepend_globaltag(get_upgrade_globaltag())

    # Set log level. Can be overridden with the "-l LEVEL" flag for basf2.
    b2.set_log_level(b2.LogLevel.WARNING)

    # ---------------------------------------------------------------------------------------
    path = b2.create_path()

    eventinfosetter = path.add_module('EventInfoSetter')
    # default phase3
    exp_number = 0
    eventinfosetter.param("expList", [exp_number])

    path.add_module('EventInfoPrinter')

    path.add_module('Progress')

    # ---------------------------------------------------------------------------------------
    # Simulation Settings:

    evtgenInput = path.add_module('EvtGenInput')
    evtgenInput.logging.log_level = b2.LogLevel.WARNING

    # ---------------------------------------------------------------------------------------

    # Detector Simulation:
    add_simulation(path, bkgfiles=get_upgrade_background_files(), useVTX=True)

    add_hit_preparation_modules(
        path,
        components=None,
        useVTX=True,
        useVTXClusterShapes=True
    )

    trackCandidatesColumnName = "RecoTracks"

    add_vtx_track_finding_vxdtf2(
        path, components=["VTX"],
        reco_tracks=trackCandidatesColumnName,
        add_mva_quality_indicator=False,
        vtx_bg_cut=None,
    )

    # Data collection for training
    add_vtx_bg_collector(
        path,
        trainingDataOutputName=args.output,
        trackCandidatesColumnName=trackCandidatesColumnName,
    )

    b2.print_path(path)

    b2.process(path)
    print(b2.statistics)


if __name__ == '__main__':
    main()