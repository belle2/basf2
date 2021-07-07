#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

from trackfindingcdc.run.display import CDCDisplayRun

import logging


class CDCDebugDisplayRun(CDCDisplayRun):
    """Read generated events or generate new events then display the CDC tracks"""

    #: add the track-finder-automaton module to the basf2 path
    finder_module = basf2.register_module("TFCDC_TrackFinderAutomaton")
    # finder_module = basf2.register_module("TFCDC_SegmentFinderFacetAutomaton")
    # finder_module.param({
    #     "ClusterFilter": "mva_bkg",
    #     "ClusterFilterParameters": {"cut": 0.2},
    #     "WriteSuperClusters": True,
    #     "WriteClusters": True,
    #     "FacetFilter": "none",
    #     "FacetRelationFilter": "none",
    # })

    #: list of modules needed for track finding
    finder_module = ["TFCDC_WireHitPreparer", "TFCDC_ClusterPreparer", finder_module, "PrintCollections"]
    #: add wire-hit-preparer module to the basf2 path
    finder_module = basf2.register_module("TFCDC_WireHitPreparer")

    #: by default, show all of the drawing options
    show_all_drawoptions = True
    # Also show draw options that are related to the cellular automaton track finder
    # on the command line!


def main():
    cdcDebugDisplayRun = CDCDebugDisplayRun()
    cdcDebugDisplayRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
