# !/usr/bin/env python
# -*- coding: utf-8 -*-

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


import basf2 as b2
from ROOT import Belle2, TFile, TFitResultPtr, TH1F, TH2D, TH2F
from ROOT import TF1, gDirectory, gROOT
import math
from ROOT.Belle2 import SVDCoGCalibrationFunction


svd_recoDigits = "SVDRecoDigitsFromTracks"
cdc_Time0 = "EventT0"
svd_Clusters = "SVDClustersFromTracks"

gROOT.SetBatch(True)

# mode = True


class SVDCoGTimeCalibrationImporterModule(b2.Module):
    """
    Python class used for evaluating the CoG corrections, create a localDB,
    creating a localDB with the corrections and a root file to check the corrections
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

    def fillLists(self, mode_byte_object, svdClusters_rel_RecoTracks_cl):
        """
        Function that fill the lists needed for the CoG corrections

        parameters:
             mode_byte_object (modeByte): modeByte that contains the information about the TB
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
            TBClusters = mode_byte_object.getTriggerBin()
            TBIndex = ord(TBClusters)
            tZero = self.cdcEventT0.getEventT0()
            # tZero_err = self.cdcEventT0.getEventT0Uncertainty()
            # tZero_err = 5.1
            tZeroSync = tZero - 4000./509 * (3 - TBIndex)
            et0 = self.EventT0Hist
            et0.Fill(tZeroSync)
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

            self.nList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] += 1
            self.sumCOGList[layerIndex][ladderIndex][sensorIndex][sideIndex][TBIndex] += timeCluster

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
            self.sumCOGList.append(layerList7)
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
                            self.sumCOGList[li][ldi][si][s].append(0)
        #: distribution of EventT0
        self.EventT0Hist = TH1F("EventT0", " ", 200, -100, 100)
        #: alpha parameter vs TB, for U side
        self.AlphaUTB = TH2F("alphaVsTB_U", " ", 400, 0.5, 2, 4, 0, 4)
        self.AlphaUTB.GetXaxis().SetTitle("alpha")
        self.AlphaUTB.GetYaxis().SetTitle("trigger bin")
        #: alpha parameter vs TB, for V side
        self.AlphaVTB = TH2F("alphaVsTB_V", " ", 400, 0.5, 2, 4, 0, 4)
        self.AlphaVTB.GetXaxis().SetTitle("alpha")
        self.AlphaVTB.GetYaxis().SetTitle("trigger bin")
        #: beta parameter vs TB, for U side
        self.BetaUTB = TH2F("betaVsTB_U", " ", 200, -100, 100, 4, 0, 4)
        self.BetaUTB.GetXaxis().SetTitle("beta (ns)")
        self.BetaUTB.GetYaxis().SetTitle("trigger bin")
        #: beta parameter vs TB, for V side
        self.BetaVTB = TH2F("betaVsTB_V", " ", 200, -100, 100, 4, 0, 4)
        self.BetaVTB.GetXaxis().SetTitle("beta (ns)")
        self.BetaVTB.GetYaxis().SetTitle("trigger bin")

        #: mean of the residuals distribution vs TB, for V side
        self.MeanHistVTB = TH2F("meanHistVsTB_V", " ", 100, -10, 10, 4, 0, 4)
        self.MeanHistVTB.GetXaxis().SetTitle("distribution mean (ns)")
        self.MeanHistVTB.GetYaxis().SetTitle("trigger bin")
        #: mean of the residuals distribution vs TB, for U side
        self.MeanHistUTB = TH2F("meanHistVsTB_U", " ", 100, -10, 10, 4, 0, 4)
        self.MeanHistUTB.GetXaxis().SetTitle("distribution mean (ns)")
        self.MeanHistUTB.GetYaxis().SetTitle("trigger bin")
        #: RMS of the residuals distribution vs TB, for V side
        self.RMSHistVTB = TH2F("rmsHistVsTB_V", " ", 100, 0, 10, 4, 0, 4)
        self.RMSHistVTB.GetXaxis().SetTitle("distribution RMS (ns)")
        self.RMSHistVTB.GetYaxis().SetTitle("trigger bin")
        #: RMS of the residuals distribution vs TB, for U side
        self.RMSHistUTB = TH2F("rmsHistVsTB_U", " ", 100, 0, 10, 4, 0, 4)
        self.RMSHistUTB.GetXaxis().SetTitle("distribution RMS (ns)")
        self.RMSHistUTB.GetYaxis().SetTitle("trigger bin")
        #: mean of the residuals distribution vs TB, for V side (from gaussian fit)
        self.MeanFitVTB = TH2F("meanFitVsTB_V", " ", 100, -10, 10, 4, 0, 4)
        self.MeanFitVTB.GetXaxis().SetTitle("fit mean (ns)")
        self.MeanFitVTB.GetYaxis().SetTitle("trigger bin")
        #: mean of the residuals distribution vs TB, for U side (from gaussian fit)
        self.MeanFitUTB = TH2F("meanFitVsTB_U", " ", 100, -10, 10, 4, 0, 4)
        self.MeanFitUTB.GetXaxis().SetTitle("fit mean (ns)")
        self.MeanFitUTB.GetYaxis().SetTitle("trigger bin")
        #: RMS of the residuals distribution vs TB, for U side (from gaussian fit)
        self.RMSFitUTB = TH2F("rmsFitVsTB_U", " ", 100, 0, 10, 4, 0, 4)
        self.RMSFitUTB.GetXaxis().SetTitle("fit sigma (ns)")
        self.RMSFitUTB.GetYaxis().SetTitle("trigger bin")
        #: RMS of the residuals distribution vs TB, for V side (from gaussian fit)
        self.RMSFitVTB = TH2F("rmsFitVsTB_V", " ", 100, 0, 10, 4, 0, 4)
        self.RMSFitVTB.GetXaxis().SetTitle("fit sigma (ns)")
        self.RMSFitVTB.GetYaxis().SetTitle("trigger bin")

        #: gaus function used for fitting distributions
        self.gaus = TF1("gaus", 'gaus(0)', -150, 100)

        self.NTOT = 0

    def event(self):
        """
        Function that allows to cicle on the events
        """
        svd_evt_info = Belle2.PyStoreObj('SVDEventInfo')
        mode_byte = svd_evt_info.getModeByte()
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
            # svdRecoDigit = svdCluster.getRelatedTo(svd_recoDigits)
            self.fillLists(mode_byte, svdCluster)

    def terminate(self):
        """
        Terminates te class and produces the output rootfile
        """

        tfile = TFile(self.outputFileName, 'recreate')
        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDCoGTimeCalibrations.t_payload()

        timeCal = SVDCoGCalibrationFunction()
        # Bias and Scale
        tbBias = [-50, -50, -50, -50]
        tbScale = [1, 1, 1, 1]
        tbBias_err = [1, 1, 1, 1]
        tbScale_err = [1, 1, 1, 1]
        # tbCovScaleBias = [1, 1, 1, 1]

        TCOGMEAN = 0
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
                            # snrMean = snr.GetMean()
                            snr.Write()
                            # ScatterPlot Histograms with Linear Fit
                            sp = self.spList[li][ldi][si][side][tb]
                            # covscalebias = sp.GetCovariance()
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
                            TCOGMEAN += n * (m * self.sumCOGList[li][ldi][si][side][tb] / n + q) / self.NTOT

                        T0MEAN = self.EventT0Hist.GetMean()
                        '''
                        print(
                            "Mean of the CoG corrected distribution: " +
                            str(TCOGMEAN) +
                            " Mean of the T0 distribution: " +
                            str(T0MEAN))
                        '''
                        if not self.notApplyCDCLatencyCorrection:
                            tbBias[0] = tbBias[0] - T0MEAN
                            tbBias[1] = tbBias[1] - T0MEAN
                            tbBias[2] = tbBias[2] - T0MEAN
                            tbBias[3] = tbBias[3] - T0MEAN

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
        self.EventT0Hist.Write()

        Belle2.Database.Instance().storeData(Belle2.SVDCoGTimeCalibrations.name, payload, iov)

        tfile.Close()
