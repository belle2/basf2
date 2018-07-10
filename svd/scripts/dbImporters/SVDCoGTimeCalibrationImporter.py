#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2, TFile, TTree, TH1F, TCanvas, TH2F, TGraph, TMultiGraph, TH2D, TLegend, TROOT, gROOT, TF1, TMath, gStyle
import os
import numpy
import math
from array import array
import basf2

import matplotlib.pyplot as plt
import simulation

hasCluster = True
hasRecoDigits = True

# inputFile = './cDST_run2520.root' #ARGHH, mancano SVDRecoDigits e SVDClusters
# inputFile = '/home/belle2/lgcorona/myHead_lgcorona1/workdir/svd/svd_phase2_scripts/Run2520
# /rootOutput/SVDRootOutput_data_exp3_Run2520_beam.0003.02520.root'
# inputFile = 'SVDRootOutput_data_exp3_Run2520_raw.physics.hlt_hadron.0003.02520_small.root'
inputFile = 'input.root'

svd_recoDigits = "SVDRecoDigits"
cdc_Time0 = "EventT0"
svd_Clusters = "SVDClusters"
svd_Tracks = "Tracks"
svd_RecoTracks = "RecoTracks"

set_random_seed(11)

gROOT.SetBatch(True)
gStyle.SetOptFit(11111111)


class SVDCoGTimeCalibrationImporterModule(basf2.Module):

    def sortDigits(self, unsortedPyStoreArray):

        # convert to a python-list to be abple to sort
        py_list = [x for x in unsortedPyStoreArray]

# sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getSensorID().getLayerNumber(),
                x.getSensorID().getLadderNumber(),
                x.getSensorID().getSensorNumber(),
                not x.isUStrip(),
                x.getCellID()))

    def sortClusters(self, unsortedPyStoreArray):

        # convert to a python-list to be abple to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getSensorID().getLayerNumber(),
                x.getSensorID().getLadderNumber(),
                x.getSensorID().getSensorNumber(),
                not x.isUCluster()))

    def initialize(self):
        self.outputFileName = 'test_run2520.root'

        self.resList = []
        self.spList = []
        self.cogList = []

        geoCache = Belle2.VXD.GeoCache.getInstance()

        self.Evt = 0
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerList0 = []
            layerList1 = []
            layerList2 = []
            self.resList.append(layerList0)
            self.spList.append(layerList1)
            self.cogList.append(layerList2)
            # layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderList0 = []
                ladderList1 = []
                ladderList2 = []
                layerList0.append(ladderList0)
                layerList1.append(ladderList1)
                layerList2.append(ladderList2)
                # ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorList0 = []
                    sensorList1 = []
                    sensorList2 = []
                    ladderList0.append(sensorList0)
                    ladderList1.append(sensorList1)
                    ladderList2.append(sensorList2)
                    # sensorNumber = sensor.getSensorNumber()
                    for side in range(2):
                        sideList0 = []
                        sideList1 = []
                        sideList2 = []
                        sensorList0.append(sideList0)
                        sensorList1.append(sideList1)
                        sensorList2.append(sideList2)

        for i in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerN = i.getLayerNumber()
            li = layerN - 3
            for j in geoCache.getLadders(i):
                ladderN = j.getLadderNumber()
                ldi = ladderN - 1
                for k in geoCache.getSensors(j):
                    sensorN = k.getSensorNumber()
                    si = sensorN - 1
                    for s in range(2):
                        for t in range(4):
                            self.resList[li][ldi][si][s].append(
                                TH1F("res" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 100, -100, 0))
                            self.spList[li][ldi][si][s].append(
                                TH2D("sp" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 50, -100, 0, 80, -80, 80))
                            self.cogList[li][ldi][si][s].append(
                                TH1F("cog" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 100, -100, 0))

        # self.fit = TF1("fit",'[0] + [1]*TMath::Sin(x/[2])',-60,60)
        self.retta = TF1("retta", '[0] + [1]*x', -100, -10)
        self.gaus = TF1("gaus", 'gaus(0)', -100, 0)

    def event(self):
        timeClusterU = 0
        timeClusterV = 0
        sideIndex = 0
        TBIndexU = 0
        TBIndexV = 0
        self.Evt = self.Evt + 1

        svdRecoDigits_list = Belle2.PyStoreArray(svd_recoDigits)
        svdRecoDigits_sorted = self.sortDigits(svdRecoDigits_list)
        svdClusters_list = Belle2.PyStoreArray(svd_Clusters)
        svdClusters_sorted = self.sortClusters(svdClusters_list)
        svdTracks_list = Belle2.PyStoreArray(svd_Tracks)
        svdRecoTracks_list = Belle2.PyStoreArray(svd_RecoTracks)
        cdcEventT0 = Belle2.PyStoreObj(cdc_Time0)

        for i, tracks in enumerate(svdTracks_list):  # per ogni traccia prende un solo cluster
            svdTracks = tracks
            svdTracks_rel_RecoTracks = svdTracks.getRelatedTo("RecoTracks")
            svdClusters_rel_RecoTracks = svdTracks_rel_RecoTracks.getRelationsFrom("SVDClusters")

            if len(svdClusters_rel_RecoTracks) == 0:
                hasCluster = False
            else:
                hasCluster = True
            print("Cluster: " + str(hasCluster))
            if hasCluster:
                for svdClusters_rel_RecoTracks_cl in svdClusters_rel_RecoTracks:
                    svdRecoDigits_rel_Clusters = svdClusters_rel_RecoTracks_cl.getRelatedTo("SVDRecoDigits")
                    if svdRecoDigits_rel_Clusters is None:
                        hasRecoDigits = False
                    else:
                        hasRecoDigits = True
                    print("Reco: " + str(hasRecoDigits))
                    # CLUSTERS
                    timeCluster = svdClusters_rel_RecoTracks_cl.getClsTime()
                    layerCluster = svdClusters_rel_RecoTracks_cl.getSensorID().getLayerNumber()
                    layerIndex = layerCluster - 3
                    sensorCluster = svdClusters_rel_RecoTracks_cl.getSensorID().getSensorNumber()
                    sensorIndex = sensorCluster - 1
                    ladderCluster = svdClusters_rel_RecoTracks_cl.getSensorID().getLadderNumber()
                    ladderIndex = ladderCluster - 1
                    sideCluster = svdClusters_rel_RecoTracks_cl.isUCluster()
                    if sideCluster:
                        sideIndex = 1
                    else:
                        sideIndex = 0

                    hasTimezero = cdcEventT0.hasEventT0()
                    print("Time: " + str(hasTimezero))
                    if hasTimezero and hasCluster and hasRecoDigits:
                        print("DENTRO")
                        TBClusters = svdRecoDigits_rel_Clusters.getModeByte().getTriggerBin()
                        TBIndex = ord(TBClusters)
                        tZero = cdcEventT0.getEventT0()
                        tZeroSync = tZero - 7.8625 * (3 - TBIndex)

                        resHist = self.resList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        resHist.Fill(timeCluster - tZeroSync)
                        spHist = self.spList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        spHist.Fill(timeCluster, tZeroSync)
                        cogHist = self.cogList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cogHist.Fill(timeCluster)

    def terminate(self):
        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDCoGTimeCalibrations.t_payload()

        timeCal = SVDCoGCalibrationFunction()
        tbBias = [0, 0, 0, 0]
        tbSlope = [0, 0, 0, 0]

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            li = layerNumber - 3
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                ldi = ladderNumber - 1
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    si = sensorNumber - 1
                    for side in range(2):
                        for tb in range(4):
                            res = self.resList[li][ldi][si][side][tb]
                            res.Fit(self.gaus, "R")
                            res.Write()
                            cog = self.cogList[li][ldi][si][side][tb]
                            cog.Write()
                            sp = self.spList[li][ldi][si][side][tb]
                            pfxsp = sp.ProfileX()
                            self.retta.SetParameters(-50, 1.5)
                            pfxsp.Fit(self.retta, "R")
                            sp.Write()
                            pfxsp.Write()
                            q = self.retta.GetParameter(0)
                            q_err = self.retta.GetParError(0)
                            m = self.retta.GetParameter(1)
                            m_err = self.retta.GetParError(1)
                            tbBias[tb] = q
                            tbScale[tb] = m
                        timeCal.set_bias(tbBias[0], tbBias[1], tbBias[2], tbBias[3])
                        timeCal.set_scale(tbScale[0], tbScale[1], tbScale[2], tbScale[3])
                        print("setting CoG calibration for " + str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, timeCal)

        Belle2.Database.Instance().storeData(Belle2.SVDCoGTimeCalibrations.name, payload, iov)

        tfile.Close()


use_database_chain()
use_central_database("data_reprocessing_prod4", LogLevel.WARNING)
use_local_database("localDB/database.txt", "localDB")

main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileName', inputFile)
main.add_module(rootinput)

main.add_module("Gearbox")
main.add_module("Geometry", useDB=True)

main.add_module(SVDCoGTimeCalibrationImporterModule())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
