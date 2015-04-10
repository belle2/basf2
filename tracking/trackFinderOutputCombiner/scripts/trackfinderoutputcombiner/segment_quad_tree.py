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
from datetime import datetime
import subprocess

from trackfindingcdc.cdcdisplay.svgdrawing.attributemaps import SegmentMCTrackIdColorMap


class SegmentQuadTreePlotter(basf2.Module):

    plot_conformal = True

    def event(self):
        plt.clf()

        segments = Belle2.PyStoreObj("CDCRecoSegment2DVector")

        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

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

        else:

            f, axarr = plt.subplots(2)

            for segment in vector:
                if segment.getStereoType() == 0:

                    positionFront = segment.front().getRecoPos2D().conformalTransformed()
                    positionBack = segment.back().getRecoPos2D().conformalTransformed()

                    theta_cut = np.arctan2((positionBack - positionFront).x(), (positionFront - positionBack).y())

                    if theta_cut < 0:
                        theta_cut += np.pi

                    theta = np.linspace(max(theta_cut - 0.2, 0), min(theta_cut + 0.2, np.pi), 20)
                    cos_theta = np.cos(theta)
                    sin_theta = np.sin(theta)

                    axarr[0].plot(
                        theta,
                        positionFront.x() *
                        cos_theta +
                        positionFront.y() *
                        sin_theta,
                        ls="-",
                        color=map(
                            0,
                            segment))
                    axarr[0].plot(theta, positionBack.x() * cos_theta + positionBack.y() * sin_theta, ls="-", color="black")
                    axarr[0].plot(
                        theta_cut,
                        positionFront.x() *
                        np.cos(theta_cut) +
                        positionFront.y() *
                        np.sin(theta_cut),
                        color="blue",
                        marker="o")

                    axarr[1].plot(
                        theta,
                        positionFront.x() *
                        sin_theta -
                        positionFront.y() *
                        cos_theta,
                        ls="-",
                        color=map(
                            0,
                            segment))
                    axarr[1].plot(theta, positionBack.x() * sin_theta - positionBack.y() * cos_theta, ls="-", color="black")
                    axarr[1].plot(
                        theta_cut,
                        positionFront.x() *
                        np.sin(theta_cut) -
                        positionFront.y() *
                        np.cos(theta_cut),
                        color="blue",
                        marker="o")

        fileName = datetime.now().isoformat() + '.svg'
        plt.savefig(fileName)
        procDisplay = subprocess.Popen(['eog', fileName])
        raw_input('Hit enter for next event')


class SegmentQuadTreeRun(MCTrackFinderRun):

    display_module_segments = CDCSVGDisplayModule(interactive=True)
    display_module_segments.draw_gftrackcand_trajectories = False
    display_module_segments.draw_gftrackcands = True
    display_module_segments.track_cands_store_array_name = "TrackCands"
    display_module_segments.draw_segments_mctrackid = False

    plotter_module = SegmentQuadTreePlotter()
    plotter_module.plot_conformal = False

    def create_path(self):
        main_path = super(SegmentQuadTreeRun, self).create_path()

        segment_finder = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
        main_path.add_module(segment_finder)

        segment_quad_tree = basf2.register_module("SegmentQuadTree")
        segment_quad_tree.param({
            "Level": 10,
            "MinimumItems": 10,
        })
        segment_quad_tree.set_log_level(basf2.LogLevel.DEBUG)
        segment_quad_tree.set_debug_level(90)
        main_path.add_module(segment_quad_tree)

        # main_path.add_module(self.plotter_module)
        # main_path.add_module(self.display_module_segments)

        return main_path


def main():
    run = SegmentQuadTreeRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
