#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2, TFile, TTree, gROOT
import numpy


import numpy as np

import sys

inputFile = sys.argv[1]
outputFile = sys.argv[2]
time0 = "EventT0"
svd_Clusters = "SVDClusters"
svd_ClustersFromTracks = "SVDClustersFromTracks"
svd_EventInfo = "SVDEventInfo"
svd_EventMD = "EventMetaData"

b2.set_random_seed(11)

gROOT.SetBatch(True)


class svdCoGCalibrationAnalysisTool(b2.Module):
    """
    Class used to check the goodness of the CoG time calibration
    """

    def initialize(self):
        """
        Initialize object (histograms, lists, ...) used by the class
        """
        #: name of the output file provided by the user
        self.outputFileName = outputFile
        #: layer array for the tree
        self.layer = np.zeros(1, dtype=int)
        #: ladder array for the tree
        self.ladder = np.zeros(1, dtype=int)
        #: sensor array for the tree
        self.sensor = np.zeros(1, dtype=int)
        #: side array for the tree
        self.side = np.zeros(1, dtype=int)
        #: TB array for the tree
        self.TB = np.zeros(1, dtype=int)
        #: T0 array for the tree
        self.evtTime = np.zeros(1, dtype=float)
        #: T0Sync array for the tree
        self.evtTimeSync = np.zeros(1, dtype=float)
        #: cluster time array for the tree
        self.clsTime = np.zeros(1, dtype=float)
        #: cluster size array for the tree
        self.clsSize = np.zeros(1, dtype=float)
        #: cluster charge array for the tree
        self.clsCharge = np.zeros(1, dtype=float)
        #: cluster SNR array for the tree
        self.clsSNR = np.zeros(1, dtype=float)
        #: run number array for the tree
        self.run = np.zeros(1, dtype=int)
        #: experiment number array for the tree
        self.exp = np.zeros(1, dtype=int)

        #: layer array for the tree for clusters associated to tracks
        self.layerTracks = np.zeros(1, dtype=int)
        #: ladder array for the tree for clusters associated to tracks
        self.ladderTracks = np.zeros(1, dtype=int)
        #: sensor array for the tree for clusters associated to tracks
        self.sensorTracks = np.zeros(1, dtype=int)
        #: side array for the tree for clusters associated to tracks
        self.sideTracks = np.zeros(1, dtype=int)
        #: T0 array for the tree for clusters associated to tracks
        self.evtTimeTracks = np.zeros(1, dtype=float)
        #: T0Sync array for the tree for clusters associated to tracks
        self.evtTimeSyncTracks = np.zeros(1, dtype=float)
        #: cluster time array for the tree for clusters associated to tracks
        self.clsTimeTracks = np.zeros(1, dtype=float)
        #: cluster size array for the tree for clusters associated to tracks
        self.clsSizeTracks = np.zeros(1, dtype=float)
        #: cluster charge array for the tree for clusters associated to tracks
        self.clsChargeTracks = np.zeros(1, dtype=float)
        #: cluster SNR array for the tree for clusters associated to tracks
        self.clsSNRTracks = np.zeros(1, dtype=float)

        # self.tFile = TFile(self.outputFileName, 'recreate')
        # self.tFile.cd()

        #: tree for clusters
        self.tree = TTree('cls', 'tree')
        #: tree for clusters, branch for layer number
        self.tree.Branch('layer', self.layer, 'layer/I')
        #: tree for clusters, branch for ladder number
        self.tree.Branch('ladder', self.ladder, 'ladder/I')
        #: tree for clusters, branch for sensor number
        self.tree.Branch('sensor', self.sensor, 'sensor/I')
        #: tree for clusters, branch for side
        self.tree.Branch('side', self.side, 'side/I')
        #: tree for clusters, branch for trigger bin
        self.tree.Branch('tb', self.TB, 'tb/I')
        #: tree for clusters, branch for T0
        self.tree.Branch('evtT0', self.evtTime, 'evtT0/D')
        #: tree for clusters, branch for T0Sync
        self.tree.Branch('evtT0Sync', self.evtTimeSync, 'evtT0Sync/D')
        #: tree for clusters, branch for cluster time
        self.tree.Branch('clsTime', self.clsTime, 'clsTime/D')
        #: tree for clusters, branch for cluster size
        self.tree.Branch('clsSize', self.clsSize, 'clsSize/D')
        #: tree for clusters, branch for cluster charge
        self.tree.Branch('clsCharge', self.clsCharge, 'clsCharge/D')
        #: tree for clusters, branch for cluster SNR
        self.tree.Branch('clsSNR', self.clsSNR, 'clsSNR/D')
        #: tree for clusters, branch for run number
        self.tree.Branch('run', self.run, 'run/I')
        #: tree for clusters, branch for exp number
        self.tree.Branch('exp', self.exp, 'exp/I')

        #: tree for clusters associated to tracks
        self.treeTracks = TTree('clsTracks', 'treeTracks')
        #: tree for clusters associated to tracks, branch for layer number
        self.treeTracks.Branch('layer', self.layerTracks, 'layer/I')
        #: tree for clusters associated to tracks, branch for ladder number
        self.treeTracks.Branch('ladder', self.ladderTracks, 'ladder/I')
        #: tree for clusters associated to tracks, branch for sensor number
        self.treeTracks.Branch('sensor', self.sensorTracks, 'sensor/I')
        #: tree for clusters associated to tracks, branch for side
        self.treeTracks.Branch('side', self.sideTracks, 'side/I')
        #: tree for clusters associated to tracks, branch for trigger bin
        self.treeTracks.Branch('tb', self.TB, 'tb/I')
        #: tree for clusters associated to tracks, branch for T0
        self.treeTracks.Branch('evtT0', self.evtTimeTracks, 'evtT0/D')
        #: tree for clusters associated to tracks, branch for T0Sync
        self.treeTracks.Branch('evtT0Sync', self.evtTimeSyncTracks, 'evtT0Sync/D')
        #: tree for clusters associated to tracks, branch for cluster time
        self.treeTracks.Branch('clsTime', self.clsTimeTracks, 'clsTime/D')
        #: tree for clusters associated to tracks, branch for cluster size
        self.treeTracks.Branch('clsSize', self.clsSizeTracks, 'clsSize/D')
        #: tree for clusters associated to tracks, branch for cluster charge
        self.treeTracks.Branch('clsCharge', self.clsChargeTracks, 'clsCharge/D')
        #: tree for clusters associated to tracks, branch for cluster SNR
        self.treeTracks.Branch('clsSNR', self.clsSNRTracks, 'clsSNR/D')
        #: tree for clusters associated to tracks, branch for run number
        self.treeTracks.Branch('run', self.run, 'run/I')
        #: tree for clusters associated to tracks, branch for exp number
        self.treeTracks.Branch('exp', self.exp, 'exp/I')

    def event(self):
        """
        Function that allows to cicle on the events
        """
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
            #: correction to pass to the FTSW reference system
            self.ftswShift = svd_evtInfo.getSVD2FTSWTimeShift(svdCluster.getFirstFrame())
            hasT0 = et0.hasEventT0()
            if hasT0:
                self.evtTime[0] = et0.getEventT0()
                self.evtTimeSync[0] = et0.getEventT0() - self.ftswShift
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
            #: correction to pass to the FTSW reference system
            self.ftswShiftTracks = svd_evtInfo.getSVD2FTSWTimeShift(svdCluster.getFirstFrame())
            hasT0 = et0.hasEventT0()
            if hasT0:
                self.evtTimeTracks[0] = et0.getEventT0()
                self.evtTimeSyncTracks[0] = et0.getEventT0() - self.ftswShiftTracks
            else:
                self.evtTimeTracks[0] = -999
                self.evtTimeSyncTracks[0] = -999
            self.treeTracks.Fill()

    def terminate(self):
        """
        Terminates te class and produces the output rootfile
        """
        #: creation of TFile
        self.tFile = TFile(self.outputFileName, 'recreate')
        self.tFile.cd()
        self.tree.Write()
        self.treeTracks.Write()
        self.tFile.Close()
        del self.tFile

# conditions.override_globaltags()
# conditions.globaltags = ["online"]


main = b2.create_path()

rootinput = b2.register_module('RootInput')
rootinput.param('inputFileName', inputFile)
main.add_module(rootinput)

# main.add_module("Gearbox")
# main.add_module("Geometry", useDB = True)

main.add_module(svdCoGCalibrationAnalysisTool())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
