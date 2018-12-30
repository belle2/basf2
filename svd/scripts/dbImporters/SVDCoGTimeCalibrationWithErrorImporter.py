# !/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################################
#
# evaluates the CoG corrections, create a localDB
# with the corrections and a root file to check
# the corrections
#
# usage: basf2 SVDCoGTimeCalibratinWithErrorImporter localDB filename
# localDB = name of the local DB folder
# filename = single root file, or file with the list of reconstructed files
#
# this script can be launched with launch_calibration_cog.sh in the
# B2SVD project, svd_CoGHitTime_calibration repository
#
#################################################################################


from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2, TFile, TTree, TH1F, TH2F, TH2D, TGraph, TFitResultPtr
from ROOT import TROOT, gROOT, TF1, TMath, gStyle, gDirectory
import os
import numpy
import math
import random
from array import array
import basf2
import sys
from ROOT.Belle2 import SVDCoGCalibrationFunction
from ROOT.Belle2 import SVDCoGTimeCalibrations

import matplotlib.pyplot as plt
import simulation

hasCluster = True
hasRecoDigits = True


localdb = sys.argv[1]
outputFile = "CoGCorrectionMonitor_" + localdb + ".root"
trk_outputFile = "TrackFilterControlNtuples_" + localdb + ".root"
nSVD = 6
nCDC = 1
pVal = 0.0  # 0001

filename = sys.argv[2]
inputFileList = []
if filename.rfind(".root") != -1:
    inputFileList.append(filename)
else:
    with open(filename, 'r') as f:
        inputFileList = [line.strip() for line in f]


svd_recoDigits = "SVDRecoDigits"
cdc_Time0 = "EventT0"
svd_Clusters = "SVDClusters"
svd_Tracks = "Tracks"
# svd_Tracks = "SelectedTracks"
svd_RecoTracks = "RecoTracks"

gROOT.SetBatch(True)
gStyle.SetOptFit(11111111)


class SVDCoGTimeCalibrationImporterModule(basf2.Module):

    def initialize(self):
        self.outputFileName = outputFile

        self.resList = []
        self.spList = []
        self.cogList = []
        self.cdcList = []
        self.snrList = []
        self.nList = []

        geoCache = Belle2.VXD.GeoCache.getInstance()

        self.Evt = 0
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerList0 = []
            layerList1 = []
            layerList2 = []
            layerList3 = []
            layerList4 = []
            layerList5 = []
            layerList6 = []
            layerList7 = []
            layerList8 = []
            self.resList.append(layerList0)
            self.spList.append(layerList1)
            self.cogList.append(layerList2)
            self.cdcList.append(layerList3)
            self.snrList.append(layerList4)
            self.nList.append(layerList8)
            # layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderList0 = []
                ladderList1 = []
                ladderList2 = []
                ladderList3 = []
                ladderList4 = []
                ladderList5 = []
                ladderList6 = []
                ladderList7 = []
                ladderList8 = []
                layerList0.append(ladderList0)
                layerList1.append(ladderList1)
                layerList2.append(ladderList2)
                layerList3.append(ladderList3)
                layerList4.append(ladderList4)
                layerList5.append(ladderList5)
                layerList6.append(ladderList6)
                layerList7.append(ladderList7)
                layerList8.append(ladderList8)
                # ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorList0 = []
                    sensorList1 = []
                    sensorList2 = []
                    sensorList3 = []
                    sensorList4 = []
                    sensorList5 = []
                    sensorList6 = []
                    sensorList7 = []
                    sensorList8 = []
                    ladderList0.append(sensorList0)
                    ladderList1.append(sensorList1)
                    ladderList2.append(sensorList2)
                    ladderList3.append(sensorList3)
                    ladderList4.append(sensorList4)
                    ladderList5.append(sensorList5)
                    ladderList6.append(sensorList6)
                    ladderList7.append(sensorList7)
                    ladderList8.append(sensorList8)
                    # sensorNumber = sensor.getSensorNumber()
                    for side in range(2):
                        sideList0 = []
                        sideList1 = []
                        sideList2 = []
                        sideList3 = []
                        sideList4 = []
                        sideList5 = []
                        sideList6 = []
                        sideList7 = []
                        sideList8 = []
                        sensorList0.append(sideList0)
                        sensorList1.append(sideList1)
                        sensorList2.append(sideList2)
                        sensorList3.append(sideList3)
                        sensorList4.append(sideList4)
                        sensorList5.append(sideList5)
                        sensorList6.append(sideList6)
                        sensorList7.append(sideList7)
                        sensorList8.append(sideList8)

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
                                TH1F("res" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 200, -100, 100))
                            self.spList[li][ldi][si][s].append(
                                TH2D("sp" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 300, -150, 150, 300, -150, 150))
                            self.cogList[li][ldi][si][s].append(
                                TH1F("cog" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 200, -100, 100))
                            self.cdcList[li][ldi][si][s].append(
                                TH1F("cdc" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 200, -100, 100))
                            self.snrList[li][ldi][si][s].append(
                                TH1F("snr" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 100, 0, 100))
                            self.nList[li][ldi][si][s].append(0)

        self.AlphaUTB = TH2F("alphaVsTB_U", " ", 400, 0.5, 2, 4, 0, 4)
        self.AlphaUTB.GetXaxis().SetTitle("alpha")
        self.AlphaUTB.GetYaxis().SetTitle("trigger bin")
        self.AlphaVTB = TH2F("alphaVsTB_V", " ", 400, 0.5, 2, 4, 0, 4)
        self.AlphaVTB.GetXaxis().SetTitle("alpha")
        self.AlphaVTB.GetYaxis().SetTitle("trigger bin")
        self.BetaUTB = TH2F("betaVsTB_U", " ", 200, -100, 100, 4, 0, 4)
        self.BetaUTB.GetXaxis().SetTitle("beta (ns)")
        self.BetaUTB.GetYaxis().SetTitle("trigger bin")
        self.BetaVTB = TH2F("betaVsTB_V", " ", 200, -100, 100, 4, 0, 4)
        self.BetaVTB.GetXaxis().SetTitle("beta (ns)")
        self.BetaVTB.GetYaxis().SetTitle("trigger bin")

        self.MeanHistVTB = TH2F("meanHistVsTB_V", " ", 100, -10, 10, 4, 0, 4)
        self.MeanHistVTB.GetXaxis().SetTitle("distribution mean (ns)")
        self.MeanHistVTB.GetYaxis().SetTitle("trigger bin")
        self.MeanHistUTB = TH2F("meanHistVsTB_U", " ", 100, -10, 10, 4, 0, 4)
        self.MeanHistUTB.GetXaxis().SetTitle("distribution mean (ns)")
        self.MeanHistUTB.GetYaxis().SetTitle("trigger bin")
        self.RMSHistVTB = TH2F("rmsHistVsTB_V", " ", 100, 0, 10, 4, 0, 4)
        self.RMSHistVTB.GetXaxis().SetTitle("distribution RMS (ns)")
        self.RMSHistVTB.GetYaxis().SetTitle("trigger bin")
        self.RMSHistUTB = TH2F("rmsHistVsTB_U", " ", 100, 0, 10, 4, 0, 4)
        self.RMSHistUTB.GetXaxis().SetTitle("distribution RMS (ns)")
        self.RMSHistUTB.GetYaxis().SetTitle("trigger bin")
        self.MeanFitVTB = TH2F("meanFitVsTB_V", " ", 100, -10, 10, 4, 0, 4)
        self.MeanFitVTB.GetXaxis().SetTitle("fit mean (ns)")
        self.MeanFitVTB.GetYaxis().SetTitle("trigger bin")
        self.MeanFitUTB = TH2F("meanFitVsTB_U", " ", 100, -10, 10, 4, 0, 4)
        self.MeanFitUTB.GetXaxis().SetTitle("fit mean (ns)")
        self.MeanFitUTB.GetYaxis().SetTitle("trigger bin")
        self.RMSFitUTB = TH2F("rmsFitVsTB_U", " ", 100, 0, 10, 4, 0, 4)
        self.RMSFitUTB.GetXaxis().SetTitle("fit sigma (ns)")
        self.RMSFitUTB.GetYaxis().SetTitle("trigger bin")
        self.RMSFitVTB = TH2F("rmsFitVsTB_V", " ", 100, 0, 10, 4, 0, 4)
        self.RMSFitVTB.GetXaxis().SetTitle("fit sigma (ns)")
        self.RMSFitVTB.GetYaxis().SetTitle("trigger bin")

        self.gaus = TF1("gaus", 'gaus(0)', -150, 100)

    def event(self):
        timeClusterU = 0
        timeClusterV = 0
        sideIndex = 0
        TBIndexU = 0
        TBIndexV = 0
        self.Evt = self.Evt + 1

        svdTracks_list = Belle2.PyStoreArray(svd_Tracks)
        svdRecoTracks_list = Belle2.PyStoreArray(svd_RecoTracks)
        cdcEventT0 = Belle2.PyStoreObj(cdc_Time0)

        for i, tracks in enumerate(svdTracks_list):
            svdTracks = tracks
            svdTracks_rel_RecoTracks = svdTracks.getRelatedTo("RecoTracks")
            svdClusters_rel_RecoTracks = svdTracks_rel_RecoTracks.getRelationsFrom("SVDClusters")

            if len(svdClusters_rel_RecoTracks) == 0:
                hasCluster = False
            else:
                hasCluster = True
                # print("Cluster: " + str(hasCluster))
            if hasCluster:
                for svdClusters_rel_RecoTracks_cl in svdClusters_rel_RecoTracks:
                    svdRecoDigits_rel_Clusters = svdClusters_rel_RecoTracks_cl.getRelatedTo("SVDRecoDigits")
                    if svdRecoDigits_rel_Clusters is None:
                        hasRecoDigits = False
                    else:
                        hasRecoDigits = True
                    # print("Reco: " + str(hasRecoDigits))
                    # CLUSTERS
                    timeCluster = svdClusters_rel_RecoTracks_cl.getClsTime()
                    snrCluster = svdClusters_rel_RecoTracks_cl.getSNR()
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
                    # print("Time: " + str(hasTimezero))
                    if hasTimezero and hasCluster and hasRecoDigits:
                        TBClusters = svdRecoDigits_rel_Clusters.getModeByte().getTriggerBin()
                        TBIndex = ord(TBClusters)
                        tZero = cdcEventT0.getEventT0()
                        # tZero_err = cdcEventT0.getEventT0Uncertainty()
                        tZero_err = 5.1
                        tZeroSync = tZero - 7.8625 * (3 - TBIndex)
                        # print(str(tZero_err))

                        resHist = self.resList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        resHist.Fill(timeCluster - tZeroSync)
                        spHist = self.spList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        # spHist.Fill(timeCluster, 1.3*timeCluster - 50 + random.gauss(0,10))
                        spHist.Fill(timeCluster, tZeroSync)
                        cogHist = self.cogList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cogHist.Fill(timeCluster)
                        cdcHist = self.cdcList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cdcHist.Fill(tZeroSync)
                        snrHist = self.snrList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        snrHist.Fill(snrCluster)

                        n = self.nList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        n = n + 1
                        self.nList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = n

    def terminate(self):

        tfile = TFile(self.outputFileName, 'recreate')
        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDCoGTimeCalibrations.t_payload()

        timeCal = SVDCoGCalibrationFunction()
        # Bias and Scale
        tbBias = [-50, -50, -50, -50]
        tbScale = [1, 1, 1, 1]
        tbBias_err = [1, 1, 1, 1]
        tbScale_err = [1, 1, 1, 1]
        tbCovScaleBias = [1, 1, 1, 1]

        geoCache = Belle2.VXD.GeoCache.getInstance()
        gDirectory.mkdir("plots")
        gDirectory.cd("plots")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            li = layerNumber - 3
            gDirectory.mkdir("layer" + str(layer))
            gDirectory.cd("layer" + str(layer))
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                ldi = ladderNumber - 1
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    si = sensorNumber - 1
                    for side in range(2):
                        for tb in range(4):
                            # Resolution distribution Histograms with Gaussian Fit
                            res = self.resList[li][ldi][si][side][tb]
                            fitResult = int(TFitResultPtr(res.Fit(self.gaus, "R")))

                            if res.GetEntries() > 5:
                                if side == 1:
                                    self.MeanHistUTB.Fill(res.GetMean(), tb)
                                    self.RMSHistUTB.Fill(res.GetRMS(), tb)
                                    if fitResult > -1:
                                        self.MeanFitUTB.Fill(self.gaus.GetParameter(1), tb)
                                        self.RMSFitUTB.Fill(self.gaus.GetParameter(2), tb)
                                else:
                                    self.MeanHistVTB.Fill(res.GetMean(), tb)
                                    self.RMSHistVTB.Fill(res.GetRMS(), tb)
                                    if fitResult > -1:
                                        self.MeanFitVTB.Fill(self.gaus.GetParameter(1), tb)
                                        self.RMSFitVTB.Fill(self.gaus.GetParameter(2), tb)

                            res.Write()
                            # COG Distribution Histograms
                            cog = self.cogList[li][ldi][si][side][tb]
                            cog.Write()
                            # CDC EventT0 Distribution Histograms
                            cdc = self.cdcList[li][ldi][si][side][tb]
                            cdc.Write()
                            # SNR Distribution Histograms
                            snr = self.snrList[li][ldi][si][side][tb]
                            snrMean = snr.GetMean()
                            snr.Write()
                            # ScatterPlot Histograms with Linear Fit
                            sp = self.spList[li][ldi][si][side][tb]
                            covscalebias = sp.GetCovariance()
                            pfxsp = sp.ProfileX()
                            sp.Write()
                            pfxsp.Write()

                            if sp.GetRMS() != 0:
                                m = sp.GetCovariance() / pow(sp.GetRMS(1), 2)
                                # m = sp.GetCovariance()/cog.GetRMS()
                                m_err = 2 / pow(sp.GetRMS(), 3) * sp.GetRMSError() * sp.GetCovariance()
                                q = sp.GetMean(2) - m * sp.GetMean(1)
                                q_err = math.sqrt(pow(sp.GetMeanError(2), 2) +
                                                  pow(m * sp.GetMeanError(1), 2) + pow(m_err * sp.GetMean(1), 2))
                            else:
                                m = 1
                                m_err = 0
                                q = 0
                                q_err = 0

                            if side == 1:
                                self.AlphaUTB.Fill(m, tb)
                                self.BetaUTB.Fill(q, tb)
                            else:
                                self.AlphaVTB.Fill(m, tb)
                                self.BetaVTB.Fill(q, tb)

                            n = self.nList[li][ldi][si][side][tb]

                            tbBias[tb] = q
                            tbScale[tb] = m
                            tbBias_err[tb] = q_err
                            tbScale_err[tb] = m_err

                        timeCal.set_bias(tbBias[0], tbBias[1], tbBias[2], tbBias[3])
                        timeCal.set_scale(tbScale[0], tbScale[1], tbScale[2], tbScale[3])
                        print("setting CoG calibration for " + str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, timeCal)
            gDirectory.cd("../")

        gDirectory.cd("../")
        self.AlphaUTB.Write()
        self.AlphaVTB.Write()
        self.BetaUTB.Write()
        self.BetaVTB.Write()
        self.MeanFitUTB.Write()
        self.MeanFitVTB.Write()
        self.RMSFitUTB.Write()
        self.RMSFitVTB.Write()
        self.MeanHistUTB.Write()
        self.MeanHistVTB.Write()
        self.RMSHistUTB.Write()
        self.RMSHistVTB.Write()

        Belle2.Database.Instance().storeData(Belle2.SVDCoGTimeCalibrations.name, payload, iov)

        tfile.Close()


use_database_chain()
use_local_database(localdb + "/database.txt", localdb, invertLogging=True)

main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileNames', inputFileList)
main.add_module(rootinput)

main.add_module("Gearbox")
main.add_module("Geometry", useDB=True)

# Track selection - NOT YET
trkFlt = register_module('TrackFilter')
trkFlt.param('outputFileName', trk_outputFile)
trkFlt.param('outputINArrayName', 'SelectedTracks')
trkFlt.param('outputOUTArrayName', 'ExcludedTracks')
trkFlt.param('min_NumHitSVD', nSVD)
trkFlt.param('min_NumHitCDC', nCDC)
trkFlt.param('min_Pvalue', pVal)
# trkFlt.logging.log_level = LogLevel.DEBUG
# main.add_module(trkFlt)

main.add_module(SVDCoGTimeCalibrationImporterModule())

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

print_path(main)

# Process events
process(main)

print(statistics)
