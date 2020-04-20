#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2, TFile, TTree, TH1F, TCanvas, TH2F, TGraph, TMultiGraph, TH2D, TLegend, TROOT, gROOT, TF1, TMath
import os
import numpy
import math
from array import array
import basf2

import simulation

import matplotlib.pyplot as plt
import numpy as np

import sys

inputFile = sys.argv[1]
outputFile = sys.argv[2]
time0 = "EventT0"
svd_Clusters = "SVDClusters"
svd_ClustersFromTracks = "SVDClustersFromTracks"
svd_EventInfo = "SVDEventInfo"
svd_EventMD = "EventMetaData"

set_random_seed(11)

gROOT.SetBatch(True)


class svdCoGCalibrationAnalysisTool(basf2.Module):

    def initialize(self):
        self.outputFileName = outputFile
        self.layer = np.zeros(1, dtype=int)
        self.ladder = np.zeros(1, dtype=int)
        self.sensor = np.zeros(1, dtype=int)
        self.side = np.zeros(1, dtype=int)
        self.TB = np.zeros(1, dtype=int)
        self.evtTime = np.zeros(1, dtype=float)
        self.evtTimeSync = np.zeros(1, dtype=float)
        self.clsTime = np.zeros(1, dtype=float)
        self.clsSize = np.zeros(1, dtype=float)
        self.clsCharge = np.zeros(1, dtype=float)
        self.clsSNR = np.zeros(1, dtype=float)
        self.run = np.zeros(1, dtype=int)
        self.exp = np.zeros(1, dtype=int)

        self.layerTracks = np.zeros(1, dtype=int)
        self.ladderTracks = np.zeros(1, dtype=int)
        self.sensorTracks = np.zeros(1, dtype=int)
        self.sideTracks = np.zeros(1, dtype=int)
        self.evtTimeTracks = np.zeros(1, dtype=float)
        self.evtTimeSyncTracks = np.zeros(1, dtype=float)
        self.clsTimeTracks = np.zeros(1, dtype=float)
        self.clsSizeTracks = np.zeros(1, dtype=float)
        self.clsChargeTracks = np.zeros(1, dtype=float)
        self.clsSNRTracks = np.zeros(1, dtype=float)

        # self.tFile = TFile(self.outputFileName, 'recreate')
        # self.tFile.cd()

        self.tree = TTree('cls', 'tree')
        self.tree.Branch('layer', self.layer, 'layer/I')
        self.tree.Branch('ladder', self.ladder, 'ladder/I')
        self.tree.Branch('sensor', self.sensor, 'sensor/I')
        self.tree.Branch('side', self.side, 'side/I')
        self.tree.Branch('tb', self.TB, 'tb/I')
        self.tree.Branch('evtT0', self.evtTime, 'evtT0/D')
        self.tree.Branch('evtT0Sync', self.evtTimeSync, 'evtT0Sync/D')
        self.tree.Branch('clsTime', self.clsTime, 'clsTime/D')
        self.tree.Branch('clsSize', self.clsSize, 'clsSize/D')
        self.tree.Branch('clsCharge', self.clsCharge, 'clsCharge/D')
        self.tree.Branch('clsSNR', self.clsSNR, 'clsSNR/D')
        self.tree.Branch('run', self.run, 'run/I')
        self.tree.Branch('exp', self.exp, 'exp/I')

        self.treeTracks = TTree('clsTracks', 'treeTracks')
        self.treeTracks.Branch('layer', self.layerTracks, 'layer/I')
        self.treeTracks.Branch('ladder', self.ladderTracks, 'ladder/I')
        self.treeTracks.Branch('sensor', self.sensorTracks, 'sensor/I')
        self.treeTracks.Branch('side', self.sideTracks, 'side/I')
        self.treeTracks.Branch('tb', self.TB, 'tb/I')
        self.treeTracks.Branch('evtT0', self.evtTimeTracks, 'evtT0/D')
        self.treeTracks.Branch('evtT0Sync', self.evtTimeSyncTracks, 'evtT0Sync/D')
        self.treeTracks.Branch('clsTime', self.clsTimeTracks, 'clsTime/D')
        self.treeTracks.Branch('clsSize', self.clsSizeTracks, 'clsSize/D')
        self.treeTracks.Branch('clsCharge', self.clsChargeTracks, 'clsCharge/D')
        self.treeTracks.Branch('clsSNR', self.clsSNRTracks, 'clsSNR/D')
        self.treeTracks.Branch('run', self.run, 'run/I')
        self.treeTracks.Branch('exp', self.exp, 'exp/I')

    def event(self):
        svd_cls = Belle2.PyStoreArray(svd_Clusters)
        svd_clsFromTrks = Belle2.PyStoreArray(svd_ClustersFromTracks)
        et0 = Belle2.PyStoreObj(time0)
        svd_evtInfo = Belle2.PyStoreObj(svd_EventInfo)
        clsTB = svd_evtInfo.getModeByte().getTriggerBin()
        self.TB[0] = ord(clsTB)
        svd_evtMD = Belle2.PyStoreObj(svd_EventMD)
        self.run[0] = svd_evtMD.getRun()
        self.exp[0] = svd_evtMD.getExperiment()

        for svdCluster in svd_cls:
            self.clsTime[0] = svdCluster.getClsTime()
            self.clsSize[0] = svdCluster.getSize()
            self.clsCharge[0] = svdCluster.getCharge()
            self.clsSNR[0] = svdCluster.getSNR()
            self.layer[0] = svdCluster.getSensorID().getLayerNumber()
            self.ladder[0] = svdCluster.getSensorID().getLadderNumber()
            self.sensor[0] = svdCluster.getSensorID().getSensorNumber()
            self.side[0] = svdCluster.isUCluster()
            hasT0 = et0.hasEventT0()
            if hasT0:
                self.evtTime[0] = et0.getEventT0()
                self.evtTimeSync[0] = et0.getEventT0() - 7.8625 * (3 - self.TB)
            else:
                self.evtTime[0] = -999
                self.evtTimeSync[0] = -999
            self.tree.Fill()

        for svdClusterTracks in svd_clsFromTrks:
            self.clsTimeTracks[0] = svdClusterTracks.getClsTime()
            self.clsSizeTracks[0] = svdClusterTracks.getSize()
            self.clsChargeTracks[0] = svdClusterTracks.getCharge()
            self.clsSNRTracks[0] = svdClusterTracks.getSNR()
            self.layerTracks[0] = svdClusterTracks.getSensorID().getLayerNumber()
            self.ladderTracks[0] = svdClusterTracks.getSensorID().getLadderNumber()
            self.sensorTracks[0] = svdClusterTracks.getSensorID().getSensorNumber()
            self.sideTracks[0] = svdClusterTracks.isUCluster()
            hasT0 = et0.hasEventT0()
            if hasT0:
                self.evtTimeTracks[0] = et0.getEventT0()
                self.evtTimeSyncTracks[0] = et0.getEventT0() - 7.8625 * (3 - self.TB)
            else:
                self.evtTimeTracks[0] = -999
                self.evtTimeSyncTracks[0] = -999
            self.treeTracks.Fill()

    def terminate(self):
        self.tFile = TFile(self.outputFileName, 'recreate')
        self.tFile.cd()
        self.tree.Write()
        self.treeTracks.Write()
        self.tFile.Close()
        del self.tFile

# conditions.override_globaltags()
# conditions.globaltags = ["online"]

main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileName', inputFile)
main.add_module(rootinput)

# main.add_module("Gearbox")
# main.add_module("Geometry", useDB = True)

main.add_module(svdCoGCalibrationAnalysisTool())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
