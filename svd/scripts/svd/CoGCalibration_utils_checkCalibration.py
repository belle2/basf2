# !/usr/bin/env python
# -*- coding: utf-8 -*-

#
# util module to check SVD CoG Calibration stored in a localDB
#
# called by SVDCoGTimeCalibrationCheck script in
# svd/scripts/caf/SVDCoGTimeCalibrationCheck.py
#


import basf2 as b2
from ROOT import (Belle2, TFile, TFitResultPtr, TH1F, TH2D, TTree, TF1,
                  gDirectory, gROOT)
# import numpy
import math

import numpy as np

cdc_Time0 = "EventT0"
svd_Clusters = "SVDClustersFromTracks"
svd_EventInfo = "SVDEventInfo"
# svd_EventMD = "EventMetaData"

gROOT.SetBatch(True)

# mode = True


class SVDCoGTimeCalibrationCheckModule(b2.Module):
    """
    Python class used for checking SVD CoG Calibration stored in a localDB
    """

    def fillLists(self, svdClusterFromTracks):
        """
        Function that fill the lists needed for the check of the calibration

        parameters:
             svdClusterFromTracks (SVDCluster): Cluster related to tracks
        """
        timeCluster = svdClusterFromTracks.getClsTime()
        snrCluster = svdClusterFromTracks.getSNR()
        sizeCluster = svdClusterFromTracks.getSize()
        chargeCluster = svdClusterFromTracks.getCharge()
        layerCluster = svdClusterFromTracks.getSensorID().getLayerNumber()
        layerIndex = layerCluster - 3
        sensorCluster = svdClusterFromTracks.getSensorID().getSensorNumber()
        sensorIndex = sensorCluster - 1
        ladderCluster = svdClusterFromTracks.getSensorID().getLadderNumber()
        ladderIndex = ladderCluster - 1
        sideCluster = svdClusterFromTracks.isUCluster()
        sideIndex = 0
        if sideCluster:
            sideIndex = 1

        hasTimezero = self.cdcEventT0.hasEventT0()
        if hasTimezero:
            tZero = self.cdcEventT0.getEventT0()
            tZeroError = self.cdcEventT0.getEventT0Uncertainty()

            # filling histograms
            resHist = self.resList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            resHist.Fill(timeCluster - tZero)
            spHist = self.spList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            spHist.Fill(timeCluster, tZero)
            cogHist = self.cogList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cogHist.Fill(timeCluster)
            cdcHist = self.cdcList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cdcHist.Fill(tZero)
            snrHist = self.snrList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            snrHist.Fill(snrCluster)
            sizeHist = self.sizeList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            sizeHist.Fill(sizeCluster)
            chargeHist = self.chargeList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            chargeHist.Fill(chargeCluster/1000.)
            cdcErrorHist = self.cdcErrorList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cdcErrorHist.Fill(tZeroError)

            #: counts the number of clusters
            self.NTOT = self.NTOT + 1

    def set_localdb(self, localDB):
        """
        Function that allows to set the localDB

        parameters:
             localDB (str): Name of the localDB used
        """
        #: set the name of the localDB used
        self.localdb = localDB

    def set_run_number(self, run):
        """
        Function that allows to save the run number

        parameters:
             run (int): run number
        """
        #: set the run number
        self.runnumber = run

    def set_exp_number(self, exp):
        """
        Function that allows to save the experiment number

        parameters:
             exp (int): experiment number
        """
        #: set the experiment number
        self.expnumber = exp

    def initialize(self):
        """
        Initialize object (histograms, lists, ...) used by the class
        """

        #: name of the output file
        self.outputFileName = "../caf/tree/SVDCoGCalibrationCheck_Exp" + \
            str(self.expnumber) + "_Run" + str(self.runnumber) + ".root"
        #: lists used to create the histograms for each TB :
        #: residuals
        self.resList = []
        #: scatterplot t0 vs cog
        self.spList = []
        #: cog
        self.cogList = []
        #: t0
        self.cdcList = []
        #: Cluster SNR
        self.snrList = []
        #: Cluster Size
        self.sizeList = []
        #: Cluster charge
        self.chargeList = []
        #: t0 no synchronized
        self.cdcErrorList = []

        geoCache = Belle2.VXD.GeoCache.getInstance()

        #: counts the number of events
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

            self.resList.append(layerList0)
            self.spList.append(layerList1)
            self.cogList.append(layerList2)
            self.cdcList.append(layerList3)
            self.snrList.append(layerList4)
            self.sizeList.append(layerList5)
            self.chargeList.append(layerList6)
            self.cdcErrorList.append(layerList7)

            for ladder in geoCache.getLadders(layer):
                ladderList0 = []
                ladderList1 = []
                ladderList2 = []
                ladderList3 = []
                ladderList4 = []
                ladderList5 = []
                ladderList6 = []
                ladderList7 = []

                layerList0.append(ladderList0)
                layerList1.append(ladderList1)
                layerList2.append(ladderList2)
                layerList3.append(ladderList3)
                layerList4.append(ladderList4)
                layerList5.append(ladderList5)
                layerList6.append(ladderList6)
                layerList7.append(ladderList7)

                for sensor in geoCache.getSensors(ladder):
                    sensorList0 = []
                    sensorList1 = []
                    sensorList2 = []
                    sensorList3 = []
                    sensorList4 = []
                    sensorList5 = []
                    sensorList6 = []
                    sensorList7 = []

                    ladderList0.append(sensorList0)
                    ladderList1.append(sensorList1)
                    ladderList2.append(sensorList2)
                    ladderList3.append(sensorList3)
                    ladderList4.append(sensorList4)
                    ladderList5.append(sensorList5)
                    ladderList6.append(sensorList6)
                    ladderList7.append(sensorList7)

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
                        self.resList[li][ldi][si].append(TH1F("res" + "_" + str(layerN) + "." +
                                                              str(ladderN) + "." + str(sensorN) + "." +
                                                              str(s), " ", 200, -100, 100))
                        self.spList[li][ldi][si].append(TH2D("sp" + "_" + str(layerN) + "." +
                                                             str(ladderN) + "." + str(sensorN) + "." +
                                                             str(s), " ", 300, -150, 150, 300, -150, 150))
                        self.cogList[li][ldi][si].append(TH1F("cog" + "_" + str(layerN) + "." +
                                                              str(ladderN) + "." + str(sensorN) + "." + str(s),
                                                              " ", 200, -100, 100))
                        self.cdcList[li][ldi][si].append(TH1F("cdc" + "_" + str(layerN) + "." +
                                                              str(ladderN) + "." + str(sensorN) + "." + str(s),
                                                              " ", 200, -100, 100))
                        self.snrList[li][ldi][si].append(TH1F("snr" + "_" + str(layerN) + "." +
                                                              str(ladderN) + "." + str(sensorN) + "." + str(s), " ", 100, 0, 100))
                        self.sizeList[li][ldi][si].append(TH1F("size" + "_" + str(layerN) + "." +
                                                               str(ladderN) + "." + str(sensorN) + "." + str(s), " ", 20, 0, 20))
                        self.chargeList[li][ldi][si].append(TH1F("charge" + "_" + str(layerN) + "." +
                                                                 str(ladderN) + "." + str(sensorN) + "." + str(s),
                                                                 " ", 100, 0, 200))
                        self.cdcErrorList[li][ldi][si].append(TH1F("cdcError" + "_" + str(layerN) + "." +
                                                                   str(ladderN) + "." + str(sensorN) + "." + str(s),
                                                                   " ", 200, 0, 50))

        #: distribution of EventT0
        self.EventT0Hist = TH1F("EventT0", " ", 160, -40, 40)

        #: gaus function used for fitting distributions
        self.gaus = TF1("gaus", 'gaus(0)', -150, 150)
        #: order 1 polynomium used for the calibration
        self.pol1 = TF1("pol1", "[0] + [1]*x", -30, 30)
        #: order 3 polynomium used for the calibration
        self.pol3 = TF1("pol3", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -50, 50)
        #: counts the number of clusters
        self.NTOT = 0

    def event(self):
        """
        Function that allows to cicle on the events
        """
        self.Evt = self.Evt + 1

        # fill EventT0 histogram
        #: registers PyStoreObj EventT0
        self.cdcEventT0 = Belle2.PyStoreObj(cdc_Time0)
        if self.cdcEventT0.hasEventT0():
            et0 = self.EventT0Hist
            et0.Fill(self.cdcEventT0.getEventT0())

        # fill plots
        svdCluster_list = Belle2.PyStoreArray(svd_Clusters)
        svd_evtInfo = Belle2.PyStoreObj(svd_EventInfo)
        clsTB = svd_evtInfo.getModeByte().getTriggerBin()
        #: trigger bin
        self.TB = ord(clsTB)

        for svdCluster in svdCluster_list:
            self.fillLists(svdCluster)

    def terminate(self):
        """
        Terminates te class and produces the output rootfile
        """

        layerNumberTree = np.zeros(1, dtype=int)
        ladderNumberTree = np.zeros(1, dtype=int)
        sensorNumberTree = np.zeros(1, dtype=int)
        sideTree = np.zeros(1, dtype=int)
        mean = np.zeros(1, dtype=float)
        meanerr = np.zeros(1, dtype=float)
        width = np.zeros(1, dtype=float)
        widtherr = np.zeros(1, dtype=float)
        cogmean = np.zeros(1, dtype=float)
        cogmeanerr = np.zeros(1, dtype=float)
        runnumber = np.zeros(1, dtype=int)
        runnumber[0] = int(self.runnumber)
        expnumber = np.zeros(1, dtype=int)
        expnumber[0] = int(self.expnumber)
        evttime = np.zeros(1, dtype=float)
        jitter = np.zeros(1, dtype=float)
        cdcErrorMean = np.zeros(1, dtype=float)
        clsSize = np.zeros(1, dtype=float)
        clsCharge = np.zeros(1, dtype=float)
        clsSNR = np.zeros(1, dtype=float)

        tfile = TFile(self.outputFileName, 'recreate')
        tfile.cd()
        tree = TTree("cog", "tree")
        tree.Branch("Layer", layerNumberTree, "Layer/I")
        tree.Branch("Ladder", ladderNumberTree, "Ladder/I")
        tree.Branch("Sensor", sensorNumberTree, "Sensor/I")
        tree.Branch("Side", sideTree, "Side/I")
        tree.Branch("ResMean", mean, "ResMean/D")
        tree.Branch("ResMeanErr", meanerr, "ResMeanErr/D")
        tree.Branch("ResWidth", width, "ResWidth/D")
        tree.Branch("ResWidthErr", widtherr, "ResWidthErr/D")
        tree.Branch("CoGMean", cogmean, "CoGMean/D")
        tree.Branch("CoGMeanErr", cogmeanerr, "CoGMeanErr/D")
        tree.Branch("RunNumber", runnumber, "RunNumber/I")
        tree.Branch("ExpNumber", expnumber, "ExpNumber/I")
        tree.Branch("EvtT0Mean", evttime, "EvtT0Mean/D")
        tree.Branch("Jitter", jitter, "Jitter/D")
        tree.Branch("EvtT0ErrorMean", cdcErrorMean, "EvtT0ErrorMean/D")
        tree.Branch("ClsSizeMean", clsSize, "ClsSizeMean/D")
        tree.Branch("ClsChargeMean", clsCharge, "ClsChargeMean/D")
        tree.Branch("ClsSNRMean", clsSNR, "ClsSNRMean/D")

        # par = [0, 1]

        # tfileHist.cd()
        geoCache = Belle2.VXD.GeoCache.getInstance()
        gDirectory.mkdir("plots")
        gDirectory.cd("plots")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            layerNumberTree[0] = layerNumber
            li = layerNumber - 3
            gDirectory.mkdir("layer" + str(layerNumber))
            gDirectory.cd("layer" + str(layerNumber))
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                ladderNumberTree[0] = ladderNumber
                ldi = ladderNumber - 1
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    sensorNumberTree[0] = sensorNumber
                    si = sensorNumber - 1
                    for side in range(2):
                        sideTree[0] = side
                        # Resolution distribution Histograms with Gaussian Fit
                        res = self.resList[li][ldi][si][side]
                        res.GetXaxis().SetTitle("cluster time - EventT0 (ns)")
                        fitResult = int(TFitResultPtr(res.Fit(self.gaus, "R")))
                        res.Write()
                        # COG Distribution Histograms
                        cog = self.cogList[li][ldi][si][side]
                        cog.GetXaxis().SetTitle("cluster time (ns)")
                        cog.Write()
                        # CDC EventT0 Distribution Histograms
                        cdc = self.cdcList[li][ldi][si][side]
                        cdc.GetXaxis().SetTitle("EventT0 (ns)")
                        cdc.Write()
                        cdcError = self.cdcErrorList[li][ldi][si][side]
                        cdcError.GetXaxis().SetTitle("EventT0 Error (ns)")
                        cdcError.Write()
                        # SNR Distribution Histograms
                        snr = self.snrList[li][ldi][si][side]
                        clsSNR[0] = snr.GetMean()
                        snr.GetXaxis().SetTitle("cluster SNR")
                        snr.Write()
                        # Size Distribution Histograms
                        size = self.sizeList[li][ldi][si][side]
                        clsSize[0] = size.GetMean()
                        size.GetXaxis().SetTitle("cluster Size")
                        size.Write()
                        # Charge Distribution Histograms
                        charge = self.chargeList[li][ldi][si][side]
                        clsCharge[0] = charge.GetMean()
                        charge.GetXaxis().SetTitle("cluster Charge")
                        charge.Write()
                        # ScatterPlot Histograms with Linear Fit
                        sp = self.spList[li][ldi][si][side]
                        # covscalebias = sp.GetCovariance()
                        pfxsp = sp.ProfileX()
                        sp.GetXaxis().SetTitle("cluster time (ns)")
                        sp.GetYaxis().SetTitle("EventT0 (ns)")
                        sp.Write()
                        pfxsp.Write()

                        mean[0] = self.gaus.GetParameter(1)
                        meanerr[0] = self.gaus.GetParError(1)
                        width[0] = self.gaus.GetParameter(2)
                        widtherr[0] = self.gaus.GetParError(2)
                        cogmean[0] = cog.GetMean()
                        if cog.GetEntries() == 0:
                            cogmeanerr[0] = -1
                        else:
                            cogmeanerr[0] = cog.GetRMS()/math.sqrt(cog.GetEntries())
                        evttime[0] = cdc.GetMean()
                        if cdc.GetEntries() == 0:
                            jitter[0] = -1
                        else:
                            jitter[0] = cdc.GetRMS()
                        cdcErrorMean[0] = cdcError.GetMean()

                        if fitResult == -1:
                            mean[0] = -100.0
                            meanerr[0] = -100.0
                            width[0] = -100.0
                            widtherr[0] = -100.0

                        tree.Fill()

            gDirectory.cd("../")

        gDirectory.cd("../")
        self.EventT0Hist.Write()
        tree.Write()
        tfile.Purge()
        tfile.Close()
        del tfile
