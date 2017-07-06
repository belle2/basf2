import basf2
from ROOT import Belle2

from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.quadtree.quadTreePlotter import SegmentQuadTreePlotter
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfinderoutputcombiner.validation import add_mc_track_finder

import logging
import sys


class SegmentQuadTreeRun(StandardEventGenerationRun):

    """ Create segments and plot them in a segment quad tree """

    display_module_segments = CDCSVGDisplayModule(interactive=True)
    display_module_segments.draw_gftrackcand_trajectories = False
    display_module_segments.draw_gftrackcands = True
    display_module_segments.draw_clusters = False
    display_module_segments.track_cands_store_array_name = "MCTrackCands"

    display_module_tracks = CDCSVGDisplayModule(interactive=True)
    display_module_tracks.draw_gftrackcands = True
    display_module_tracks.track_cands_store_array_name = "TrackCands"

    display_module_mc = CDCSVGDisplayModule(interactive=True)
    display_module_mc.draw_gftrackcand_trajectories = True
    display_module_mc.draw_segments_mctrackid = True
    display_module_mc.track_cands_store_array_name = "MCTrackCands"

    plotter_module = SegmentQuadTreePlotter()
    plotter_module.draw_quad_tree_content = True
    plotter_module.draw_segment_intersection = True
    plotter_module.draw_mc_information = True

    def create_path(self):
        """ Make SegmentFinding and QuadTreeFinding and plotting/display/validation"""
        main_path = super(SegmentQuadTreeRun, self).create_path()

        add_mc_track_finder(main_path)
        main_path.add_module("TFCDC_WireHitPreparer")
        main_path.add_module("TFCDC_ClusterPreparer")

        segment_finder = basf2.register_module("TFCDC_SegmentFinderFacetAutomaton")
        segment_finder.param({
            "WriteFacets": True,
            "SegmentOrientation": "none",
        })
        main_path.add_module(segment_finder)

        main_path.add_module(Filler())

        segment_quad_tree = basf2.register_module("SegmentQuadTree")
        segment_quad_tree.param({
            "Level": 4,
            "MinimumItems": 1,
            "DoPostprocessing": True,
        })
        segment_quad_tree.set_log_level(basf2.LogLevel.DEBUG)
        segment_quad_tree.set_debug_level(100)
        main_path.add_module(segment_quad_tree)
        main_path.add_module(self.display_module_tracks)

        return main_path


def main():
    run = SegmentQuadTreeRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
