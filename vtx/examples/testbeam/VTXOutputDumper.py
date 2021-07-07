#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import TrackData
from ROOT import HitData
import sys
import math
import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2

from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine('struct HitData {\
    int cellU_hit;\
    int cellV_hit;\
    int cellU_fit;\
    int cellV_fit;\
    float cellUCenter_fit;\
    float cellVCenter_fit;\
    float u_hit;\
    float v_hit;\
    float u_fit;\
    float v_fit;\
    float u_fiterr;\
    float v_fiterr;\
    int size;\
    int sizeU;\
    int sizeV;\
    float seedCharge;\
    float clusterCharge;\
    float theta_u;\
    float theta_v;\
    int hasTrack;\
};'
                  )

gROOT.ProcessLine('struct TrackData {\
    int cellU_fit;\
    int cellV_fit;\
    float cellUCenter_fit;\
    float cellVCenter_fit;\
    float u_fit;\
    float v_fit;\
    int hasHit;\
};'
                  )


conversion_cm_to_mm = 10


class VTXOutputDumper(b2.Module):

    """
    A simple module to check the reconstruction of VTXClusters.
    This module writes its output to 2 ROOT tree.

    Note that all lenght units are converted from cm in mm
    """

    def __init__(self, name=None):
        """Initialize the module"""

        super(VTXOutputDumper, self).__init__()
        #: Output ROOT file.
        if name is not None:
            self.file = ROOT.TFile(name, 'recreate')
        else:
            self.file = ROOT.TFile("VTXOutputDumper.root", 'recreate')
        #: TTree for output data
        self.Hit = ROOT.TTree('Hit', 'Hit data of VTX simulation')
        self.Track = ROOT.TTree('Track', 'Track data of VTX simulation')
        #: Instance of HitData class
        self.dataHit = HitData()
        self.dataTrack = TrackData()
        # Declare tree branches
        for key in HitData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.dataHit.__getattribute__(key), int):
                    formstring = '/I'
                self.Hit.Branch(key, AddressOf(self.dataHit, key), key + formstring)

        for key in TrackData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.dataTrack.__getattribute__(key), int):
                    formstring = '/I'
                self.Track.Branch(key, AddressOf(self.dataTrack, key), key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find clusters with a truehit and print some stats."""

        # Get instance of position estimator
        PositionEstimator = Belle2.VTX.VTXClusterPositionEstimator.getInstance()

        # Fill the Hit Tree
        clusters = Belle2.PyStoreArray('VTXClusters')

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster in clusters:

            truehits = cluster.getRelationsTo('VTXTrueHits')

            # Here we ask only for clusters with exactly one TrueHit.
            # if len(truehits) != 1:
            #     continue

            for truehit in truehits:
                # Now let's store some data
                # Get sensor geometry information
                sensor_info = Belle2.VXD.GeoCache.get(cluster.getSensorID())
                sensor_info_truehit = Belle2.VXD.GeoCache.get(truehit.getSensorID())
                assert (sensor_info == sensor_info_truehit), B2ERROR("Different sensor info for TrueHit and Cluster")

                thickness = sensor_info.getThickness()
                mom = truehit.getMomentum()
                tu = mom[0] / mom[2]
                tv = mom[1] / mom[2]

                # TrueHit information
                pitchU = conversion_cm_to_mm * sensor_info.getUPitch(truehit.getV())
                pitchV = conversion_cm_to_mm * sensor_info.getVPitch(truehit.getV())
                # Print the pitch obtained from the sensor info
                # print('PitchU={:.4f}mm and PitchV={:.4f}mm'.format(pitchU,pitchV  ) )
                self.dataHit.u_fit = conversion_cm_to_mm * truehit.getU()
                self.dataHit.v_fit = conversion_cm_to_mm * truehit.getV()
                truehit_UcellID = sensor_info.getUCellID(truehit.getU())
                truehit_VcellID = sensor_info.getVCellID(truehit.getV())
                self.dataHit.cellU_fit = truehit_UcellID
                self.dataHit.cellV_fit = truehit_VcellID
                self.dataHit.cellUCenter_fit = conversion_cm_to_mm * sensor_info.getUCellPosition(truehit_UcellID)
                self.dataHit.cellVCenter_fit = conversion_cm_to_mm * sensor_info.getVCellPosition(truehit_VcellID)
                self.dataHit.theta_u = math.atan2(truehit.getExitU() - truehit.getEntryU(), thickness)
                self.dataHit.theta_v = math.atan2(truehit.getExitV() - truehit.getEntryV(), thickness)
                # Cluster information
                offset = PositionEstimator.getClusterOffset(cluster, tu, tv)
                if offset:
                    # print('Correction found')
                    shiftU = sensor_info.getUCellPosition(cluster.getUStart())
                    shiftV = sensor_info.getVCellPosition(cluster.getVStart())
                    self.dataHit.u_hit = conversion_cm_to_mm * (shiftU + offset.getU())
                    self.dataHit.v_hit = conversion_cm_to_mm * (shiftV + offset.getV())
                else:
                    # print('No correction found')
                    self.dataHit.u_hit = conversion_cm_to_mm * cluster.getU()
                    self.dataHit.v_hit = conversion_cm_to_mm * cluster.getV()
                self.dataHit.cellU_hit = sensor_info.getUCellID(cluster.getU())
                self.dataHit.cellV_hit = sensor_info.getVCellID(cluster.getV())
                self.dataHit.u_fiterr = 0
                self.dataHit.v_fiterr = 0
                self.dataHit.size = cluster.getSize()
                self.dataHit.sizeU = cluster.getUSize()
                self.dataHit.sizeV = cluster.getVSize()
                self.dataHit.clusterCharge = cluster.getCharge()
                self.dataHit.seedCharge = cluster.getSeedCharge()
                self.dataHit.hasTrack = 0
                self.file.cd()
                self.Hit.Fill()

        mcparticles = Belle2.PyStoreArray('MCParticles')

        for mcparticle in mcparticles:

            truehits = mcparticle.getRelationsTo('VTXTrueHits')

            for truehit in truehits:
                sensor_info = Belle2.VXD.GeoCache.get(truehit.getSensorID())

                thickness = sensor_info.getThickness()

                self.dataTrack.u_fit = conversion_cm_to_mm * truehit.getU()
                self.dataTrack.v_fit = conversion_cm_to_mm * truehit.getV()
                truehit_UcellID = sensor_info.getUCellID(truehit.getU())
                truehit_VcellID = sensor_info.getVCellID(truehit.getV())
                self.dataTrack.cellU_fit = truehit_UcellID
                self.dataTrack.cellV_fit = truehit_VcellID
                self.dataTrack.cellUCenter_fit = conversion_cm_to_mm * sensor_info.getUCellPosition(truehit_UcellID)
                self.dataTrack.cellVCenter_fit = conversion_cm_to_mm * sensor_info.getVCellPosition(truehit_VcellID)
                self.dataTrack.hasHit = 0
                self.file.cd()
                self.Track.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()
