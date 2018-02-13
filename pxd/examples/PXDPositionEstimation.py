#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>benni</contact>
  <description>
    Histogram the difference between the estimated cluster position and the
    position of the related truehit for all layers of the PXD.
  </description>
</header>
"""

import math
from basf2 import *
import ROOT
from ROOT import Belle2

import utility
import copy

# set_log_level(LogLevel.ERROR)
# set some random seed
set_random_seed(10346)


class PXDPositionEstimation(Module):
    """
    Histogram the difference between the estimated cluster position and the position of the related truehit.
    """

    def __init__(self):
        """
        Create a member to access cluster shape position estimator
        """
        super().__init__()  # don't forget to call parent constructor
        # TODO: fully replace this by PXDPositionEstimator singleton class
        self.position_estimator = Belle2.PyDBObj('PXDClusterPositionEstimatorPar')
        self.shape_indexer = Belle2.PyDBObj('PXDClusterShapeIndexPar')

    def initialize(self):
        """
        Create histograms with pulls and residuals
        """

        # Counters to measure coverage of corrections
        self.nclusters = 0
        self.nfound_shapes = 0
        self.nfound_offset = 0

        # Let's create a root file to store all profiles
        #: Output file to store all plots
        self.rfile = ROOT.TFile("PXDPositionEstimation.root", "RECREATE")
        self.rfile.cd()

        # Create histograms for momenta and incidence angles
        self.hist_map_momentum = {}
        self.hist_map_theta_u = {}
        self.hist_map_theta_v = {}
        self.hist_map_clustercharge = {}

        for kind in range(5):
            self.hist_map_momentum[kind] = ROOT.TH1F("hist_momentum_kind_{:d}".format(
                kind), 'Particle momentum kind={:d}'.format(kind), 5000, 0.0, 10.0)
            self.hist_map_theta_u[kind] = ROOT.TH1F("hist_theta_u_kind_{:d}".format(
                kind), 'Particle angle thetaU kind={:d}'.format(kind), 180, -90, +90)
            self.hist_map_theta_v[kind] = ROOT.TH1F("hist_theta_v_kind_{:d}".format(
                kind), 'Particle angle thetaV kind={:d}'.format(kind), 180, -90, +90)
            self.hist_map_clustercharge[kind] = ROOT.TH1F("hist_clustercharge_kind_{:d}".format(
                kind), 'Cluster charge kind={:d}'.format(kind), 255, 0.0, 255.0)

        # Create histogram for residuals
        self.hist_map_residual_u = {}
        self.hist_map_residual_v = {}
        self.hist_map_residual_v_special = {}
        self.hist_map_residual_pull_u = {}
        self.hist_map_residual_pull_v = {}

        for kind in range(5):
            for mode in range(3):
                self.hist_map_residual_u[(kind, mode)] = ROOT.TH1F('hist_map_residual_u_kind_{:d}_mode_{:d}'.format(
                    kind, mode), 'PXD residual U kind={:d} mode={:d}'.format(kind, mode), 400, -0.004, +0.004)
                self.hist_map_residual_v[(kind, mode)] = ROOT.TH1F('hist_map_residual_v_kind_{:d}_mode_{:d}'.format(
                    kind, mode), 'PXD residual V kind={:d} mode={:d}'.format(kind, mode), 400, -0.004, +0.004)
                self.hist_map_residual_pull_u[(kind, mode)] = ROOT.TH1F('hist_map_residual_pull_u_kind_{:d}_mode_{:d}'.format(
                    kind, mode), 'PXD residual pull U kind={:d} mode={:d}'.format(kind, mode), 200, -5, +5)
                self.hist_map_residual_pull_v[(kind, mode)] = ROOT.TH1F('hist_map_residual_pull_v_kind_{:d}_mode_{:d}'.format(
                    kind, mode), 'PXD residual pull V kind={:d} mode={:d}'.format(kind, mode), 200, -5, +5)

                self.binlimits = {}
                self.binlimits[0] = (-90, -30)
                self.binlimits[1] = (-30, +30)
                self.binlimits[2] = (+30, +90)

                for bin in range(3):
                    name = 'hist_map_residual_v_kind_{:d}_mode_{:d}_special_{:d}'.format(kind, mode, bin)
                    title = 'PXD residual V kind={:d} mode={:d} {:.0f}<thetaV<{:.0f}'.format(
                        kind, mode, self.binlimits[bin][0], self.binlimits[bin][1])
                    self.hist_map_residual_v_special[(kind, mode, bin)] = ROOT.TH1F(name, title, 400, -0.004, +0.004)

    def event(self):
        """Fill the residual and pull histograms"""

        # Get truehits
        truehits = Belle2.PyStoreArray("PXDTrueHits")

        for truehit in truehits:
            if isinstance(truehit, Belle2.PXDTrueHit):
                sensor_info = Belle2.VXD.GeoCache.get(truehit.getSensorID())
                clusters = truehit.getRelationsFrom("PXDClusters")

                # now check if we find a cluster
                for j, cls in enumerate(clusters):
                    # we ignore all clusters where less then 100 electrons come from
                    # our truehit
                    if clusters.weight(j) < 100:
                        continue

                    mom = truehit.getMomentum()
                    thetaV = math.atan(mom[1] / mom[2]) * 180 / math.pi
                    thetaU = math.atan(mom[0] / mom[2]) * 180 / math.pi
                    reject, pixelkind = utility.check_cluster(cls)

                    # Only look at primary particles
                    for mcp in truehit.getRelationsFrom("MCParticles"):
                        if not mcp.hasStatus(1):
                            reject = True

                    if not reject and mom.Mag() > 0.02:

                        self.nclusters += 1

                        eta = utility.computeEta(cls, thetaU, thetaV)
                        shape_name = utility.get_short_shape_name(cls, thetaU, thetaV)
                        shape_index = self.shape_indexer.getShapeIndex(shape_name)

                        shiftU = sensor_info.getUCellPosition(cls.getUStart())
                        shiftV = sensor_info.getVCellPosition(cls.getVStart())

                        # Fill momentum and angles for pixelkind
                        self.hist_map_momentum[pixelkind].Fill(mom.Mag())
                        self.hist_map_theta_u[pixelkind].Fill(thetaU)
                        self.hist_map_theta_v[pixelkind].Fill(thetaV)
                        self.hist_map_clustercharge[pixelkind].Fill(cls.getCharge())

                        # Fill pixelkind=4 for all PXD sensors
                        self.hist_map_momentum[4].Fill(mom.Mag())
                        self.hist_map_theta_u[4].Fill(thetaU)
                        self.hist_map_theta_v[4].Fill(thetaV)
                        self.hist_map_clustercharge[4].Fill(cls.getCharge())

                        # Fill the histograms (mode=2)
                        mode = 2
                        pull_u = (truehit.getU() - cls.getU()) / cls.getUSigma()
                        pull_v = (truehit.getV() - cls.getV()) / cls.getVSigma()

                        self.hist_map_residual_u[(pixelkind, mode)].Fill(truehit.getU() - cls.getU())
                        self.hist_map_residual_v[(pixelkind, mode)].Fill(truehit.getV() - cls.getV())
                        self.hist_map_residual_pull_u[(pixelkind, mode)].Fill(pull_u)
                        self.hist_map_residual_pull_v[(pixelkind, mode)].Fill(pull_v)

                        if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                            self.hist_map_residual_v_special[(pixelkind, mode, 0)].Fill(truehit.getV() - cls.getV())
                        elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                            self.hist_map_residual_v_special[(pixelkind, mode, 1)].Fill(truehit.getV() - cls.getV())
                        else:
                            self.hist_map_residual_v_special[(pixelkind, mode, 2)].Fill(truehit.getV() - cls.getV())

                        shape_likelyhood = self.position_estimator.getShapeLikelyhood(shape_index, thetaU, thetaV, pixelkind)
                        if shape_likelyhood > 0:
                            self.nfound_shapes += 1

                        if self.position_estimator.hasOffset(shape_index, eta, thetaU, thetaV, pixelkind):
                            # Now, we can safely querry the correction
                            self.nfound_offset += 1
                            offset = self.position_estimator.getOffset(shape_index, eta, thetaU, thetaV, pixelkind)

                            # Fill the histograms (mode=0)
                            mode = 0
                            pull_u = (truehit.getU() - shiftU - offset.getU()) / (math.sqrt(offset.getUSigma2()))
                            pull_v = (truehit.getV() - shiftV - offset.getV()) / (math.sqrt(offset.getVSigma2()))

                            # ########################################
                            # FIXME: this is for testing the PXDClusterPositionEstimator singleton
                            # correctedCluster = copy.deepcopy(cls)
                            tv = math.atan(mom[1] / mom[2])
                            tu = math.atan(mom[0] / mom[2])
                            a = Belle2.PXD.PXDClusterPositionEstimator.getInstance()
                            # cls = a.correctCluster(cls, tu, tv)
                            # print('BENNI position estimator initialized')
                            new_position_estimator = a.getPositionEstimatorParameters()
                            new_shape_indexer = a.getShapeIndexParameters()
                            new_shape_index = new_shape_indexer.getShapeIndex(shape_name)
                            # print('  BENNI: new_index={}    old_index={}'.format(new_shape_index, shape_index))
                            # new_likelyhood = a.getShapeLikelyhood(cls, tu, tv)
                            # print('  BENNI: new_likelyhood={:f}    old_likelyhood={:f}'.format(new_likelyhood, shape_likelyhood))
                            new_name = a.getShortName(cls, thetaU, thetaV)
                            # print('  BENNI: new_name={}   old_name={}'.format(new_name, shape_name))
                            # END TESTING ############################

                            self.hist_map_residual_u[(pixelkind, mode)].Fill(truehit.getU() - shiftU - offset.getU())
                            self.hist_map_residual_v[(pixelkind, mode)].Fill(truehit.getV() - shiftV - offset.getV())
                            self.hist_map_residual_pull_u[(pixelkind, mode)].Fill(pull_u)
                            self.hist_map_residual_pull_v[(pixelkind, mode)].Fill(pull_v)

                            if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                                self.hist_map_residual_v_special[(pixelkind, mode, 0)].Fill(truehit.getV() - shiftV - offset.getV())
                            elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                                self.hist_map_residual_v_special[(pixelkind, mode, 1)].Fill(truehit.getV() - shiftV - offset.getV())
                            else:
                                self.hist_map_residual_v_special[(pixelkind, mode, 2)].Fill(truehit.getV() - shiftV - offset.getV())

                            # Fill the histograms (mode=1)
                            mode = 1
                            self.hist_map_residual_u[(pixelkind, mode)].Fill(truehit.getU() - shiftU - offset.getU())
                            self.hist_map_residual_v[(pixelkind, mode)].Fill(truehit.getV() - shiftV - offset.getV())
                            self.hist_map_residual_pull_u[(pixelkind, mode)].Fill(pull_u)
                            self.hist_map_residual_pull_v[(pixelkind, mode)].Fill(pull_v)

                            if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                                self.hist_map_residual_v_special[(pixelkind, mode, 0)].Fill(truehit.getV() - shiftV - offset.getV())
                            elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                                self.hist_map_residual_v_special[(pixelkind, mode, 1)].Fill(truehit.getV() - shiftV - offset.getV())
                            else:
                                self.hist_map_residual_v_special[(pixelkind, mode, 2)].Fill(truehit.getV() - shiftV - offset.getV())

                        else:

                            # Fill the histograms (mode=1)
                            mode = 1
                            pull_u = (truehit.getU() - cls.getU()) / cls.getUSigma()
                            pull_v = (truehit.getV() - cls.getV()) / cls.getVSigma()

                            self.hist_map_residual_u[(pixelkind, mode)].Fill(truehit.getU() - cls.getU())
                            self.hist_map_residual_v[(pixelkind, mode)].Fill(truehit.getV() - cls.getV())
                            self.hist_map_residual_pull_u[(pixelkind, mode)].Fill(pull_u)
                            self.hist_map_residual_pull_v[(pixelkind, mode)].Fill(pull_v)

                            if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                                self.hist_map_residual_v_special[(pixelkind, mode, 0)].Fill(truehit.getV() - cls.getV())
                            elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                                self.hist_map_residual_v_special[(pixelkind, mode, 1)].Fill(truehit.getV() - cls.getV())
                            else:
                                self.hist_map_residual_v_special[(pixelkind, mode, 2)].Fill(truehit.getV() - cls.getV())

    def terminate(self):
        """
        Format and write all histograms and plot them
        """

        for kind in range(5):
            self.hist_map_momentum[kind].SetLineWidth(2)
            self.hist_map_momentum[kind].SetXTitle('momentum / GeV')
            self.hist_map_momentum[kind].SetYTitle('number of particles')

            self.hist_map_theta_u[kind].SetLineWidth(2)
            self.hist_map_theta_u[kind].SetXTitle('thetaU / degree')
            self.hist_map_theta_u[kind].SetYTitle('number of particles')

            self.hist_map_theta_v[kind].SetLineWidth(2)
            self.hist_map_theta_v[kind].SetXTitle('thetaV / degree')
            self.hist_map_theta_v[kind].SetYTitle('number of particles')

            self.hist_map_clustercharge[kind].SetLineWidth(2)
            self.hist_map_clustercharge[kind].SetXTitle('cluster charge / ADU')
            self.hist_map_clustercharge[kind].SetYTitle('number of particles')

        for kind in range(5):
            for mode in range(3):
                self.hist_map_residual_pull_u[(kind, mode)].SetLineWidth(2)
                self.hist_map_residual_pull_u[(kind, mode)].SetXTitle('pull u')
                self.hist_map_residual_pull_u[(kind, mode)].SetYTitle('number of particles')

                self.hist_map_residual_pull_v[(kind, mode)].SetLineWidth(2)
                self.hist_map_residual_pull_v[(kind, mode)].SetXTitle('pull v')
                self.hist_map_residual_pull_v[(kind, mode)].SetYTitle('number of particles')

                self.hist_map_residual_u[(kind, mode)].SetLineWidth(2)
                self.hist_map_residual_u[(kind, mode)].SetXTitle('residuals u / cm')
                self.hist_map_residual_u[(kind, mode)].SetYTitle('number of particles')

                self.hist_map_residual_v[(kind, mode)].SetLineWidth(2)
                self.hist_map_residual_v[(kind, mode)].SetXTitle('residuals v / cm')
                self.hist_map_residual_v[(kind, mode)].SetYTitle('number of particles')

                for bin in range(3):
                    self.hist_map_residual_v_special[(kind, mode, bin)].SetLineWidth(2)
                    self.hist_map_residual_v_special[(kind, mode, bin)].SetXTitle('residuals v / cm')
                    self.hist_map_residual_v_special[(kind, mode, bin)].SetYTitle('number of particles')

        self.hcoverage = ROOT.TH1F("hist_coverage", 'Coverage of corrections', 2, 1, 2)
        self.hcoverage.SetBinContent(1, 100.0 * float(self.nfound_offset / self.nclusters))
        self.hcoverage.SetBinContent(2, 100.0 * float(self.nfound_shapes / self.nclusters))
        self.hcoverage.SetLineWidth(2)
        self.hcoverage.SetYTitle('coverage / %')
        self.hcoverage.SetTitle('Coverage of cluster shape corrections')

        print("Coverage of cluster shape corrections is {:.2f}% ".format(100.0 * float(self.nfound_offset / self.nclusters)))
        print("Coverage of cluster shape likelyhoods is {:.2f}% ".format(100.0 * float(self.nfound_shapes / self.nclusters)))

        self.rfile.Write()
        self.rfile.Close()


if __name__ == "__main__":

    import argparse
    import glob
    import sys

    parser = argparse.ArgumentParser(description="Test cluster shape corrections on generic BBbar events")
    parser.add_argument(
        '--bglocation',
        dest='bglocation',
        default='/home/benjamin/prerelease-01-00-00b-validation/samples',
        type=str,
        help='Location of bg overlay files')
    parser.add_argument('--bkgOverlay', dest='bkgOverlay', action="store_true", help='Perform background overlay')
    parser.add_argument('--nevents', dest='nevents', default=5000, type=int, help='Number of events')
    args = parser.parse_args()

    # Find background overlay files
    bkgfiles = glob.glob(args.bglocation + '/*.root')
    if len(bkgfiles) == 0:
        print('No BG overlay files found')
        sys.exit()

    # Now let's create a path to simulate our events.
    main = create_path()
    main.add_module("EventInfoSetter", evtNumList=[args.nevents])
    main.add_module("Gearbox")
    # We only need the pxd for this
    main.add_module("Geometry", components=['MagneticField', 'BeamPipe', 'PXD'], useDB=False)
    # main.add_module("Geometry",useDB=False)

    # Generate BBbar events
    main.add_module("EvtGenInput")

    # Background overlay input
    if bkgfiles:
        if args.bkgOverlay:
            bkginput = register_module('BGOverlayInput')
            bkginput.param('inputFileNames', bkgfiles)
            main.add_module(bkginput)

    main.add_module("FullSim")
    main.add_module("PXDDigitizer")

    # Background overlay executor - after digitizer
    if bkgfiles:
        if args.bkgOverlay:
            main.add_module('BGOverlayExecutor', PXDDigitsName='')
            main.add_module("PXDDigitSorter", digits='')

    main.add_module("ActivatePXDCalibration")
    main.add_module("PXDClusterizer")

    positionestimation = PXDPositionEstimation()
    main.add_module(positionestimation)
    main.add_module("Progress")

    process(main)
    print(statistics)
