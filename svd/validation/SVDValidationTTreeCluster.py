#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <contact> SVD Software Group, svd-software@belle2.org </contact>
    <description>
    This module is used for the SVD validation.
    It gets information about clusters and truehits, saving in a ttree in a ROOT file.
    </description>
</header>
"""

import math

import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, addressof
from ROOT import TVector3

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataCluster {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    int cluster_truehits_number;\
    int matched;\
    float strip_pitch;\
    float cluster_theta;\
    float cluster_phi;\
    float cluster_position;\
    float cluster_positionSigma;\
    float cluster_clsTime;\
    float cluster_clsTimeSigma;\
    float cluster_charge;\
    float cluster_seedCharge;\
    float cluster_size;\
    float cluster_snr;\
    float cluster_interstripPosition;\
    float cluster_pull;\
    float cluster_residual;\
    float truehit_position;\
    float truehit_interstripPosition;\
    float truehit_deposEnergy;\
    float truehit_lossmomentum;\
    float truehit_time;\
    float eventt0_all;\
    float eventt0_top;\
    float eventt0_cdc;\
    float eventt0_svd;\
    float eventt0_ecl;\
};')


from ROOT import EventDataCluster  # noqa


class SVDValidationTTreeCluster(b2.Module):
    '''class to produce the ttree for cluster reconstruction validation'''

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeCluster, self).__init__()

        #: Output ROOT file
        self.file = ROOT.TFile('../SVDValidationTTreeCluster.root', 'recreate')
        #: TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')
        #: instance of EventData class
        self.data = EventDataCluster()

        # Declare tree branches
        for key in EventDataCluster.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, addressof(self.data, key), key + formstring)

    def event(self):
        """ Find clusters with a truehit and save needed information """
        clusters = Belle2.PyStoreArray('SVDClusters')
        eventt0 = Belle2.PyStoreObj('EventT0')

        # get EventT0: combined, TOP, CDC, ECL
        self.data.eventt0_all = -1
        self.data.eventt0_top = -1
        self.data.eventt0_cdc = -1
        self.data.eventt0_ecl = -1
        self.data.eventt0_svd = -1
        top = Belle2.Const.DetectorSet(Belle2.Const.TOP)
        cdc = Belle2.Const.DetectorSet(Belle2.Const.CDC)
        ecl = Belle2.Const.DetectorSet(Belle2.Const.ECL)
        svd = Belle2.Const.DetectorSet(Belle2.Const.SVD)
        if eventt0.hasEventT0():
            self.data.eventt0_all = eventt0.getEventT0()
        if eventt0.hasTemporaryEventT0(svd):
            tmp = eventt0.getTemporaryEventT0s(Belle2.Const.SVD)
            self.data.eventt0_svd = tmp.back().eventT0
        if eventt0.hasTemporaryEventT0(cdc):
            tmp = eventt0.getTemporaryEventT0s(Belle2.Const.CDC)
            self.data.eventt0_cdc = tmp.back().eventT0
        if eventt0.hasTemporaryEventT0(top):
            tmp = eventt0.getTemporaryEventT0s(Belle2.Const.TOP)
            self.data.eventt0_top = tmp.back().eventT0
        if eventt0.hasTemporaryEventT0(ecl):
            evtT0List_ECL = eventt0.getTemporaryEventT0s(Belle2.Const.ECL)
            # Select the event t0 value from the ECL as the one with the smallest chi squared value (defined as ".quality")
            smallest_ECL_t0_minChi2 = evtT0List_ECL[0].quality
            self.data.eventt0_ecl = evtT0List_ECL[0].eventT0
            for tmp in evtT0List_ECL:
                if tmp.quality < smallest_ECL_t0_minChi2:
                    smallest_ECL_t0_minChi2 = tmp.quality
                    self.data.eventt0_ecl = tmp.eventT0

        for cluster in clusters:

            cluster_truehits = cluster.getRelationsTo('SVDTrueHits')

            # Sensor identification
            sensorInfo = Belle2.VXD.GeoCache.get(cluster.getSensorID())
            sensorID = cluster.getSensorID()
            self.data.sensor_id = int(sensorID)
            sensorNum = sensorID.getSensorNumber()
            self.data.sensor = sensorNum
            layerNum = sensorID.getLayerNumber()
            self.data.layer = layerNum
            if (layerNum == 3):
                sensorType = 1
            else:
                if (sensorNum == 1):
                    sensorType = 0
                else:
                    sensorType = 1
            self.data.sensor_type = sensorType
            ladderNum = sensorID.getLadderNumber()
            self.data.ladder = ladderNum
            if cluster.isUCluster():
                strip_dir = 0
            else:
                strip_dir = 1

            self.data.matched = 1
            # We want only clusters with at least one associated TrueHit
            # but, to compute purity, we need to store also some information
            # for clusters not related to true hits
            if len(cluster_truehits) == 0:
                self.data.matched = 0
                # Fill tree
                self.file.cd()
                self.tree.Fill()
                continue

            # find the trueHit with highest energy deposit (the "best" match)
            energy = 0
            bestTrueHitIndex = 0
            for i, trueHit in enumerate(cluster_truehits):
                if trueHit.getEnergyDep() > energy:
                    energy = trueHit.getEnergyDep()
                    bestTrueHitIndex = i
            bestTrueHit = cluster_truehits[bestTrueHitIndex]

            # Cluster information
            self.data.cluster_clsTime = cluster.getClsTime()
            self.data.cluster_clsTimeSigma = cluster.getClsTimeSigma()
            self.data.cluster_charge = cluster.getCharge()
            self.data.cluster_seedCharge = cluster.getSeedCharge()
            self.data.cluster_size = cluster.getSize()
            self.data.cluster_snr = cluster.getSNR()
            cluster_position = cluster.getPosition()
            if cluster.isUCluster():
                cluster_position = cluster.getPosition(bestTrueHit.getV())
                # Interstrip position calculations
                strip_pitch = sensorInfo.getUPitch(bestTrueHit.getV())
            else:
                strip_pitch = sensorInfo.getVPitch(bestTrueHit.getU())
            self.data.strip_dir = strip_dir
            self.data.strip_pitch = strip_pitch
            self.data.cluster_interstripPosition = cluster_position % strip_pitch / strip_pitch
            # theta and phi definitions
            if cluster.isUCluster():
                uPos = cluster_position
                vPos = 0
            else:
                uPos = 0
                vPos = cluster_position
            localPosition = TVector3(uPos, vPos, 0)  # sensor center at (0, 0, 0)
            globalPosition = sensorInfo.pointToGlobal(localPosition, True)
            x = globalPosition.X()
            y = globalPosition.Y()
            z = globalPosition.Z()
            # see https://d2comp.kek.jp/record/242?ln=en for the Belle II
            # coordinate system and related variables
            rho = math.sqrt(x * x + y * y)
            r = math.sqrt(x * x + y * y + z * z)
            # get theta as arccosine(z/r)
            thetaRadians = math.acos(z / r)
            theta = (thetaRadians * 180) / math.pi
            # get phi as arccosine(x/rho)
            phiRadians = math.acos(x / rho)
            if y < 0:
                phi = 360 - (phiRadians * 180) / math.pi
            else:
                phi = (phiRadians * 180) / math.pi
            self.data.cluster_theta = theta
            self.data.cluster_phi = phi
            # Pull calculations
            clusterPos = cluster_position
            clusterPosSigma = cluster.getPositionSigma()
            if cluster.isUCluster():
                truehitPos = bestTrueHit.getU()
            else:
                truehitPos = bestTrueHit.getV()
            cluster_residual = clusterPos - truehitPos
            cluster_pull = cluster_residual / clusterPosSigma
            self.data.cluster_position = clusterPos
            self.data.cluster_positionSigma = clusterPosSigma
            self.data.cluster_residual = cluster_residual
            self.data.cluster_pull = cluster_pull
            # Truehit information
            self.data.truehit_position = truehitPos
            truehit_interstripPosition = truehitPos % strip_pitch / strip_pitch
            self.data.truehit_interstripPosition = truehit_interstripPosition
            self.data.truehit_deposEnergy = bestTrueHit.getEnergyDep()
            self.data.truehit_lossmomentum = bestTrueHit.getEntryMomentum().R() - bestTrueHit.getExitMomentum().R()
            self.data.truehit_time = bestTrueHit.getGlobalTime()

            # DEBUG options, commented out
            # print("\n new cluster with at least one true hit ("+str(len(cluster_truehits))+")")
            # print("best True hit has energy = "+str(bestTrueHit.getEnergyDep()*1e6)+" eV,\
            # global time = "+str(bestTrueHit.getGlobalTime()))
            # print("cluster isU = "+str(cluster.isUCluster())+", size = "+str(self.data.cluster_size)+"\
            # position = "+str(clusterPos))
            # print("cluster charge = "+str(self.data.cluster_charge)+", time = "+str(self.data.cluster_clsTime))

            # Fill tree
            self.file.cd()
            self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
