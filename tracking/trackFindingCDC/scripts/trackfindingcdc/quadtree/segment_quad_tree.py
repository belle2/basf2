import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.validation import MCTrackFinderRun, AddValidationMethod
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
from datetime import datetime
import subprocess

import IPython

from trackfindingcdc.cdcdisplay.svgdrawing.attributemaps import SegmentMCTrackIdColorMap
from trackfindingcdc.quadtree.quadTreePlotter import SegmentQuadTreePlotter


class Filler(basf2.Module):

    def event(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()


class SegmentPlotter(basf2.Module):

    """ Small helper to draw the sinograms of the segments """

    def event(self):
        """ Do the drawing """
        plt.clf()

        segments = Belle2.PyStoreObj("CDCRecoSegment2DVector")

        map = SegmentMCTrackIdColorMap()

        # Wrapper around std::vector like
        wrapped_vector = segments.obj()
        vector = wrapped_vector.get()

        if self.plot_conformal:

            for segment in vector:
                if segment.getStereoType() == 0:
                    positionFront = segment.front().getRecoPos2D().conformalTransformed()
                    positionBack = segment.back().getRecoPos2D().conformalTransformed()
                    plt.plot([positionFront.x(), positionBack.x()], [positionFront.y(), positionBack.y()],
                             ls="-", marker="o", color=map(0, segment))


class SegmentQuadTreeRun(MCTrackFinderRun, AddValidationMethod):

    display_module_segments = CDCSVGDisplayModule(interactive=True)
    display_module_segments.draw_gftrackcand_trajectories = False
    display_module_segments.draw_gftrackcands = True
    display_module_segments.draw_clusters = False
    display_module_segments.track_cands_store_array_name = "MCTrackCands"

    display_module_tracks = CDCSVGDisplayModule(interactive=True)
    display_module_tracks.draw_gftrackcand_trajectories = True
    display_module_tracks.draw_gftrackcands = True
    display_module_tracks.track_cands_store_array_name = "LegendreTrackCands"

    display_module_mc = CDCSVGDisplayModule(interactive=True)
    display_module_mc.draw_gftrackcand_trajectories = True
    display_module_mc.draw_gftrackcands = True
    display_module_mc.track_cands_store_array_name = "MCTrackCands"

    plotter_module = SegmentQuadTreePlotter()

    def create_path(self):
        """ Make SegmentFinding and QuadTreeFinding and plotting/display/validation"""
        main_path = super(SegmentQuadTreeRun, self).create_path()

        segment_finder = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
        segment_finder.param({
            "WriteFacets": True,
            "SegmentOrientation": "none",
        })
        main_path.add_module(segment_finder)

        main_path.add_module(Filler())

        segment_quad_tree = basf2.register_module("SegmentQuadTree")
        segment_quad_tree.param({
            "Level": 6,
            "MinimumItems": 1,
        })
        segment_quad_tree.set_log_level(basf2.LogLevel.ERROR)
        main_path.add_module(segment_quad_tree)

        hit_quad_tree = basf2.register_module("CDCLegendreTracking")
        hit_quad_tree.param("GFTrackCandidatesColName", "LegendreTrackCands")
        # main_path.add_module(hit_quad_tree)

        # self.create_validation(main_path, "TrackCands", "SegmentQuadTree.root")
        # self.create_validation(main_path, "LegendreTrackCands", "HitQuadTree.root")

        # main_path.add_module(self.plotter_module)
        # main_path.add_module(self.display_module_tracks)
        # main_path.add_module(self.display_module_mc)

        return main_path


def main():
    run = SegmentQuadTreeRun()
    run.n_events = 1
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
