#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


# This steering file steers fills cluster position residuals and pulls with truehits to
# test the cluster position estimator payloads from CAF.
#
# Execute as: basf2 test_cluster_position_estimator.py

import math
import basf2 as b2
import ROOT
from ROOT import Belle2

# set some random seed
b2.set_random_seed(10346)


class PXDPositionEstimation(b2.Module):
    """
    Histogram the difference position residuals and pulls between clusters and truehits.
    """

    def initialize(self):
        """
        Create histograms for pulls and residuals
        """

        #: Counter for all clusters
        self.nclusters = 0
        #: Counter for cluster where shape likelyhood was found in payload
        self.nfound_shapes = 0
        #: Counter for clusters where position correction was found in payload
        self.nfound_offset = 0

        # Let's create a root file to store all profiles
        #: Output file to store all plots
        self.rfile = ROOT.TFile("PXDPositionEstimation.root", "RECREATE")
        self.rfile.cd()

        #: Histograms for true particle momenta
        self.hist_map_momentum = {}
        #: Histograms for true particle angle thetaU
        self.hist_map_theta_u = {}
        #: Histograms for true particle angle thetaV
        self.hist_map_theta_v = {}
        #: Histograms for cluster charge related to particle
        self.hist_map_clustercharge = {}

        # Loop over clusterkinds (=4 means 'all' kinds)
        for kind in range(5):
            self.hist_map_momentum[kind] = ROOT.TH1F(f"hist_momentum_kind_{kind:d}",
                                                     f'Particle momentum kind={kind:d}', 5000, 0.0, 10.0)
            self.hist_map_theta_u[kind] = ROOT.TH1F(f"hist_theta_u_kind_{kind:d}",
                                                    f'Particle angle thetaU kind={kind:d}', 180, -90, +90)
            self.hist_map_theta_v[kind] = ROOT.TH1F(f"hist_theta_v_kind_{kind:d}",
                                                    f'Particle angle thetaV kind={kind:d}', 180, -90, +90)
            self.hist_map_clustercharge[kind] = ROOT.TH1F(f"hist_clustercharge_kind_{kind:d}",
                                                          f'Cluster charge kind={kind:d}', 255, 0.0, 255.0)

        #: Histograms for u residuals
        self.hist_map_residual_u = {}
        #: Histograms for v residuals
        self.hist_map_residual_v = {}
        #: Histograms for v residuals for smaller thetaV range
        self.hist_map_residual_v_special = {}
        #: Histograms for u residual pulls
        self.hist_map_residual_pull_u = {}
        #: Histograms for v residual pulls
        self.hist_map_residual_pull_v = {}

        for kind in range(4):
            for mode in range(3):
                self.hist_map_residual_u[(kind, mode)] = ROOT.TH1F(f'hist_map_residual_u_kind_{kind:d}_mode_{mode:d}',
                                                                   f'PXD residual U kind={kind:d} mode={mode:d}',
                                                                   400, -0.007, +0.007)
                self.hist_map_residual_v[(kind, mode)] = ROOT.TH1F(f'hist_map_residual_v_kind_{kind:d}_mode_{mode:d}',
                                                                   f'PXD residual V kind={kind:d} mode={mode:d}',
                                                                   400, -0.007, +0.007)
                self.hist_map_residual_pull_u[(kind, mode)] = ROOT.TH1F(f'hist_map_residual_pull_u_kind_{kind:d}_mode_{mode:d}',
                                                                        f'PXD residual pull U kind={kind:d} mode={mode:d}',
                                                                        200, -10, +10)
                self.hist_map_residual_pull_v[(kind, mode)] = ROOT.TH1F(f'hist_map_residual_pull_v_kind_{kind:d}_mode_{mode:d}',
                                                                        f'PXD residual pull V kind={kind:d} mode={mode:d}',
                                                                        200, -10, +10)

                #: ThetaV angle ranges for v residuals
                self.binlimits = {}
                self.binlimits[0] = (-90, -30)
                self.binlimits[1] = (-30, +30)
                self.binlimits[2] = (+30, +90)

                for bin in range(3):
                    name = f'hist_map_residual_v_kind_{kind:d}_mode_{mode:d}_special_{bin:d}'
                    title = f'PXD residual V kind={kind:d} mode={mode:d} {self.binlimits[bin][0]:.0f}' + \
                        f'<thetaV<{self.binlimits[bin][1]:.0f}'
                    self.hist_map_residual_v_special[(kind, mode, bin)] = ROOT.TH1F(name, title, 400, -0.007, +0.007)

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
                    tu = mom[0] / mom[2]
                    tv = mom[1] / mom[2]
                    thetaU = math.atan(tu) * 180 / math.pi
                    thetaV = math.atan(tv) * 180 / math.pi

                    # Only look at primary particles -> check if the following is needed
                    # for mcp in truehit.getRelationsFrom("MCParticles"):
                    #    if not mcp.hasStatus(1):
                    #        reject = True

                    # Get instance of position estimator
                    PositionEstimator = Belle2.PXD.PXDClusterPositionEstimator.getInstance()
                    clusterkind = cls.getKind()

                    # Clusterkinds 0,1,2,3 refer to all cases which can currently
                    # be corrected. Cases where a cluster pixel touches a sensor
                    # edge or contains pixel with varying vPitch are excluded here.
                    if clusterkind <= 3 and mom.Mag() > 0.02:

                        self.nclusters += 1

                        # Fill momentum and angles for clusterkind
                        self.hist_map_momentum[clusterkind].Fill(mom.Mag())
                        self.hist_map_theta_u[clusterkind].Fill(thetaU)
                        self.hist_map_theta_v[clusterkind].Fill(thetaV)
                        self.hist_map_clustercharge[clusterkind].Fill(cls.getCharge())

                        # Fill clusterkind=4 for all PXD sensors
                        self.hist_map_momentum[4].Fill(mom.Mag())
                        self.hist_map_theta_u[4].Fill(thetaU)
                        self.hist_map_theta_v[4].Fill(thetaV)
                        self.hist_map_clustercharge[4].Fill(cls.getCharge())

                        # Fill the histograms (mode=2)
                        mode = 2
                        pull_u = (truehit.getU() - cls.getU()) / cls.getUSigma()
                        pull_v = (truehit.getV() - cls.getV()) / cls.getVSigma()

                        self.hist_map_residual_u[(clusterkind, mode)].Fill(truehit.getU() - cls.getU())
                        self.hist_map_residual_v[(clusterkind, mode)].Fill(truehit.getV() - cls.getV())
                        self.hist_map_residual_pull_u[(clusterkind, mode)].Fill(pull_u)
                        self.hist_map_residual_pull_v[(clusterkind, mode)].Fill(pull_v)

                        if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                            self.hist_map_residual_v_special[(clusterkind, mode, 0)].Fill(truehit.getV() - cls.getV())
                        elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                            self.hist_map_residual_v_special[(clusterkind, mode, 1)].Fill(truehit.getV() - cls.getV())
                        else:
                            self.hist_map_residual_v_special[(clusterkind, mode, 2)].Fill(truehit.getV() - cls.getV())

                        shape_likelyhood = PositionEstimator.getShapeLikelyhood(cls, tu, tv)
                        if shape_likelyhood > 0:
                            self.nfound_shapes += 1

                        offset = PositionEstimator.getClusterOffset(cls, tu, tv)
                        if offset:
                            # Now, we can safely querry the correction
                            self.nfound_offset += 1

                            # We need to explicitely add a shift to the offsets
                            # This is not needed when working with PXDRecoHits
                            shiftU = sensor_info.getUCellPosition(cls.getUStart())
                            shiftV = sensor_info.getVCellPosition(cls.getVStart())

                            # Fill the histograms (mode=0)
                            mode = 0
                            pull_u = (truehit.getU() - shiftU - offset.getU()) / (math.sqrt(offset.getUSigma2()))
                            pull_v = (truehit.getV() - shiftV - offset.getV()) / (math.sqrt(offset.getVSigma2()))

                            self.hist_map_residual_u[(clusterkind, mode)].Fill(truehit.getU() - shiftU - offset.getU())
                            self.hist_map_residual_v[(clusterkind, mode)].Fill(truehit.getV() - shiftV - offset.getV())
                            self.hist_map_residual_pull_u[(clusterkind, mode)].Fill(pull_u)
                            self.hist_map_residual_pull_v[(clusterkind, mode)].Fill(pull_v)

                            if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                                self.hist_map_residual_v_special[(clusterkind, mode, 0)].Fill(
                                    truehit.getV() - shiftV - offset.getV())
                            elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                                self.hist_map_residual_v_special[(clusterkind, mode, 1)].Fill(
                                    truehit.getV() - shiftV - offset.getV())
                            else:
                                self.hist_map_residual_v_special[(clusterkind, mode, 2)].Fill(
                                    truehit.getV() - shiftV - offset.getV())

                            # Fill the histograms (mode=1)
                            mode = 1
                            self.hist_map_residual_u[(clusterkind, mode)].Fill(truehit.getU() - shiftU - offset.getU())
                            self.hist_map_residual_v[(clusterkind, mode)].Fill(truehit.getV() - shiftV - offset.getV())
                            self.hist_map_residual_pull_u[(clusterkind, mode)].Fill(pull_u)
                            self.hist_map_residual_pull_v[(clusterkind, mode)].Fill(pull_v)

                            if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                                self.hist_map_residual_v_special[(clusterkind, mode, 0)].Fill(
                                    truehit.getV() - shiftV - offset.getV())
                            elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                                self.hist_map_residual_v_special[(clusterkind, mode, 1)].Fill(
                                    truehit.getV() - shiftV - offset.getV())
                            else:
                                self.hist_map_residual_v_special[(clusterkind, mode, 2)].Fill(
                                    truehit.getV() - shiftV - offset.getV())

                        else:

                            # Fill the histograms (mode=1)
                            mode = 1
                            pull_u = (truehit.getU() - cls.getU()) / cls.getUSigma()
                            pull_v = (truehit.getV() - cls.getV()) / cls.getVSigma()

                            self.hist_map_residual_u[(clusterkind, mode)].Fill(truehit.getU() - cls.getU())
                            self.hist_map_residual_v[(clusterkind, mode)].Fill(truehit.getV() - cls.getV())
                            self.hist_map_residual_pull_u[(clusterkind, mode)].Fill(pull_u)
                            self.hist_map_residual_pull_v[(clusterkind, mode)].Fill(pull_v)

                            if thetaV >= self.binlimits[0][0] and thetaV < self.binlimits[0][1]:
                                self.hist_map_residual_v_special[(clusterkind, mode, 0)].Fill(truehit.getV() - cls.getV())
                            elif thetaV >= self.binlimits[1][0] and thetaV < self.binlimits[1][1]:
                                self.hist_map_residual_v_special[(clusterkind, mode, 1)].Fill(truehit.getV() - cls.getV())
                            else:
                                self.hist_map_residual_v_special[(clusterkind, mode, 2)].Fill(truehit.getV() - cls.getV())

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

        for kind in range(4):
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

        hcoverage = ROOT.TH1F("hist_coverage", 'Coverage of corrections', 2, 1, 2)
        hcoverage.SetBinContent(1, 100.0 * float(self.nfound_offset / self.nclusters))
        hcoverage.SetBinContent(2, 100.0 * float(self.nfound_shapes / self.nclusters))
        hcoverage.SetLineWidth(2)
        hcoverage.SetYTitle('coverage / %')
        hcoverage.SetTitle('Coverage of cluster shape corrections')

        print(f"Coverage of cluster shape corrections is {100.0 * float(self.nfound_offset / self.nclusters):.2f}% ")
        print(f"Coverage of cluster shape likelyhoods is {100.0 * float(self.nfound_shapes / self.nclusters):.2f}% ")

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
    args = parser.parse_args()

    # Find background overlay files
    bkgfiles = glob.glob(args.bglocation + '/*.root')
    if len(bkgfiles) == 0:
        print('No BG overlay files found')
        sys.exit()

    # Now let's create a path to simulate our events.
    main = b2.create_path()
    main.add_module("EventInfoSetter", evtNumList=[10000])
    main.add_module("Gearbox")
    # We only need the pxd for this
    main.add_module("Geometry", components=['MagneticField', 'BeamPipe', 'PXD'], useDB=False)

    # Generate BBbar events
    main.add_module("EvtGenInput")

    # Background overlay input
    if bkgfiles:
        if args.bkgOverlay:
            bkginput = b2.register_module('BGOverlayInput')
            bkginput.param('inputFileNames', bkgfiles)
            main.add_module(bkginput)

    main.add_module("FullSim")
    main.add_module("PXDDigitizer")

    # Background overlay executor - after digitizer
    if bkgfiles:
        if args.bkgOverlay:
            main.add_module('BGOverlayExecutor', PXDDigitsName='')
            main.add_module("PXDDigitSorter", digits='')

    main.add_module("ActivatePXDClusterPositionEstimator")
    main.add_module("PXDClusterizer")

    positionestimation = PXDPositionEstimation()
    main.add_module(positionestimation)
    main.add_module("Progress")

    b2.process(main)
    print(b2.statistics)
