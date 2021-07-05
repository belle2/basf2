##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from trackfindingcdc.cdcdisplay.svgdrawing import attributemaps
import bisect
from datetime import datetime
import numpy as np
import matplotlib.pyplot as plt
import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')


class QuadTreePlotter(basf2.Module):
    """
    This Module is able to draw the content coming from a QuadTreeImplementation with debugOutput = True.
    """

    def __init__(self, queue):
        """
        Do not forget to set the ranges! Otherwise you will end up with an empty plot..
        """
        basf2.Module.__init__(self)
        #: cached output filename
        self.file_name_of_quad_tree_content = "output.root"
        #: cached flag to draw QuadTree
        self.draw_quad_tree_content = True
        #: cached minimum x value
        self.range_x_min = 0
        #: cached maximum x value
        self.range_x_max = 0
        #: cached minimum y value
        self.range_y_min = 0
        #: cached maximum y value
        self.range_y_max = 0

        #: cached value of the queue input parameter
        self.queue = queue
        #: cached array of output filenames (one file per image)
        self.file_names = []

    def plot_quad_tree_content(self):
        """
        Draw the quad tree content coming from the root file if enabled.
        """

        import seaborn as sb

        if not self.draw_quad_tree_content:
            return

        input_file = ROOT.TFile(self.file_name_of_quad_tree_content)

        hist = input_file.Get("histUsed")

        xAxis = hist.GetXaxis()
        yAxis = hist.GetYaxis()

        x_edges = np.array([xAxis.GetBinLowEdge(iX) for iX in range(1, xAxis.GetNbins() + 2)])
        y_edges = np.array([yAxis.GetBinLowEdge(iY) for iY in range(1, yAxis.GetNbins() + 2)])

        arr_l = np.array([[hist.GetBinContent(iX, iY) for iY in range(1, yAxis.GetNbins() + 1)]
                          for iX in range(1, xAxis.GetNbins() + 1)])

        hist = input_file.Get("histUnused")

        xAxis = hist.GetXaxis()
        yAxis = hist.GetYaxis()

        x_edges = np.array([xAxis.GetBinLowEdge(iX) for iX in range(1, xAxis.GetNbins() + 2)])
        y_edges = np.array([yAxis.GetBinLowEdge(iY) for iY in range(1, yAxis.GetNbins() + 2)])

        l2 = np.array([[hist.GetBinContent(iX, iY) for iY in range(1, yAxis.GetNbins() + 1)]
                       for iX in range(1, xAxis.GetNbins() + 1)])

        cmap = sb.cubehelix_palette(8, start=2, rot=0, dark=0, light=1, reverse=False, as_cmap=True)

        plt.gca().pcolorfast(x_edges, y_edges, (arr_l + l2).T, cmap=cmap)

        x_labels = ["{1:0.{0}f}".format(int(not float(x).is_integer()), x) if i % 4 == 0 else "" for i, x in enumerate(x_edges)]
        plt.xticks(x_edges, x_labels)
        y_labels = ["{1:0.{0}f}".format(int(not float(y).is_integer()), y) if i % 4 == 0 else "" for i, y in enumerate(y_edges)]
        plt.yticks(y_edges, y_labels)

    def save_and_show_file(self):
        """
        Save the plot to a svg and show it (maybe a png would be better?)
        """
        fileName = "/tmp/" + datetime.now().isoformat() + '.svg'
        plt.savefig(fileName)
        self.file_names.append(fileName)

    def init_plotting(self):
        """
        Initialize the figure with the plot ranges
        We need to implement axes labels later!
        """
        plt.clf()
        plt.xlim(self.range_x_min, self.range_x_max)
        plt.ylim(self.range_y_min, self.range_y_max)

    def event(self):
        """
        Draw everything
        """
        self.init_plotting()
        self.plot_quad_tree_content()
        self.save_and_show_file()

    def terminate(self):
        """Termination signal at the end of the event processing"""
        self.queue.put("quadTree", self.file_names)


class SegmentQuadTreePlotter(QuadTreePlotter):

    """
    Implementation of a quad tree plotter for SegmentQuadTrees
    """

    #: by default, do not draw a segment intersection
    draw_segment_intersection = True and False
    #: by default, do not draw a segment
    draw_segment = True and False
    #: by default, do not draw an averaged segment
    draw_segment_averaged = True and False
    #: by default, do not draw a fitted segment
    draw_segment_fitted = True and False
    #: by default, do not draw the MC information
    draw_mc_information = True and False
    #: by default, do not draw the MC hits
    draw_mc_hits = True and False

    #: by default, polar angles and cuts are in the range (0,pi) rather than (-pi/2,+pi/2)
    theta_shifted = False
    #: an alias for the maximum value of the polar angle
    maximum_theta = np.pi

    def calculateIntersectionInQuadTreePicture(self, first, second):
        """
        Calculate the point where the two given hits intersect

        params
        ------
        first: hit
        second: hit
        """
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
        """
        Transform a given normal coordinate position to a legendre position (conformal transformed)

        params
        ------
        position: TrackFindingCDC.Vector2D
        """
        position = position.conformalTransformed()

        theta = np.linspace(self.range_x_min, self.range_x_max, 100)
        r = position.x() * np.cos(theta) + position.y() * np.sin(theta)

        return theta, r

    def forAllAxialSegments(self, f):
        """
        Loop over all segments and execute a function

        params
        ------
        f: function
        """
        items = Belle2.PyStoreObj("CDCSegment2DVector")
        wrapped_vector = items.obj()
        vector = wrapped_vector.get()

        for quad_tree_item in vector:
            if quad_tree_item.getStereoType() == 0:
                f(quad_tree_item)

    def convertToQuadTreePicture(self, phi, mag, charge):
        """
        Convert given track parameters into a point in the legendre space

        params
        ------
        phi: phi of the track
        mag: magnitude of pt
        charge: charge of the track
        """
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
        """
        Draw everything according to the given options

        Attributes
        ----------
        draw_segment_intersection
        draw_segment
        draw_segment_averaged
        draw_segment_fitted
        draw_mc_information
        draw_mc_hits
        """
        if self.theta_shifted:
            #: lower x bound for polar angle
            self.range_x_min = -self.maximum_theta / 2
            #: upper x bound for polar angle
            self.range_x_max = self.maximum_theta / 2
        else:
            self.range_x_min = 0
            self.range_x_max = self.maximum_theta

        #: lower y bound
        self.range_y_min = -0.08
        #: upper y bound
        self.range_y_max = 0.08

        self.init_plotting()
        # self.plotQuadTreeContent()

        if self.draw_segment_intersection:
            map = attributemaps.SegmentMCTrackIdColorMap()

            def f(segment):
                theta, r = self.calculateIntersectionInQuadTreePicture(segment.front(), segment.back())
                plt.plot(theta, r, color=list(map(0, segment)), marker="o")

            self.forAllAxialSegments(f)

        if self.draw_segment:
            map = attributemaps.SegmentMCTrackIdColorMap()

            def f(segment):
                theta, r = self.calculatePositionInQuadTreePicture(segment.front().getRecoPos2D())
                plt.plot(theta, r, color=list(map(0, segment)), marker="", ls="-")

            self.forAllAxialSegments(f)

        if self.draw_segment_averaged:
            map = attributemaps.SegmentMCTrackIdColorMap()

            def f(segment):
                middle_index = int(np.round(segment.size() / 2.0))
                middle_point = list(segment.items())[middle_index]
                theta_front, r_front = self.calculateIntersectionInQuadTreePicture(segment.front(), middle_point)
                theta_back, r_back = self.calculateIntersectionInQuadTreePicture(middle_point, segment.back())

                plt.plot([theta_front, theta_back], [r_front, r_back], color=list(map(0, segment)), marker="o", ls="-")

            self.forAllAxialSegments(f)

        if self.draw_segment_fitted:
            map = attributemaps.SegmentMCTrackIdColorMap()
            fitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getOriginCircleFitter()

            def f(segment):
                trajectory = fitter.fit(segment)
                momentum = trajectory.getUnitMom2D(Belle2.TrackFindingCDC.Vector2D(0, 0)).scale(trajectory.getAbsMom2D())
                theta, r = self.convertToQuadTreePicture(momentum.phi(), momentum.norm(), trajectory.getChargeSign())
                plt.plot(theta, r, color=list(map(0, segment)), marker="o")

            self.forAllAxialSegments(f)

        if self.draw_hits:
            cdcHits = Belle2.PyStoreArray("CDCHits")
            storedWireHits = Belle2.PyStoreObj('CDCWireHitVector')
            wireHits = storedWireHits.obj().get()

            array = Belle2.PyStoreArray("MCTrackCands")
            cdc_hits = [cdcHits[i] for track in array for i in track.getHitIDs()]

            for cdcHit in cdcHits:
                if cdcHit in cdc_hits:
                    continue
                wireHit = wireHits.at(bisect.bisect_left(wireHits, cdcHit))
                theta, r = self.calculatePositionInQuadTreePicture(wireHit.getRefPos2D())

                plt.plot(theta, r, marker="", color="black", ls="-", alpha=0.8)

        if self.draw_mc_hits:
            storedWireHits = Belle2.PyStoreObj('CDCWireHitVector')
            wireHits = storedWireHits.obj().get()

            map = attributemaps.listColors
            array = Belle2.PyStoreArray("MCTrackCands")
            cdcHits = Belle2.PyStoreArray("CDCHits")

            for track in array:
                mcTrackID = track.getMcTrackId()

                for cdcHitID in track.getHitIDs(Belle2.Const.CDC):
                    cdcHit = cdcHits[cdcHitID]
                    wireHit = wireHits.at(bisect.bisect_left(wireHits, cdcHit))

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


class StereoQuadTreePlotter(QuadTreePlotter):

    """
    Implementation of a quad tree plotter for StereoHitAssignment
    """

    #: by default, do not draw the MC hits
    draw_mc_hits = False
    #: by default, do not draw the MC tracks
    draw_mc_tracks = False
    #: by default, do not draw the track hits
    draw_track_hits = False
    #: by default, draw the last track
    draw_last_track = True
    #: by default, do not delete the bad track hits
    delete_bad_hits = False

    def create_trajectory_from_track(self, track):
        """
        Convert a genfit::TrackCand into a TrackFindingCDC.CDCTrajectory3D

        params
        ------
        track: genfit::TrackCand
        """
        Vector3D = Belle2.TrackFindingCDC.Vector3D
        Trajectory3D = Belle2.TrackFindingCDC.CDCTrajectory3D

        position = Vector3D(track.getPosSeed())
        momentum = Vector3D(track.getMomSeed())
        charge = track.getChargeSeed()

        return Trajectory3D(position, momentum, charge)

    def create_reco_hit3D(self, cdcHit, trajectory3D, rlInfo):
        """
        Use a cdc hit and a trajectory to reconstruct a CDCRecoHit3D

        params
        ------
        cdcHit: CDCHit
        trajectory3D: TrackFindingCDC.CDCTrajectory3D
        rlInfo: RightLeftInfo ( = short)
        """
        storedWireHits = Belle2.PyStoreObj('CDCWireHitVector')
        wireHits = storedWireHits.obj().get()

        CDCRecoHit3D = Belle2.TrackFindingCDC.CDCRecoHit3D
        wireHit = wireHits.at(bisect.bisect_left(wireHits, cdcHit))
        rightLeftWireHit = Belle2.TrackFindingCDC.CDCRLWireHit(wireHit, rlInfo)
        if rightLeftWireHit.getStereoType() != 0:
            recoHit3D = CDCRecoHit3D.reconstruct(rightLeftWireHit, trajectory3D.getTrajectory2D())
            return recoHit3D
        else:
            return None

    def get_plottable_line(self, recoHit3D):
        """
        Minim the task of the StereoQuadTree by showing the line of quadtree nodes
        a hit belongs to
        """
        z0 = [self.range_y_min, self.range_y_max]
        arr_l = np.array((np.array(recoHit3D.getRecoPos3D().z()) - z0) / recoHit3D.getArcLength2D())
        return arr_l, z0

    def plot_hit_line(self, recoHit3D, color):
        """
        Draw one recoHit3D
        """
        if recoHit3D:
            if recoHit3D.getStereoType() == 0:
                return

            arr_l, z0 = self.get_plottable_line(recoHit3D)
            plt.plot(arr_l, z0, marker="", ls="-", alpha=0.4, color=color)

    def event(self):
        """
        Draw the hit content according to the attributes

        Attributes
        ----------
        draw_mc_hits
        draw_mc_tracks
        draw_track_hits
        draw_last_track
        delete_bad_hits
        """
        #: by default, draw the QuadTree
        self.draw_quad_tree_content = True

        #: default lower x bound
        self.range_x_min = -2 - np.sqrt(3)
        #: default upper x bound
        self.range_x_max = 2 + np.sqrt(3)

        #: default lower y bound
        self.range_y_min = -100
        #: default upper y bound
        self.range_y_max = -self.range_y_min

        self.init_plotting()
        self.plot_quad_tree_content()

        map = attributemaps.listColors
        cdcHits = Belle2.PyStoreArray("CDCHits")

        items = Belle2.PyStoreObj("CDCTrackVector")
        wrapped_vector = items.obj()
        track_vector = wrapped_vector.get()

        mcHitLookUp = Belle2.TrackFindingCDC.CDCMCHitLookUp().getInstance()
        mcHitLookUp.fill()

        storedWireHits = Belle2.PyStoreObj('CDCWireHitVector')
        wireHits = storedWireHits.obj().get()

        if self.draw_mc_hits:
            mc_track_cands = Belle2.PyStoreArray("MCTrackCands")

            for track in mc_track_cands:
                mcTrackID = track.getMcTrackId()
                trajectory = self.create_trajectory_from_track(track)

                for cdcHitID in track.getHitIDs(Belle2.Const.CDC):
                    cdcHit = cdcHits[cdcHitID]

                    leftRecoHit3D = self.create_reco_hit3D(cdcHit, trajectory, -1)
                    rightRecoHit3D = self.create_reco_hit3D(cdcHit, trajectory, 1)

                    self.plot_hit_line(leftRecoHit3D, color=map[mcTrackID % len(map)])
                    self.plot_hit_line(rightRecoHit3D, color=map[mcTrackID % len(map)])

        if self.draw_mc_tracks:
            mc_track_cands = Belle2.PyStoreArray("MCTrackCands")

            for track in mc_track_cands:
                mcTrackID = track.getMcTrackId()
                trajectory = self.create_trajectory_from_track(track)
                z0 = trajectory.getTrajectorySZ().getZ0()

                for cdcHitID in track.getHitIDs(Belle2.Const.CDC):
                    cdcHit = cdcHits[cdcHitID]
                    recoHit3D = self.create_reco_hit3D(cdcHit, trajectory, mcHitLookUp.getRLInfo(cdcHit))

                    if recoHit3D:
                        arr_l = (recoHit3D.getRecoPos3D().z() - z0) / recoHit3D.getArcLength2D()
                        plt.plot(arr_l, z0, marker="o", color=map[mcTrackID % len(map)], ls="", alpha=0.2)

        if self.draw_track_hits:
            for id, track in enumerate(track_vector):
                for recoHit3D in list(track.items()):
                    self.plot_hit_line(recoHit3D, color=map[id % len(map)])

        if self.draw_last_track and len(track_vector) != 0:

            last_track = track_vector[-1]
            trajectory = last_track.getStartTrajectory3D().getTrajectory2D()

            for wireHit in wireHits:
                for rlInfo in (-1, 1):
                    recoHit3D = Belle2.TrackFindingCDC.CDCRecoHit3D.reconstruct(wireHit, rlInfo, trajectory)

                    if (self.delete_bad_hits and
                        (wireHit.getRLInfo() != mcHitLookUp.getRLInfo(wireHit.getWireHit().getHit()) or
                         not recoHit3D.isInCellZBounds())):
                        continue

                    if recoHit3D in list(last_track.items()):
                        color = map[len(track_vector) % len(map)]
                    else:
                        if wireHit.getRLInfo() == 1:
                            color = "black"
                        else:
                            color = "gray"
                    self.plot_hit_line(recoHit3D, color)

        plt.xlabel(r"$\tan \ \lambda$")
        plt.ylabel(r"$z_0$")
        self.save_and_show_file()


class QueueStereoQuadTreePlotter(StereoQuadTreePlotter):

    """
    A wrapper around the svg drawer in the tracking package that
    writes its output files as a list to the queue
    """

    def __init__(self, queue, label, *args, **kwargs):
        """ The same as the base class, except:

        Arguments
        ---------

        queue: The queue to write to
        label: The key name in the queue
        """
        #: The queue to handle
        self.queue = queue
        #: The label for writing to the queue
        self.label = label
        StereoQuadTreePlotter.__init__(self, *args, **kwargs)

        #: The list of created paths
        self.file_list = []

    def terminate(self):
        """ Overwrite the terminate to put the list to the queue"""
        StereoQuadTreePlotter.terminate(self)
        self.queue.put(self.label, self.file_list)

    def save_and_show_file(self):
        """ Overwrite the function to listen for every new filename """

        from datetime import datetime
        from matplotlib import pyplot as plt

        fileName = "/tmp/" + datetime.now().isoformat() + '.svg'
        plt.savefig(fileName)
        self.file_list.append(fileName)
