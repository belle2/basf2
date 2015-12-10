#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

from trackfindingcdc.run.display import CDCDisplayRun

import logging


class CDCDebugDisplayRun(CDCDisplayRun):
    # finder_module = basf2.register_module("TrackFinderCDCAutomatonDev")
    finder_module = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
    finder_module.param({
        "ClusterFilter": "tmva",
        "ClusterFilterParameters": {"cut": "0.2"},
        "WriteSuperClusters": True,
        "WriteClusters": True,
        "RemainingCDCHitsStoreArrayName": "GoodCDCHits",
        "FacetFilter": "none",
        "FacetRelationFilter": "none",
        # "CreateGFTrackCands" : True,
        # "SegmentOrientation": "none",
    })

    finder_module = [finder_module, "PrintCollections"]
    show_all_drawoptions = True
    # Also show draw options that are related to the cellular automaton track finder
    # on the command line!


def main():
    cdcDebugDisplayRun = CDCDebugDisplayRun()
    cdcDebugDisplayRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
