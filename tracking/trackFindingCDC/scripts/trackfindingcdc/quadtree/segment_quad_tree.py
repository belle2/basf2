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

level = 1


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


class SegmentQuadTreePlotter(basf2.Module):

    draw_quad_tree_content = True

    draw_segment_content = True

    def plotIntersectionInQuadTreePicture(self, first, second, color):
        theta_cut, r_cut = self.calculateIntersectionInQuadTreePicture(first, second)
        plt.plot(theta_cut, r_cut, color=color, marker="o")

    def calculateIntersectionInQuadTreePicture(self, first, second):
        positionFront = first.getRecoPos2D().conformalTransformed()
        positionBack = second.getRecoPos2D().conformalTransformed()

        theta_cut = np.arctan2((positionBack - positionFront).x(), (positionFront - positionBack).y())

        while theta_cut < 0:
            theta_cut += np.pi

        r_cut = positionFront.x() * np.cos(theta_cut) + positionFront.y() * np.sin(theta_cut)

        return theta_cut, r_cut

    def plotQuadTreeContent(self, output):
        import matplotlib.patches as patches
        import matplotlib.colors as colors
        import matplotlib.cm as cmx

        hist = output.Get("histUsed")
        histUnused = output.Get("histUsed")

        xList = list()
        yList = list()

        xAxis = hist.GetXaxis()
        yAxis = hist.GetYaxis()

        cm = plt.get_cmap('jet')
        cNorm = colors.Normalize(vmin=0, vmax=50)
        scalarMap = cmx.ScalarMappable(norm=cNorm, cmap=cm)

        for iX in xrange(1, xAxis.GetNbins() + 1):
            for iY in xrange(1, yAxis.GetNbins() + 1):
                length = hist.GetBinContent(iX, iY) + histUnused.GetBinContent(iX, iY)
                width = xAxis.GetBinWidth(iX) / 8192 * np.pi
                height = yAxis.GetBinWidth(iY)
                y = yAxis.GetBinLowEdge(iY)
                x = xAxis.GetBinLowEdge(iX) / 8192 * np.pi

                patch = patches.Rectangle((x, y), width=width, height=height, facecolor=scalarMap.to_rgba(length), lw=1)
                plt.gca().add_patch(patch)

    def event(self):
        map = SegmentMCTrackIdColorMap()

        mcParticles = Belle2.PyStoreArray("MCTrackCands")

        mcSegmentLookUp = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()

        plt.clf()

        if self.draw_quad_tree_content:
            output = ROOT.TFile("output.root")
            self.plotQuadTreeContent(output)

        if self.draw_segment_content:
            segments = Belle2.PyStoreObj("CDCRecoSegment2DVector")
            wrapped_vector = segments.obj()
            vector = wrapped_vector.get()

            for segment in vector:
                if segment.getStereoType() == 0:
                    lastHit = None

                    theta_avg = 0
                    r_avg = 0

                    for hit in segment.items():
                        if lastHit is not None:
                            theta, r = self.calculateIntersectionInQuadTreePicture(lastHit, hit)
                            theta_avg += theta
                            r_avg += r

                        lastHit = hit

                    plt.plot(theta_avg / len(segment), r_avg / len(segment), color=map(0, segment), marker="o")

                    mcTrackId = mcSegmentLookUp.getMCTrackId(segment)
                    mcTrack = mcParticles[mcTrackId]

        plt.xlim(0, np.pi)
        plt.ylim(-0.05, 0.05)
        fileName = datetime.now().isoformat() + '.svg'
        plt.savefig(fileName)
        procDisplay = subprocess.Popen(['eog', fileName])
        raw_input("Press Enter to continue..")


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
    plotter_module.draw_quad_tree_content = False

    def create_path(self):
        """ Make SegmentFinding and QuadTreeFinding and plotting/display/validation"""
        main_path = super(SegmentQuadTreeRun, self).create_path()

        segment_finder = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
        segment_finder.param({
            "WriteSuperClusters": False,
            "WriteClusters": False,
            "WriteFacets": True,
            "SegmentOrientation": "none",
        })
        main_path.add_module(segment_finder)

        main_path.add_module(Filler())

        segment_quad_tree = basf2.register_module("SegmentQuadTree")
        segment_quad_tree.param({
            "Level": level,
            "MinimumItems": 1,
        })
        segment_quad_tree.set_log_level(basf2.LogLevel.DEBUG)
        segment_quad_tree.set_debug_level(90)
        main_path.add_module(segment_quad_tree)

        hit_quad_tree = basf2.register_module("CDCLegendreTracking")
        hit_quad_tree.param("GFTrackCandidatesColName", "LegendreTrackCands")
        main_path.add_module(hit_quad_tree)

        # self.create_validation(main_path, "TrackCands", "SegmentQuadTree.root")
        # self.create_validation(main_path, "LegendreTrackCands", "HitQuadTree.root")

        main_path.add_module(self.plotter_module)
        # main_path.add_module(self.display_module_tracks)
        # main_path.add_module(self.display_module_mc)

        return main_path


def main():
    run = SegmentQuadTreeRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
