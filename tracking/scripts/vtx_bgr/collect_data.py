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
Script to collect data for training bg remover expert for VTX upgrade of Belle II.
Outputs a root file train.root with training data.

Usage:
export BELLE2_VTX_UPGRADE_GT=upgrade_2021-07-16_vtx_5layer
export BELLE2_VTX_BACKGROUND_DIR=/path/to/your/overlay/files/
basf2 collect_data.py -n 2000
"""


import basf2 as b2

from simulation import add_simulation
from vtx import get_upgrade_globaltag, get_upgrade_background_files
from tracking.path_utils import add_vtx_track_finding_vxdtf2, add_hit_preparation_modules
from vtx_bgr.path_utils import add_vtx_bg_collector
# from tracking import add_tracking_reconstruction


def main():
    """Main function"""

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

    # Data reconstruction
    # add_tracking_reconstruction(path, useVTX=True, use_vtx_to_cdc_ckf=True)

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
        vtx_bg_cut=0,
    )

    # Data collection for training
    add_vtx_bg_collector(
        path,
        trainingDataOutputName="train.root",
        trackCandidatesColumnName=trackCandidatesColumnName,
    )

    b2.print_path(path)

    b2.process(path)
    print(b2.statistics)


if __name__ == '__main__':
    main()
