import basf2

from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.quadtree.quadTreePlotter import SegmentQuadTreePlotter
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfinderoutputcombiner.validation import add_mc_track_finder

import logging
import sys


class SegmentQuadTreeRun(StandardEventGenerationRun):

    """ Create segments and plot them in a segment quad tree """

    #: Use CDCSVGDisplay to draw the MCTrackCand segments
    display_module_segments = CDCSVGDisplayModule(
        #: \cond
        interactive=True
        #: \endcond
    )
    #: Do not draw the genfit MCTrackCand trajectories
    display_module_segments.draw_gftrackcand_trajectories = False
    #: Draw the genfit MCTrackCands
    display_module_segments.draw_gftrackcands = True
    #: Do not draw the associated clusters
    display_module_segments.draw_clusters = False
    #: Tell the plotter where to find the information
    display_module_segments.track_cands_store_array_name = "MCTrackCands"

    #: Use CDCSVGDisplay to draw the TrackCand information
    display_module_tracks = CDCSVGDisplayModule(
        #: \cond
        interactive=True
        #: \endcond
    )
    #: Draw the genfit TrackCands
    display_module_tracks.draw_gftrackcands = True
    #: Tell the plotter where to find the information
    display_module_tracks.track_cands_store_array_name = "TrackCands"

    #: Use CDCSVGDisplay to draw the MC tracks
    display_module_mc = CDCSVGDisplayModule(
        #: \cond
        interactive=True
        #: \endcond
    )
    #: Draw the genfit MCTrackCand trajectories
    display_module_mc.draw_gftrackcand_trajectories = True
    #: Draw the track IDs
    display_module_mc.draw_segments_mctrackid = True
    #: Tell the plotter where to find the information
    display_module_mc.track_cands_store_array_name = "MCTrackCands"

    #: Convert the information to QuadTree segments then draw these
    plotter_module = SegmentQuadTreePlotter()
    #: Draw the QuadTree segments
    plotter_module.draw_quad_tree_content = True
    #: Draw the segment intersections
    plotter_module.draw_segment_intersection = True
    #: Draw the MC information
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

        # main_path.add_module(Filler())

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
