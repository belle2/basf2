#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine(
'struct EventData {\
    long exp;\
    long run;\
    unsigned long evt;\
    int vxd_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int truehit_index;\
    int cluster_index;\
    int pixel_type;\
    float truehit_u;\
    float truehit_v;\
    float truehit_EntryW;\
    float truehit_ExitW;\
    float truehit_LossMomentum;\
    float truehit_time;\
    float truehit_charge;\
    float theta_u;\
    float theta_v;\
    float cluster_u;\
    float cluster_v;\
    float cluster_uEtaPosition;\
    float cluster_vEtaPosition;\
    float cluster_uError;\
    float cluster_vError;\
    float cluster_rho;\
    float cluster_charge;\
    float cluster_seed;\
    float cluster_size;\
    float cluster_uSize;\
    float cluster_vSize;\
    float cluster_uPull;\
    float cluster_vPull;\
};')

from ROOT import EventData


class PXDValidationTTree(Module):

    """
    A simple module to check the reconstruction of PXDTrueHits.
    This module writes its output to a ROOT tree.
    Primary goal is supporting of validation plots
    """

    def __init__(self):
        """Initialize the module"""

        super(PXDValidationTTree, self).__init__()
        # # Output ROOT file.
        self.file = ROOT.TFile('PXDValidationTTreeOutput.root', 'recreate')
        # # TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of PXD simulation')
        ## Histogram for constances
        self.h_consts = ROOT.TH1F('h_consts', 'Constances', 12, 0, 12)
        self.h_consts.GetXaxis().SetBinLabel(1,
             "Sensor thicknes [cm] (0.0075)")
        self.h_consts.GetXaxis().SetBinLabel(2,
             "Pitch in u - all sensors [cm] (0.005)")
        self.h_consts.GetXaxis().SetBinLabel(3,
             "Pitch in v, layer 1 small pitch [cm] (0.0055)")
        self.h_consts.GetXaxis().SetBinLabel(4,
             "Pitch in v, layer 1 large pitch [cm] (0.006)")
        self.h_consts.GetXaxis().SetBinLabel(5,
             "Pitch in v, layer 2 small pitch [cm] (0.007)")
        self.h_consts.GetXaxis().SetBinLabel(6,
             "Pitch in v, layer 2 large pitch [cm] (0.0085)")
        self.h_consts.GetXaxis().SetBinLabel(7,
             "Number of cells in u, all layers (250)")
        self.h_consts.GetXaxis().SetBinLabel(8,
             "Number of cells in v, layer 1 small pitch (256)")
        self.h_consts.GetXaxis().SetBinLabel(9,
             "Number of cells in v, layer 1 large pitch (512)")
        self.h_consts.GetXaxis().SetBinLabel(10,
             "Number of cells in v, layer 2 small pitch (256)")
        self.h_consts.GetXaxis().SetBinLabel(11,
            "Number of cells in v, layer 2 large pitch (512)")
        self.h_consts.GetXaxis().SetBinLabel(12,
            "Noise of digits of sensors [electrons] (200)")
        # # Instance of EventData class
        self.data = EventData()
        # Declare tree branches
        for key in EventData.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key),
                                 key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find clusters with a truehit and save needed information."""

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        pxd_clusters = Belle2.PyStoreArray('PXDClusters')
        WrtS1Pitch = 0
        WrtS2Pitch = 0
        VCellsSplit = 256
        DigitNoise = 200
#        belle2xmlFile = open('Belle2.xml', 'r')
#        belle2xml = belle2xmlFile.read()
#        belle2xmlFile.close()
        for cluster in pxd_clusters:
            cluster_truehits = cluster.getRelationsTo("PXDTrueHits")

            # Here we ask only for clusters with exactly one TrueHit.
            if len(cluster_truehits) != 1:
                continue

            for truehit in cluster_truehits:
                # Now let's store some data
                # Event identification
                self.data.exp = Belle2.PyStoreObj(\
                    'EventMetaData').obj().getExperiment()
                self.data.run = Belle2.PyStoreObj(\
                    'EventMetaData').obj().getRun()
                self.data.evt = Belle2.PyStoreObj(\
                    'EventMetaData').obj().getEvent()
                # Sesnor identification
                vxd_id = truehit.getSensorID()
                self.data.vxd_id = vxd_id.getID()
                self.data.layer = vxd_id.getLayerNumber()
                self.data.ladder = vxd_id.getLadderNumber()
                self.data.sensor = vxd_id.getSensorNumber()
                sensor_info = Belle2.VXD.GeoCache.get(vxd_id)
                UPitch = sensor_info.getUPitch()
                VPitch = sensor_info.getVPitch(truehit.getV())
                if vxd_id.getLayerNumber() == 1:
                    if vxd_id.getSensorNumber() == 1:
                        if WrtS1Pitch == 0:
                            self.h_consts.SetBinContent(1,
                                sensor_info.getThickness())
                            self.h_consts.SetBinContent(2,
                                sensor_info.getUPitch())
                            self.h_consts.SetBinContent(3,
                                sensor_info.getVPitch(-2.0))
                            self.h_consts.SetBinContent(4,
                                sensor_info.getVPitch(+2.0))
                            self.h_consts.SetBinContent(7,
                                sensor_info.getUCells())
                            self.h_consts.SetBinContent(8,
                                VCellsSplit)
                            self.h_consts.SetBinContent(9,
                                sensor_info.getVCells() - VCellsSplit)
                            self.h_consts.SetBinContent(12,
                                DigitNoise)
                            WrtS1Pitch = 1
                if vxd_id.getLayerNumber() == 2:
                    if vxd_id.getSensorNumber() == 1:
                        if WrtS2Pitch == 0:
                            self.h_consts.SetBinContent(5,
                                sensor_info.getVPitch(-2.0))
                            self.h_consts.SetBinContent(6,
                                sensor_info.getVPitch(+2.0))
                            self.h_consts.SetBinContent(10,
                                VCellsSplit)
                            self.h_consts.SetBinContent(11,
                                sensor_info.getVCells() - VCellsSplit)
                            WrtS2Pitch = 1
                self.data.pixel_type = (vxd_id.getLayerNumber() - 1) * 2
                if vxd_id.getLayerNumber() == 1:
                    if VPitch > 0.0058:
                        self.data.pixel_type += 1
                if vxd_id.getLayerNumber() == 2:
                    if VPitch > 0.0080:
                        self.data.pixel_type += 1
                # Hit identification
                self.data.truehit_index = truehit.getArrayIndex()
                self.data.cluster_index = cluster.getArrayIndex()
                # Get sensor geometry information
                sensor_info = Belle2.VXD.GeoCache.get(vxd_id)
                thickness = sensor_info.getThickness()
                # TrueHit information
                self.data.truehit_u = truehit.getU()
                self.data.truehit_v = truehit.getV()
                self.data.truehit_EntryW = truehit.getEntryW()
                self.data.truehit_ExitW = truehit.getExitW()
                self.data.truehit_LossMomentum = \
                    truehit.getEntryMomentum().Mag() - \
                        truehit.getExitMomentum().Mag()
                self.data.truehit_time = truehit.getGlobalTime()
                self.data.truehit_charge = truehit.getEnergyDep()
                self.data.theta_u = \
                    math.atan2(truehit.getExitU() - truehit.getEntryU(), \
                    thickness)
                self.data.theta_v = \
                    math.atan2(truehit.getExitV() - truehit.getEntryV(), \
                    thickness)
                # Cluster information
                self.data.cluster_u = cluster.getU()
                self.data.cluster_v = cluster.getV()
                self.data.cluster_uError = cluster.getUSigma()
                self.data.cluster_vError = cluster.getVSigma()
                self.data.cluster_rho = cluster.getRho()
                self.data.cluster_charge = cluster.getCharge()
                self.data.cluster_seed = cluster.getSeedCharge()
                self.data.cluster_size = cluster.getSize()
                self.data.cluster_uSize = cluster.getUSize()
                self.data.cluster_vSize = cluster.getVSize()
                self.data.cluster_uEtaPosition = 0.5 + (cluster.getU()\
                            - sensor_info.getUCellPosition(\
                            sensor_info.getUCellID(cluster.getU())))\
                            / UPitch
                self.data.cluster_vEtaPosition = 0.5 + (cluster.getV()\
                            - sensor_info.getVCellPosition(\
                            sensor_info.getVCellID(cluster.getV())))\
                            / VPitch
                self.data.cluster_uPull = (cluster.getU() - truehit.getU())\
                            / cluster.getUSigma()
                self.data.cluster_vPull = (cluster.getV() - truehit.getV())\
                            / cluster.getVSigma()
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.h_consts.GetListOfFunctions().Add(ROOT.TNamed("Description", \
            "Transfer of some basic constances to root for later procesing."))
        self.h_consts.GetListOfFunctions().Add(ROOT.TNamed("Check", \
            "Validation: Check differences to reference plot, \
            must be no change."))

        self.file.cd()
        self.file.Write()
    #    self.h_consts.Write()
        self.file.Flush()
        self.file.Close()
