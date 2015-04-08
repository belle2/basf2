import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
from trackfinderoutputcombiner.validation import MCTrackFinderRun
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.validation import MCTrackFinderRun
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners
import logging
import sys

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfindingcdc.cdcdisplay.svgdrawing.attributemaps import CDCSegmentColorMap
CDCSegmentColorMap.bkgSegmentColor = "black"

try:
    import root_pandas
except ImportError:
    print "do a pip install git+https://github.com/ibab/root_pandas"
import pandas
import matplotlib.pyplot as plt
import numpy as np


class SegmentQuadTreeRun(MCTrackFinderRun):

    display_module_segments = CDCSVGDisplayModule(interactive=True)
    display_module_segments.draw_gftrackcand_trajectories = True
    display_module_segments.draw_gftrackcands = False
    display_module_segments.track_cands_store_array_name = "MCTrackCands"
    display_module_segments.draw_segments_id = True
    CDCSegmentColorMap

    def create_path(self):
        main_path = super(SegmentQuadTreeRun, self).create_path()

        segment_finder = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
        main_path.add_module(segment_finder)

        segment_quad_tree = basf2.register_module("SegmentQuadTree")
        segment_quad_tree.param("RecoSegments", "CDCRecoSegment2DVector")
        segment_quad_tree.set_log_level(basf2.LogLevel.DEBUG)
        segment_quad_tree.set_debug_level(100)
        main_path.add_module(segment_quad_tree)

        # main_path.add_module(self.display_module_segments)

        return main_path


def main():
    run = SegmentQuadTreeRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
