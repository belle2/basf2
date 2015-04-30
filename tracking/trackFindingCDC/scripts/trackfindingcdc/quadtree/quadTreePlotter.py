import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.colors as colors
import matplotlib.cm as cmx
import numpy as np
import subprocess
from datetime import datetime

import seaborn as sb


from trackfindingcdc.cdcdisplay.svgdrawing import attributemaps


class SegmentQuadTreePlotter(basf2.Module):

    file_name_of_quad_tree_content = "output.root"

    draw_quad_tree_content = True and False
    draw_segment_intersection = True and False
    draw_segment = True and False
    draw_segment_averaged = True and False
    draw_segment_fitted = True and False
    draw_mc_information = True and False
    draw_mc_hits = True and False

    theta_shifted = False
    maximum_theta = np.pi

    def plotQuadTreeContent(self, input_file):
        hist = input_file.Get("histUsed")
        histUnused = input_file.Get("histUnused")

        xList = list()
        yList = list()

        xAxis = hist.GetXaxis()
        yAxis = hist.GetYaxis()

        length_list = []
        width_list = []
        height_list = []
        x_list = []
        y_list = []

        for iX in xrange(1, xAxis.GetNbins() + 1):
            for iY in xrange(1, yAxis.GetNbins() + 1):
                length_list += [hist.GetBinContent(iX, iY) + histUnused.GetBinContent(iX, iY)]
                width_list += [xAxis.GetBinWidth(iX) / 8192 * self.maximum_theta]
                height_list += [yAxis.GetBinWidth(iY)]
                y_list += [yAxis.GetBinLowEdge(iY)]
                x_list += [xAxis.GetBinLowEdge(iX) / 8192 * self.maximum_theta]

        cm = plt.get_cmap('Blues')
        cNorm = colors.Normalize(vmin=0, vmax=max(length_list))
        scalarMap = cmx.ScalarMappable(norm=cNorm, cmap=cm)

        for length, height, width, x, y in zip(length_list, height_list, width_list, x_list, y_list):
            patch = patches.Rectangle((x, y), width=width, height=height, facecolor=scalarMap.to_rgba(length), lw=1)
            plt.gca().add_patch(patch)

    def calculateIntersectionInQuadTreePicture(self, first, second):
        positionFront = first.getRecoPos2D().conformalTransformed()
        positionBack = second.getRecoPos2D().conformalTransformed()

        theta_cut = np.arctan2((positionBack - positionFront).x(), (positionFront - positionBack).y())

        if self.theta_shifted:
            while theta_cut < - self.maximum_theta / 2:
                theta_cut += self.maximum_theta
        else:
            while theta_cut < 0:
                theta_cut += self.maximum_theta

        r_cut = positionFront.x() * np.cos(theta_cut) + positionFront.y() * np.sin(theta_cut)

        return theta_cut, r_cut

    def calculatePositionInQuadTreePicture(self, position):
        position = position.conformalTransformed()

        theta = np.linspace(-self.maximum_theta / 2, self.maximum_theta / 2, 100)
        r = position.x() * np.cos(theta) + position.y() * np.sin(theta)

        return theta, r

    def forAllAxialSegments(self, f):
        items = Belle2.PyStoreObj("CDCRecoSegment2DVector")
        wrapped_vector = items.obj()
        vector = wrapped_vector.get()

        for quad_tree_item in vector:
            if quad_tree_item.getStereoType() == 0:
                f(quad_tree_item)

    def convertToQuadTreePicture(self, phi, mag, charge):
        theta = phi + np.pi / 2
        r = 1 / mag * 1.5 * 0.00299792458 * charge
        if self.theta_shifted:
            if theta > self.maximum_theta / 2 or theta < -self.maximum_theta / 2:
                theta = theta % self.maximum_theta - self.maximum_theta / 2
            else:
                r *= -1
        else:
            if theta > self.maximum_theta or theta < 0:
                theta = theta % self.maximum_theta
            else:
                r *= -1
        return theta, r

    def event(self):
        plt.clf()

        if self.theta_shifted:
            plt.xlim(-self.maximum_theta / 2, self.maximum_theta / 2)
        else:
            plt.xlim(0, self.maximum_theta)
        plt.ylim(-0.05, 0.05)
        plt.xticks([])
        plt.yticks([])

        if self.draw_quad_tree_content:
            input_file = ROOT.TFile(self.file_name_of_quad_tree_content)
            self.plotQuadTreeContent(input_file)

        if self.draw_segment_intersection:
            map = attributemaps.SegmentMCTrackIdColorMap()

            def f(segment):
                theta, r = self.calculateIntersectionInQuadTreePicture(segment.front(), segment.back())
                plt.plot(theta, r, color=map(0, segment), marker="o")

            self.forAllAxialSegments(f)

        if self.draw_segment:
            map = attributemaps.SegmentMCTrackIdColorMap()

            def f(segment):
                theta, r = self.calculatePositionInQuadTreePicture(segment.front().getRecoPos2D())
                plt.plot(theta, r, color=map(0, segment), marker="", ls="-")

            self.forAllAxialSegments(f)

        if self.draw_segment_averaged:
            map = attributemaps.SegmentMCTrackIdColorMap()

            def f(segment):
                middle_index = int(np.round(segment.size() / 2.0))
                middle_point = segment.items()[middle_index]
                theta_front, r_front = self.calculateIntersectionInQuadTreePicture(segment.front(), middle_point)
                theta_back, r_back = self.calculateIntersectionInQuadTreePicture(middle_point, segment.back())

                plt.plot([theta_front, theta_back], [r_front, r_back], color=map(0, segment), marker="o", ls="-")

            self.forAllAxialSegments(f)

        if self.draw_segment_fitted:
            map = attributemaps.SegmentMCTrackIdColorMap()
            fitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getOriginCircleFitter()

            def f(segment):
                trajectory = fitter.fit(segment)
                momentum = trajectory.getUnitMom2D(Belle2.TrackFindingCDC.Vector2D(0, 0)).scale(trajectory.getAbsMom2D())
                theta, r = self.convertToQuadTreePicture(momentum.phi(), momentum.norm(), trajectory.getChargeSign())
                plt.plot(theta, r, color=map(0, segment), marker="o")

            self.forAllAxialSegments(f)

        if self.draw_mc_hits:
            wireHitTopology = Belle2.TrackFindingCDC.CDCWireHitTopology.getInstance()
            map = attributemaps.listColors
            array = Belle2.PyStoreArray("MCTrackCands")
            cdcHits = Belle2.PyStoreArray("CDCHits")

            for track in array:
                mcTrackID = track.getMcTrackId()

                for cdcHitID in track.getHitIDs(Belle2.Const.CDC):
                    cdcHit = cdcHits[cdcHitID]
                    wireHit = wireHitTopology.getWireHit(cdcHit)

                    theta, r = self.calculatePositionInQuadTreePicture(wireHit.getRefPos2D())

                    plt.plot(theta, r, marker="", color=map[mcTrackID % len(map)], ls="-", alpha=0.2)

        if self.draw_mc_information:
            map = attributemaps.listColors
            array = Belle2.PyStoreArray("MCTrackCands")

            for track in array:
                momentum = track.getMomSeed()

                # HARDCODED!!! Temporary solution
                theta, r = self.convertToQuadTreePicture(momentum.Phi(), momentum.Mag(), track.getChargeSeed())
                mcTrackID = track.getMcTrackId()

                plt.plot(theta, r, marker="o", color="black", ms=10)
                plt.plot(theta, r, marker="o", color=map[mcTrackID % len(map)], ms=5)

        self.save_and_show_file()

    def save_and_show_file(self):
        fileName = "/tmp/" + datetime.now().isoformat() + '.svg'
        plt.savefig(fileName)
        procDisplay = subprocess.Popen(['eog', fileName])
        raw_input("Press Enter to continue..")
