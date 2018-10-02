# !/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2, TFile, TTree, TH1F, TCanvas, TH2F, TGraph, TFitResultPtr
from ROOT import TMultiGraph, TH2D, TLegend, TROOT, gROOT, TF1, TMath, gStyle
import os
import numpy
import math
import random
from array import array
import basf2
from ROOT.Belle2 import SVDCoGCalibrationFunction
from ROOT.Belle2 import SVDCoGTimeCalibrations

import matplotlib.pyplot as plt
import simulation

hasCluster = True
hasRecoDigits = True

# inputFile = './cDST_run2520.root' #ARGHH, mancano SVDRecoDigits e SVDClusters
# inputFile = '/home/belle2/lgcorona/myHead_lgcorona1/workdir/svd/svd_phase2_scripts/Run2520
# /rootOutput/SVDRootOutput_data_exp3_Run2520_beam.0003.02520.root'
# inputFile = 'SVDRootOutput_data_exp3_Run2520_raw.physics.hlt_hadron.0003.02520_small.root'
# inputFile = '/home/belle2/lgcorona/myHead_lgcorona1/workdir/svd/
# svd_phase2_scripts/Run2733WithCalPeak/rootOutput/SVDRootOutput_data_exp3_coll_Run2733_beam.0003.02733.root'
inputFile = '/home/belle2/lgcorona/myHead_lgcorona1/workdir/svd/\
            svd_phase2_scripts/SVDRootOutput_data_exp3_Run2733_raw.physics.hlt_hadron.0003.02733.root'
svd_recoDigits = "SVDRecoDigits"
cdc_Time0 = "EventT0"
svd_Clusters = "SVDClusters"
svd_Tracks = "Tracks"
svd_RecoTracks = "RecoTracks"

set_random_seed(11)

gROOT.SetBatch(True)
gStyle.SetOptFit(11111111)


class SVDCoGTimeCalibrationImporterModule(basf2.Module):

    def initialize(self):
        self.outputFileName = 'run2733_corr_Err_Nofit.root'

        self.resList = []
        self.spList = []
        self.cogList = []
        self.cdcList = []
        self.snrList = []
        self.errT0List = []
        self.errCOGList = []
        self.meanCOGList = []

        self.sumCOGList = []
        self.sumCDCList = []
        self.sumCOG2List = []
        self.sumCDC2List = []
        self.sumCOGCDCList = []
        self.sumSigmaCDCList = []
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
            errlayerList0 = []
            errlayerList1 = []
            errlayerList2 = []
            errlayerList3 = []
            errlayerList4 = []
            errlayerList5 = []
            errlayerList6 = []
            self.resList.append(layerList0)
            self.spList.append(layerList1)
            self.cogList.append(layerList2)
            self.cdcList.append(layerList3)
            self.snrList.append(layerList4)
            self.errT0List.append(layerList5)
            self.errCOGList.append(layerList6)
            self.meanCOGList.append(layerList7)
            self.sumCOGList.append(errlayerList0)
            self.sumCDCList.append(errlayerList1)
            self.sumCOG2List.append(errlayerList2)
            self.sumCDC2List.append(errlayerList3)
            self.sumCOGCDCList.append(errlayerList4)
            self.sumSigmaCDCList.append(errlayerList5)
            self.nList.append(errlayerList6)
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
                errladderList0 = []
                errladderList1 = []
                errladderList2 = []
                errladderList3 = []
                errladderList4 = []
                errladderList5 = []
                errladderList6 = []
                layerList0.append(ladderList0)
                layerList1.append(ladderList1)
                layerList2.append(ladderList2)
                layerList3.append(ladderList3)
                layerList4.append(ladderList4)
                layerList5.append(ladderList5)
                layerList6.append(ladderList6)
                layerList7.append(ladderList7)
                errlayerList0.append(errladderList0)
                errlayerList1.append(errladderList1)
                errlayerList2.append(errladderList2)
                errlayerList3.append(errladderList3)
                errlayerList4.append(errladderList4)
                errlayerList5.append(errladderList5)
                errlayerList6.append(errladderList6)
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
                    errsensorList0 = []
                    errsensorList1 = []
                    errsensorList2 = []
                    errsensorList3 = []
                    errsensorList4 = []
                    errsensorList5 = []
                    errsensorList6 = []
                    ladderList0.append(sensorList0)
                    ladderList1.append(sensorList1)
                    ladderList2.append(sensorList2)
                    ladderList3.append(sensorList3)
                    ladderList4.append(sensorList4)
                    ladderList5.append(sensorList5)
                    ladderList6.append(sensorList6)
                    ladderList7.append(sensorList7)
                    errladderList0.append(errsensorList0)
                    errladderList1.append(errsensorList1)
                    errladderList2.append(errsensorList2)
                    errladderList3.append(errsensorList3)
                    errladderList4.append(errsensorList4)
                    errladderList5.append(errsensorList5)
                    errladderList6.append(errsensorList6)
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
                        errsideList0 = []
                        errsideList1 = []
                        errsideList2 = []
                        errsideList3 = []
                        errsideList4 = []
                        errsideList5 = []
                        errsideList6 = []
                        sensorList0.append(sideList0)
                        sensorList1.append(sideList1)
                        sensorList2.append(sideList2)
                        sensorList3.append(sideList3)
                        sensorList4.append(sideList4)
                        sensorList5.append(sideList5)
                        sensorList6.append(sideList6)
                        sensorList7.append(sideList7)
                        errsensorList0.append(errsideList0)
                        errsensorList1.append(errsideList1)
                        errsensorList2.append(errsideList2)
                        errsensorList3.append(errsideList3)
                        errsensorList4.append(errsideList4)
                        errsensorList5.append(errsideList5)
                        errsensorList6.append(errsideList6)

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
                            self.errT0List[li][ldi][si][s].append(
                                TH1F("errT0" + "_" + str(k) + "." + str(s) + "." + str(t), " ", 100, 0, 100))
                            self.sumCOGList[li][ldi][si][s].append(0)
                            self.sumCDCList[li][ldi][si][s].append(0)
                            self.sumCOG2List[li][ldi][si][s].append(0)
                            self.sumCDC2List[li][ldi][si][s].append(0)
                            self.sumCOGCDCList[li][ldi][si][s].append(0)
                            self.sumSigmaCDCList[li][ldi][si][s].append(0)
                            self.nList[li][ldi][si][s].append(0)

        self.AlphaErrHist = TH1F("#alpha_err_distribution", " ", 125, 0, 0.5)
        self.BetaErrHist = TH1F("#beta_err_distribution", " ", 150, 0, 30)
        self.RmsTCogHist = TH1F("RMS_CoG_distribution", " ", 100, 0, 20)
        self.TCogErrHist = TH1F("CoG_err", " ", 100, 0, 50)
        self.ChiSquareHist = TH1F("chi", " ", 200, 0, 2000)
        self.errCOG = TH1F("errCOG", " ", 100, 0, 20)
        self.resAnalyticalCOG = TH1F("resAnalyticCOG", " ", 100, 0, 20)
        self.resCOG = TH1F("resCOG", " ", 100, 0, 20)
        self.meanCOG = TH1F("meanCOG", " ", 100, -20, 20)

        # self.fit = TF1("fit",'[0] + [1]*TMath::Sin(x/[2])',-60,60)
        self.retta = TF1("retta", '[0] + [1]*x', -150, 100)
        self.gaus = TF1("gaus", 'gaus(0)', -150, 100)

    def event(self):
        timeClusterU = 0
        timeClusterV = 0
        sideIndex = 0
        TBIndexU = 0
        TBIndexV = 0
        self.Evt = self.Evt + 1
        self.Counter = 0

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
                        errT0Hist = self.errT0List[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        errT0Hist.Fill(tZero_err)
                        # What is needed to evaluate the error
                        cog = self.sumCOGList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cog = cog + timeCluster
                        self.sumCOGList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = cog

                        cdc = self.sumCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cdc = cdc + tZeroSync
                        self.sumCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = cdc

                        cog2 = self.sumCOG2List[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cog2 = cog2 + timeCluster * timeCluster
                        self.sumCOG2List[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = cog2

                        cdc2 = self.sumCDC2List[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cdc2 = cdc2 + tZeroSync * tZeroSync
                        self.sumCDC2List[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = cdc2

                        cogcdc = self.sumCOGCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        cogcdc = cogcdc + timeCluster * tZeroSync
                        self.sumCOGCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = cogcdc

                        errcdc = self.sumSigmaCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        errcdc = errcdc + tZero_err * tZero_err
                        self.sumSigmaCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = errcdc
                        # err = self.sumSigmaCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        # print(str(err))
                        n = self.nList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        n = n + 1
                        self.nList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] = n
                        # m = self.sumCOGList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex]
                        # print(str(m))

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
        # Res of TCoG corrected
        tbTCogErr = [1, 1, 1, 1]
        tbTCogMean = [1, 1, 1, 1]

        sensorNoFitted = []

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
                            # Resolution distribution Histograms with Gaussian Fit
                            res = self.resList[li][ldi][si][side][tb]
                            res.Fit(self.gaus, "R")
                            res.Write()
                            # COG Distribution Histograms
                            cog = self.cogList[li][ldi][si][side][tb]
                            cog.Write()
                            # CDC EventT0 Distribution Histograms
                            cdc = self.cdcList[li][ldi][si][side][tb]
                            cdc.Write()
                            errt0 = self.errT0List[li][ldi][si][side][tb]
                            errt0.Write()
                            # SNR Distribution Histograms
                            snr = self.snrList[li][ldi][si][side][tb]
                            snrMean = snr.GetMean()
                            snr.Write()
                            # ScatterPlot Histograms with Linear Fit
                            sp = self.spList[li][ldi][si][side][tb]
                            covscalebias = sp.GetCovariance()
                            # pfxsp = sp.ProfileX("_pfx",1,-1,"i")
                            pfxsp = sp.ProfileX()
                            self.retta.SetParameters(-50, 1.5)
                            pfxsp.Fit(self.retta, "R")
                            # fitResult = int(TFitResultPtr(pfxsp.Fit(self.retta,"S")))
                            fitResult = -1
                            chi = self.retta.GetChisquare()
                            self.ChiSquareHist.Fill(chi)
                            # 0: The fit converges, -1: The fit does not converge
                            # print(str(fitResult))
                            sp.Write()
                            pfxsp.Write()
                            # Extraction of Fit parameters
                            q = self.retta.GetParameter(0)
                            q_err = self.retta.GetParError(0)
                            m = self.retta.GetParameter(1)
                            m_err = self.retta.GetParError(1)
                            # tbBias[tb] = q
                            # tbScale[tb] = m
                            # tbBias_err[tb] = q_err
                            # tbScale_err[tb] = m_err
                            sigmarescog = self.gaus.GetParameter(2)
                            sigmarescog_err = self.gaus.GetParError(2)
                            meanrescog = self.gaus.GetParameter(1)
                            meanrescog_err = self.gaus.GetParError(1)
                            # print("Beta: " + str(q) + " Slope: " + str(m))
                            if fitResult == -1 or chi >= 200:
                                if sp.GetRMS() != 0:
                                    m = sp.GetCovariance() / pow(sp.GetRMS(1), 2)
                                    # m = sp.GetCovariance()/cog.GetRMS()
                                    m_err = 2 / pow(sp.GetRMS(), 3) * sp.GetRMSError()
                                q = sp.GetMean(2) - m * sp.GetMean(1)
                                q_err = math.sqrt(pow(sp.GetMeanError(2), 2) +
                                                  pow(m * sp.GetMeanError(1), 2) + pow(m_err * sp.GetMean(1), 2))
                                self.Counter = self.Counter + 1
                                sensorNoFitted.append("Sensors No Fitted or Fitted with Chi2 > 200: " +
                                                      str(sensor) + "." + str(side) + "." + str(tb))
                            print("Beta: " + str(q) + " Slope: " + str(m))
                            self.AlphaErrHist.Fill(m_err)
                            self.BetaErrHist.Fill(q_err)
                            n = self.nList[li][ldi][si][side][tb]
                            print("N " + str(n))
                            sumsigmacdc = self.sumSigmaCDCList[li][ldi][si][side][tb]
                            sumcog = self.sumCOGList[li][ldi][si][side][tb]
                            sumcdc = self.sumCDCList[li][ldi][si][side][tb]
                            sumcog2 = self.sumCOG2List[li][ldi][si][side][tb]
                            sumcdc2 = self.sumCDC2List[li][ldi][si][side][tb]
                            sumcogcdc = self.sumCOGCDCList[li][ldi][si][side][tb]
                            if n != 0:
                                sigmacdc2 = sumsigmacdc / n
                                biascog = m * sumcog / n + q - sumcdc / n
                                sigmaTCog = m * m * sumcog2 / n + q * q + sumcdc2 / n + \
                                    2 * m * q * sumcog / n - 2 * m * sumcogcdc / n - 2 * q * sumcdc / n
                                # sigmaTCog is the estimation of residual (alpha*tcogRAW + beta - tCDC)
                                # TCogErr = sigmaTCog - sigmacdc2 - biascog*biascog
                                TCogErr = math.sqrt(pow(cog.GetMean() * m_err, 2) + pow(q_err, 2) + pow(m * sigmarescog, 2))
                                TCogMean = m * sumcog / n + q - sumcdc / n
                                # iprint("PROVA " + str(n) + " " + str(sigmaTCog) + " " +
                                # str(biascog*biascog) + " " + str(sigmacdc2) + " " + str(TCogErr))
                            else:
                                TCogErr = math.sqrt(pow(cog.GetMean() * m_err, 2) + pow(q_err, 2) + pow(m * sigmarescog, 2))
                                TCogMean = m * meanrescog + q
                                sigmaTCog = 1

                            tbBias[tb] = q
                            tbScale[tb] = m
                            tbBias_err[tb] = q_err
                            tbScale_err[tb] = m_err
                            tbCovScaleBias[tb] = covscalebias
                            tbTCogErr[tb] = math.sqrt(TCogErr)
                            tbTCogMean[tb] = TCogMean
                            self.errCOG.Fill(TCogErr)
                            self.meanCOG.Fill(TCogMean)
                            self.resCOG.Fill(sigmarescog)
                            self.resAnalyticalCOG.Fill(math.sqrt(sigmaTCog))

                        timeCal.set_bias(tbBias[0], tbBias[1], tbBias[2], tbBias[3])
                        timeCal.set_scale(tbScale[0], tbScale[1], tbScale[2], tbScale[3])
                        print("setting CoG calibration for " + str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, timeCal)

        self.errCOG.Write()
        self.meanCOG.Write()
        self.AlphaErrHist.Write()
        self.BetaErrHist.Write()
        # self.RmsTCogHist.Write()
        # self.TCogErrHist.Write()
        self.ChiSquareHist.Write()
        self.resCOG.Write()
        self.resAnalyticalCOG.Write()

        Belle2.Database.Instance().storeData(Belle2.SVDCoGTimeCalibrations.name, payload, iov)

        tfile.Close()
        print("Number of not fitted graphs: " + str(self.Counter))
        for i in range(len(sensorNoFitted)):
            print(str(sensorNoFitted[i]))


# use_database_chain()
reset_database()
use_database_chain()
use_central_database("data_reprocessing_prod4", LogLevel.WARNING)
use_central_database("svdonly_phase2analysis_with_master", LogLevel.WARNING)
use_local_database("localDB2733NofitCorr/database2733NofitCorr.txt", "localDB2733NofitCorr")

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
