##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
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


import basf2 as b2
from ROOT import Belle2, TFile, TH1F, TH2D
from ROOT import TF1, gDirectory, gROOT
# import numpy
from ROOT.Belle2 import SVDCoGCalibrationFunction


svd_recoDigits = "SVDRecoDigitsFromTracks"
cdc_Time0 = "EventT0"
svd_Clusters = "SVDClustersFromTracks"

gROOT.SetBatch(True)

# mode = True


class SVDCoGTimeCalibrationImporterModule(b2.Module):
    """
    Python class used for checking SVD CoG Calibration stored in a localDB,
    creating a localDB with the correction and a root file to check the corrections
    """

    def notApplyCorrectForCDCLatency(self, mode):
        """
        Function that allows to set if apply the CDC latency correction

        parameters:
             mode (bool):
             - if True -> not apply correction
             - if False -> apply correction
        """
        #: parameter that allows to apply or not the CDC latency correction
        self.notApplyCDCLatencyCorrection = mode
        print("Not Correct for CDC latency: " + str(mode) + " " + str(self.notApplyCDCLatencyCorrection))

    def fillLists(self, svdRecoDigits_rel_Clusters, svdClusters_rel_RecoTracks_cl):
        """
        Function that fill the lists needed for the CoG corrections

        parameters:
             svdRecoDigits_rel_Clusters (SVDRecoDigit): reco digits related to clusters
             svdClusters_rel_RecoTracks_cl (SVDCluster): cluster related to tracks
        """

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

        hasTimezero = self.cdcEventT0.hasEventT0()
        # print("Time: " + str(hasTimezero))
        if hasTimezero:
            svdEventInfo = Belle2.PyStoreObj("SVDEventInfo")
            TBClusters = svdEventInfo.getModeByte().getTriggerBin()
            TBIndex = ord(TBClusters)
            tZero = self.cdcEventT0.getEventT0()
            # tZero_err = self.cdcEventT0.getEventT0Uncertainty()
            # tZero_err = 5.1
            tZeroSync = tZero - 4000./509 * (3 - TBIndex)
            et0 = self.EventT0Hist
            et0.Fill(tZeroSync)
            # print(str(tZero_err))

            # print(self.resList[sideIndex])
            resHist = self.resList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            resHist.Fill(timeCluster - tZeroSync)
            spHist = self.spList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            # spHist.Fill(timeCluster, 1.3*timeCluster - 50 + random.gauss(0,10))
            spHist.Fill(timeCluster, tZeroSync)
            cogHist = self.cogList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cogHist.Fill(timeCluster)
            cdcHist = self.cdcList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cdcHist.Fill(tZeroSync)
            snrHist = self.snrList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            snrHist.Fill(snrCluster)

            self.nList[layerIndex][ladderIndex][sensorIndex][sideIndex] += 1

            self.sumCOGList[layerIndex][ladderIndex][sensorIndex][sideIndex] += timeCluster
            self.sumCOGList2[layerIndex][ladderIndex][sensorIndex][sideIndex] += timeCluster * timeCluster
            self.sumCOGList3[layerIndex][ladderIndex][sensorIndex][sideIndex] += timeCluster * timeCluster * timeCluster
            self.sumCOGList4[layerIndex][ladderIndex][sensorIndex][sideIndex] += timeCluster * \
                timeCluster * timeCluster * timeCluster
            self.sumCOGList5[layerIndex][ladderIndex][sensorIndex][sideIndex] += timeCluster * \
                timeCluster * timeCluster * timeCluster * timeCluster
            self.sumCOGList6[layerIndex][ladderIndex][sensorIndex][sideIndex] += timeCluster * \
                timeCluster * timeCluster * timeCluster * timeCluster * timeCluster

            self.sumCDCList[layerIndex][ladderIndex][sensorIndex][sideIndex] += tZeroSync
            self.sumCDCCOGList[layerIndex][ladderIndex][sensorIndex][sideIndex] += tZeroSync * timeCluster
            self.sumCDCCOGList2[layerIndex][ladderIndex][sensorIndex][sideIndex] += tZeroSync * timeCluster * timeCluster
            self.sumCDCCOGList3[layerIndex][ladderIndex][sensorIndex][sideIndex] += tZeroSync * \
                timeCluster * timeCluster * timeCluster

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

    def initialize(self):
        """
        Initialize object (histograms, lists, ...) used by the class
        """

        #: name of the output file
        self.outputFileName = "CoGCorrectionMonitor_" + self.localdb + ".root"

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
        #: number of clusters
        self.nList = []
        #: sum of CoG times
        self.sumCOGList = []
        #: sum of CoG times squared
        self.sumCOGList2 = []
        #: sum of CoG times to the third
        self.sumCOGList3 = []
        #: sum of CoG times to the fourth
        self.sumCOGList4 = []
        #: sum of CoG times to the fifth
        self.sumCOGList5 = []
        #: sum of CoG times to the sixth
        self.sumCOGList6 = []
        #: sum of t0 times
        self.sumCDCList = []
        #: sum of t0*cog
        self.sumCDCCOGList = []
        #: sum of t0*cog squared
        self.sumCDCCOGList2 = []
        #: sum of t0 times to the third
        self.sumCDCCOGList3 = []

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

            layerList9 = []
            layerList10 = []
            layerList11 = []
            layerList12 = []
            layerList13 = []
            layerList14 = []
            layerList15 = []
            layerList16 = []
            layerList17 = []

            self.resList.append(layerList0)
            self.spList.append(layerList1)
            self.cogList.append(layerList2)
            self.cdcList.append(layerList3)
            self.snrList.append(layerList4)

            self.nList.append(layerList8)
            self.sumCOGList.append(layerList7)
            self.sumCOGList2.append(layerList9)
            self.sumCOGList3.append(layerList10)
            self.sumCOGList4.append(layerList11)
            self.sumCOGList5.append(layerList12)
            self.sumCOGList6.append(layerList13)

            self.sumCDCList.append(layerList14)
            self.sumCDCCOGList.append(layerList15)
            self.sumCDCCOGList2.append(layerList16)
            self.sumCDCCOGList3.append(layerList17)

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

                ladderList9 = []
                ladderList10 = []
                ladderList11 = []
                ladderList12 = []
                ladderList13 = []
                ladderList14 = []
                ladderList15 = []
                ladderList16 = []
                ladderList17 = []

                layerList0.append(ladderList0)
                layerList1.append(ladderList1)
                layerList2.append(ladderList2)
                layerList3.append(ladderList3)
                layerList4.append(ladderList4)
                layerList5.append(ladderList5)
                layerList6.append(ladderList6)
                layerList7.append(ladderList7)
                layerList8.append(ladderList8)

                layerList9.append(ladderList9)
                layerList10.append(ladderList10)
                layerList11.append(ladderList11)
                layerList12.append(ladderList12)
                layerList13.append(ladderList13)
                layerList14.append(ladderList14)
                layerList15.append(ladderList15)
                layerList16.append(ladderList16)
                layerList17.append(ladderList17)
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

                    sensorList9 = []
                    sensorList10 = []
                    sensorList11 = []
                    sensorList12 = []
                    sensorList13 = []
                    sensorList14 = []
                    sensorList15 = []
                    sensorList16 = []
                    sensorList17 = []

                    ladderList0.append(sensorList0)
                    ladderList1.append(sensorList1)
                    ladderList2.append(sensorList2)
                    ladderList3.append(sensorList3)
                    ladderList4.append(sensorList4)
                    ladderList5.append(sensorList5)
                    ladderList6.append(sensorList6)
                    ladderList7.append(sensorList7)
                    ladderList8.append(sensorList8)

                    ladderList9.append(sensorList9)
                    ladderList10.append(sensorList10)
                    ladderList11.append(sensorList11)
                    ladderList12.append(sensorList12)
                    ladderList13.append(sensorList13)
                    ladderList14.append(sensorList14)
                    ladderList15.append(sensorList15)
                    ladderList16.append(sensorList16)
                    ladderList17.append(sensorList17)
                    # sensorNumber = sensor.getSensorNumber()
                    '''
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
                     '''
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
                        self.resList[li][ldi][si].append(
                            TH1F("res" + "_" + str(k) + "." + str(s), " ", 200, -100, 100))
                        self.spList[li][ldi][si].append(
                            TH2D("sp" + "_" + str(k) + "." + str(s), " ", 300, -150, 150, 300, -150, 150))
                        self.cogList[li][ldi][si].append(
                            TH1F("cog" + "_" + str(k) + "." + str(s), " ", 200, -100, 100))
                        self.cdcList[li][ldi][si].append(
                            TH1F("cdc" + "_" + str(k) + "." + str(s), " ", 200, -100, 100))
                        self.snrList[li][ldi][si].append(
                            TH1F("snr" + "_" + str(k) + "." + str(s), " ", 100, 0, 100))
                        self.nList[li][ldi][si].append(0)
                        self.sumCOGList[li][ldi][si].append(0)
                        self.sumCOGList2[li][ldi][si].append(0)
                        self.sumCOGList3[li][ldi][si].append(0)
                        self.sumCOGList4[li][ldi][si].append(0)
                        self.sumCOGList5[li][ldi][si].append(0)
                        self.sumCOGList6[li][ldi][si].append(0)
                        self.sumCDCList[li][ldi][si].append(0)
                        self.sumCDCCOGList[li][ldi][si].append(0)
                        self.sumCDCCOGList2[li][ldi][si].append(0)
                        self.sumCDCCOGList3[li][ldi][si].append(0)

        #: distribution of EventT0
        self.EventT0Hist = TH1F("EventT0", " ", 200, -100, 100)
        #: gaus function used for fitting distributions
        self.gaus = TF1("gaus", 'gaus(0)', -150, 100)
        #: third order polynomial function used for fitting distributions
        self.pol = TF1("pol", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -150, 150)

        self.NTOT = 0

    def event(self):
        """
        Function that allows to cicle on the events
        """
        # timeClusterU = 0
        # timeClusterV = 0
        # sideIndex = 0
        # TBIndexU = 0
        # TBIndexV = 0
        #: counts the number of events
        self.Evt = self.Evt + 1

        #: registers PyStoreObj EventT0
        self.cdcEventT0 = Belle2.PyStoreObj(cdc_Time0)
        svdCluster_list = Belle2.PyStoreArray(svd_Clusters)
        # svdRecoDigit_list = Belle2.PyStoreArray(svd_recoDigits)

        for svdCluster in svdCluster_list:
            svdRecoDigit = svdCluster.getRelatedTo(svd_recoDigits)
            self.fillLists(svdRecoDigit, svdCluster)

    def terminate(self):
        """
        Terminates te class and produces the output rootfile
        """

        tfile = TFile(self.outputFileName, 'recreate')
        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDCoGTimeCalibrations.t_payload()

        timeCal = SVDCoGCalibrationFunction()
        par = [0, 1, 0, 0]
        # TCOGMEAN = 0
        T0MEAN = 0

        '''
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
                            n = self.nList[li][ldi][si][side][tb]
                            NTOT += n
        '''
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
                        # Resolution distribution Histograms with Gaussian Fit
                        res = self.resList[li][ldi][si][side]
                        # fitResult = int(TFitResultPtr(res.Fit(self.gaus, "R")))

                        res.Write()
                        # COG Distribution Histograms
                        cog = self.cogList[li][ldi][si][side]
                        cog.Write()
                        # CDC EventT0 Distribution Histograms
                        cdc = self.cdcList[li][ldi][si][side]
                        cdc.Write()
                        # SNR Distribution Histograms
                        snr = self.snrList[li][ldi][si][side]
                        # snrMean = snr.GetMean()
                        snr.Write()
                        # ScatterPlot Histograms with Linear Fit
                        sp = self.spList[li][ldi][si][side]
                        # covscalebias = sp.GetCovariance()
                        pfxsp = sp.ProfileX()
                        self.pol.SetParameters(-50, 1.5, 0.001, 0.00001)
                        pfxsp.Fit(self.pol, "R")
                        par[0] = self.pol.GetParameter(0)
                        par[1] = self.pol.GetParameter(1)
                        par[2] = self.pol.GetParameter(2)
                        par[3] = self.pol.GetParameter(3)
                        sp.Write()
                        pfxsp.Write()

                        # T0MEAN = self.EventT0Hist.GetMean()
                        T0MEAN = cdc.GetMean()
                        '''
                        print(
                            "Mean of the CoG corrected distribution: " +
                            str(TCOGMEAN) +
                            " Mean of the T0 distribution: " +
                            str(T0MEAN))
                        '''
                        timeCal.set_current(1)
                        timeCal.set_pol3parameters(par[0] - T0MEAN, par[1], par[2], par[3])
                        print("setting CoG calibration for " + str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, timeCal)
            gDirectory.cd("../")

        gDirectory.cd("../")
        self.EventT0Hist.Write()

        Belle2.Database.Instance().storeData(Belle2.SVDCoGTimeCalibrations.name, payload, iov)

        tfile.Close()
