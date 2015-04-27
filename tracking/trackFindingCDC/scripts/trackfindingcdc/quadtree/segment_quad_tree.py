import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

import logging
import sys

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfinderoutputcombiner.validation import MCTrackFinderRun, AddValidationMethod
from trackfindingcdc.quadtree.quadTreePlotter import SegmentQuadTreePlotter


class Filler(basf2.Module):

    def event(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()


class SegmentQuadTreeRun(MCTrackFinderRun, AddValidationMethod):
    display_module_segments = CDCSVGDisplayModule(interactive=True)
    display_module_segments.draw_segments_mctrackid = True

    display_module_tracks = CDCSVGDisplayModule(interactive=True)
    display_module_tracks.draw_gftrackcands = True
    display_module_tracks.track_cands_store_array_name = "TrackCands"

    plotter_module = SegmentQuadTreePlotter()
    plotter_module.draw_quad_tree_content = True
    plotter_module.draw_segment_intersection = False
    plotter_module.draw_segment_averaged = False
    plotter_module.draw_segment = False
    plotter_module.draw_mc_information = False
    plotter_module.draw_mc_hits = True

    def create_path(self):
        """ Make SegmentFinding and QuadTreeFinding and plotting/display/validation"""
        main_path = super(SegmentQuadTreeRun, self).create_path()

        segment_finder = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
        segment_finder.param({
            "SegmentOrientation": "none",
        })
        main_path.add_module(segment_finder)

        main_path.add_module(Filler())

        segment_quad_tree = basf2.register_module("SegmentQuadTree")
        segment_quad_tree.param({
            "Level": 4,
            "MinimumItems": 1,
        })
        segment_quad_tree.set_log_level(basf2.LogLevel.DEBUG)
        segment_quad_tree.set_debug_level(90)
        main_path.add_module(segment_quad_tree)

        # main_path.add_module(self.display_module_segments)
        main_path.add_module(self.plotter_module)
        # main_path.add_module(self.display_module_tracks)

        return main_path


def main():
    run = SegmentQuadTreeRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
