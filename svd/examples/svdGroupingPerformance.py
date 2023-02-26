#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##################################################################################
#
# Example Script to use SVDEventT0PerformanceHisto on simulation or real data
#
# Use: basf2 -i <RAWDATAFILE> script -- --fileDir temp/ --fileTag test [...]
#
###################################################################################

import basf2 as b2
from basf2 import conditions as b2conditions
from svd.executionTime_utils import SVDExtraEventStatisticsModule
import rawdata as raw
import tracking as trk
import simulation as sim
import argparse
from background import get_background_files
import generators as ge

from ROOT import Belle2, TH1F, TH2F, TFile, TMath
import math


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--seed", default="1",
                    help="seed")
parser.add_argument("--fileDir", default="./",
                    help="Output File Dir")
parser.add_argument("--fileTag", default="test",
                    help="Output File Tag")
parser.add_argument("--moduleName", default="SVDGroupingPerformance",
                    help="Module Prefix")
parser.add_argument("--isMC", action="store_true",
                    help="Use Simulation")
parser.add_argument("--isRawTime", action="store_true",
                    help="Raw SVD Time")
parser.add_argument("--doSVDGrouping", action="store_true",
                    help="Assign Groups to SVD Clusters")
parser.add_argument("--exponentialSort", type=float, default=30,
                    help="Select only one group")
parser.add_argument("--signalGroupSelection", type=int, default=20,
                    help="Select only few groups")
parser.add_argument("--formSuperGroup", action="store_true",
                    help="form super group using selected")
parser.add_argument("--useSVDGroupInfo", action="store_true",
                    help="Use SVD Grouping info in SVDSpacePointCreator")
parser.add_argument("--CoG3TimeCalibration_bucket36", action="store_true",
                    help="SVD Time calibration")
parser.add_argument("--CoG3TimeCalibration_bucket32", action="store_true",
                    help="SVD Time calibration")
parser.add_argument("--is3sample", action="store_true",
                    help="Emulate SVD 3 samples")
parser.add_argument("--executionTime", action="store_true",
                    help="Store exection time tree")
parser.add_argument("--test", action="store_true",
                    help="Test with small numbers of events")
parser.add_argument("--OffOff", action="store_true",
                    help="OffOff SVD Cluster Selection")
parser.add_argument("--OnOn", action="store_true",
                    help="OnOn SVD Cluster Selection")
parser.add_argument("--OffOn", action="store_true",
                    help="OffOn SVD Cluster Selection")
parser.add_argument("--OnOff", action="store_true",
                    help="OnOff SVD Cluster Selection")
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")

if args.test:
    b2.set_log_level(b2.LogLevel.DEBUG)
    b2.set_debug_level(1)


outputFileTag = ""
if args.isMC:
    outputFileTag += "_MC"
if args.is3sample:
    outputFileTag += "_emulated3sample"
if args.isRawTime:
    outputFileTag += "_RawTime"
if args.doSVDGrouping:
    outputFileTag += "_Grouping"
    outputFileTag += "_expoSort" + str(int(args.exponentialSort))
    outputFileTag += "_Signal" + str(int(args.signalGroupSelection))
    if args.formSuperGroup:
        outputFileTag += "_SuperGroup"
    if args.useSVDGroupInfo:
        outputFileTag += "_UsedInSPs"
if args.OffOff:
    outputFileTag += "_OffOff"
if args.OnOn:
    outputFileTag += "_OnOn"
if args.OffOn:
    outputFileTag += "_OffOn"
if args.OnOff:
    outputFileTag += "_OnOff"
outputFileTag += "_" + str(args.fileTag)


minTime = -160.
maxTime = 160.
sigLoc = 0
sigMin = -50
sigMax = 50
if args.isRawTime:
    minTime = -20.
    maxTime = 200.
    sigLoc = 100
    sigMin = 70
    sigMax = 130
if args.is3sample:
    sigMin = -50
    sigMax = 50


class SVDGroupingPerformance(b2.Module):
    ''' check cluster grouping'''

    def initialize(self):
        '''define histograms'''

        usedBin1 = 100
        usedBin2 = 2000
        eclMinVal = 19.5
        eclMaxVal = 1100.5

        timeBin = int(maxTime - minTime)

        #: \cond
        self.TH1F_Index = {}
        self.TH1F_Store = []
        TH1F_Store_Count = 0

        self.TH2F_Index = {}
        self.TH2F_Store = []
        TH2F_Store_Count = 0

        self.reso_TH2D_Histogram_Index = {}
        self.reso_TH2D_Histograms = []
        reso_TH2D_Histograms_Count = 0

        self.outputFileName = str(args.fileDir) + str(args.moduleName) + outputFileTag
        self.f = TFile(self.outputFileName + ".root", "RECREATE")
        #: \endcond

        # EventT0s

        self.TH1F_Store.append(TH1F("th1f_svdEventT0", "SVD EventT0", 500, -50., 50.))
        self.TH1F_Store[-1].GetXaxis().SetTitle("SVD EventT0 (ns)")
        self.TH1F_Index["th1f_svdEventT0"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_svdEventT0_State", "SVD EventT0 Availability", 2, -0.5, 1.5))
        self.TH1F_Store[-1].GetXaxis().SetBinLabel(1, "Not Available")
        self.TH1F_Store[-1].GetXaxis().SetBinLabel(2, "Available")
        self.TH1F_Index["th1f_svdEventT0_State"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        self.TH1F_Store.append(TH1F("th1f_cdcEventT0", "CDC EventT0", 500, -50., 50.))
        self.TH1F_Store[-1].GetXaxis().SetTitle("CDC EventT0 (ns)")
        self.TH1F_Index["th1f_cdcEventT0"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_cdcEventT0_State", "CDC EventT0 Availability", 2, -0.5, 1.5))
        self.TH1F_Store[-1].GetXaxis().SetBinLabel(1, "Not Available")
        self.TH1F_Store[-1].GetXaxis().SetBinLabel(2, "Available")
        self.TH1F_Index["th1f_cdcEventT0_State"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        # EventT0 - TOP Online EventT0

        self.TH1F_Store.append(TH1F("th1f_svdEventT0topRef", "SVD EventT0 - TOP Online EventT0", 500, -20., 20.))
        self.TH1F_Store[-1].GetXaxis().SetTitle("SVD EventT0 - TOP Online EventT0 (ns)")
        self.TH1F_Index["th1f_svdEventT0topRef"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        # Cluster Time

        self.TH1F_Store.append(TH1F("th1f_clsTime_PreTracking", "(PreTracking) Cluster Time", timeBin, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH1F_Store[-1].SetLineColor(1)
        self.TH1F_Index["th1f_clsTime_PreTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_bkgClsTime_PreTracking", "(PreTracking) Bkg Cluster Time", timeBin, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH1F_Store[-1].SetLineColor(2)
        self.TH1F_Index["th1f_bkgClsTime_PreTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_sigClsTime_PreTracking", "(PreTracking) Sig Cluster Time", timeBin, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH1F_Store[-1].SetLineColor(3)
        self.TH1F_Index["th1f_sigClsTime_PreTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        self.TH1F_Store.append(TH1F("th1f_clsTime_PostTracking", "(PostTracking) Cluster Time", timeBin, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH1F_Store[-1].SetLineColor(1)
        self.TH1F_Index["th1f_clsTime_PostTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_bkgClsTime_PostTracking", "(PostTracking) Bkg Cluster Time", timeBin, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH1F_Store[-1].SetLineColor(2)
        self.TH1F_Index["th1f_bkgClsTime_PostTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_sigClsTime_PostTracking", "(PostTracking) Sig Cluster Time", timeBin, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH1F_Store[-1].SetLineColor(3)
        self.TH1F_Index["th1f_sigClsTime_PostTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        self.TH1F_Store.append(TH1F("th1f_trackTime", "Track Time", timeBin * 10, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Track Time (ns)")
        self.TH1F_Index["th1f_trackTime"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_trackTimeCDCRef", "Track Time - CDC EventT0", 10000, -50, 50))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Track Time - CDC EventT0 (ns)")
        self.TH1F_Index["th1f_trackTimeCDCRef"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        # Cluster Time - TOP Online EventT0

        self.TH1F_Store.append(TH1F("th1f_clsTimeCdcRef_PreTracking", "(PreTracking) Cluster Time - CDC EventT0",
                                    timeBin * 2, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time - CDC EventT0 (ns)")
        self.TH1F_Store[-1].SetLineColor(1)
        self.TH1F_Index["th1f_clsTimeCdcRef_PreTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        self.TH1F_Store.append(TH1F("th1f_clsTimeCdcRef_PostTracking", "(PostTracking) Cluster Time - CDC EventT0",
                                    timeBin * 2, minTime, maxTime))
        self.TH1F_Store[-1].GetXaxis().SetTitle("Cluster Time - CDC EventT0 (ns)")
        self.TH1F_Store[-1].SetLineColor(1)
        self.TH1F_Index["th1f_clsTimeCdcRef_PostTracking"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        # ECL TC Emax

        self.TH1F_Store.append(TH1F("th1f_eclTCEmax", "ECL E_{max}^{TC}", 1250, 0.5, 1250.5))
        self.TH1F_Store[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.TH1F_Index["th1f_eclTCEmax"] = TH1F_Store_Count
        TH1F_Store_Count += 1
        self.TH1F_Store.append(TH1F("th1f_eclTCEmax_State", "ECL E_{max}^{TC} Availability", 2, -0.5, 1.5))
        self.TH1F_Store[-1].GetXaxis().SetBinLabel(1, "Not Available")
        self.TH1F_Store[-1].GetXaxis().SetBinLabel(2, "Available")
        self.TH1F_Index["th1f_eclTCEmax_State"] = TH1F_Store_Count
        TH1F_Store_Count += 1

        self.TH2F_Store.append(TH2F("th2f_timeGr_vs_grAmp", "Time Group vs Group Amplitude", 60, 0., 300, 21, -1.5, 19.5))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Group Amplitude")
        self.TH2F_Store[-1].GetYaxis().SetTitle("Time Group")
        self.TH2F_Index["th2f_timeGr_vs_grAmp"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_timeGr_vs_grCenter", "Time Group vs Group Center", 80, minTime, maxTime, 21, -1.5, 19.5))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Group Center (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("Time Group")
        self.TH2F_Index["th2f_timeGr_vs_grCenter"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_timeGr_vs_grWidth", "Time Group vs Group Width", 50, 1, 11, 21, -1.5, 19.5))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Group Width (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("Time Group")
        self.TH2F_Index["th2f_timeGr_vs_grWidth"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_timeGr_vs_clsTime", "Time Group vs Cluster Time",
                               timeBin, minTime, maxTime, 21, -1.5, 19.5))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("Time Group")
        self.TH2F_Index["th2f_timeGr_vs_clsTime"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_timeGr_vs_clsTime_PostTracking", "(PostTracking) Time Group vs Cluster Time",
                                    timeBin, minTime, maxTime, 21, -1.5, 19.5))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Cluster Time (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("Time Group")
        self.TH2F_Index["th2f_timeGr_vs_clsTime_PostTracking"] = TH2F_Store_Count
        TH2F_Store_Count += 1

        for i in range(4):
            self.TH2F_Store.append(TH2F("th2f_sig" + str(i) + "GrMean_vs_cdcEventT0", "Center of 'Group " +
                                        str(i) + "' vs CDC EventT0", 500, -50., 50., 500, -50., 50.))
            self.TH2F_Store[-1].GetXaxis().SetTitle("Center of 'Group " + str(i) + "' (ns)")
            self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
            self.TH2F_Index["th2f_sig" + str(i) + "GrMean_vs_cdcEventT0"] = TH2F_Store_Count
            TH2F_Store_Count += 1
            self.TH2F_Store.append(TH2F("th2f_sig" + str(i) + "GrMean_vs_cdcEventT0_rot45pos",
                                        "Center of 'Group " + str(i) + "' vs CDC EventT0 (Rotated 45)",
                                        500, -50., 50., 500, -50., 50.))
            self.TH2F_Store[-1].GetXaxis().SetTitle("Center of 'Group " + str(i) + "' (ns)")
            self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
            self.TH2F_Index["th2f_sig" + str(i) + "GrMean_vs_cdcEventT0_rot45pos"] = TH2F_Store_Count
            TH2F_Store_Count += 1
            self.TH2F_Store.append(TH2F("th2f_sig" + str(i) + "GrMean_vs_cdcEventT0_rot45neg",
                                        "Center of 'Group " + str(i) + "' vs CDC EventT0 (Rotated -45)",
                                        500, -50., 50., 500, -50., 50.))
            self.TH2F_Store[-1].GetXaxis().SetTitle("Center of 'Group " + str(i) + "' (ns)")
            self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
            self.TH2F_Index["th2f_sig" + str(i) + "GrMean_vs_cdcEventT0_rot45neg"] = TH2F_Store_Count
            TH2F_Store_Count += 1

        self.TH2F_Store.append(TH2F("th2f_aveClsTimeInTrk_vs_cdcEventT0",
                                    "Track Time vs CDC EventT0", 500, -50., 50., 500, -50., 50.))
        self.TH2F_Store[-1].GetXaxis().SetTitle("<Cluster Time> in Track (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
        self.TH2F_Index["th2f_aveClsTimeInTrk_vs_cdcEventT0"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_svdEventT0_vs_cdcEventT0", "SVD EventT0 vs CDC EventT0",
                                    500, -50., 50., 500, -50., 50.))
        self.TH2F_Store[-1].GetXaxis().SetTitle("SVD EventT0 (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
        self.TH2F_Index["th2f_svdEventT0_vs_cdcEventT0"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_svdEventT0_vs_cdcEventT0_rot45pos",
                                    "SVD EventT0 vs CDC EventT0 (Rotated 45)", 500, -50., 50., 500, -50., 50.))
        self.TH2F_Store[-1].GetXaxis().SetTitle("SVD EventT0 (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
        self.TH2F_Index["th2f_svdEventT0_vs_cdcEventT0_rot45pos"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_svdEventT0_vs_cdcEventT0_rot45neg",
                                    "SVD EventT0 vs CDC EventT0 (Rotated -45)", 500, -50., 50., 500, -50., 50.))
        self.TH2F_Store[-1].GetXaxis().SetTitle("SVD EventT0 (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 (ns)")
        self.TH2F_Index["th2f_svdEventT0_vs_cdcEventT0_rot45neg"] = TH2F_Store_Count
        TH2F_Store_Count += 1

        self.TH2F_Store.append(TH2F("th2f_trkTime_vs_clsTime_inCDCEventT0", "Track Time vs Cluster Time",
                                    timeBin, minTime, maxTime, timeBin, minTime, maxTime))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Track Time - Cluster Time (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 - Track Time (ns)")
        self.TH2F_Index["th2f_trkTime_vs_clsTime_inCDCEventT0"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_trkTime_vs_clsTime_inSVDEventT0", "Track Time vs Cluster Time",
                                    timeBin, minTime, maxTime, timeBin, minTime, maxTime))
        self.TH2F_Store[-1].GetXaxis().SetTitle("Track Time - Cluster Time (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("SVD EventT0 - Track Time (ns)")
        self.TH2F_Index["th2f_trkTime_vs_clsTime_inSVDEventT0"] = TH2F_Store_Count
        TH2F_Store_Count += 1

        self.TH2F_Store.append(TH2F("th2f_aveClsTimeInTrk_vs_clsTime_inCDCEventT0", "<Cluster Time> in Track vs Cluster Time",
                                    timeBin, minTime, maxTime, timeBin, minTime, maxTime))
        self.TH2F_Store[-1].GetXaxis().SetTitle("<Cluster Time> in Track - Cluster Time (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("CDC EventT0 - <Cluster Time> in Track (ns)")
        self.TH2F_Index["th2f_aveClsTimeInTrk_vs_clsTime_inCDCEventT0"] = TH2F_Store_Count
        TH2F_Store_Count += 1
        self.TH2F_Store.append(TH2F("th2f_aveClsTimeInTrk_vs_clsTime_inSVDEventT0", "<Cluster Time> in Track vs Cluster Time",
                                    timeBin, minTime, maxTime, timeBin, minTime, maxTime))
        self.TH2F_Store[-1].GetXaxis().SetTitle("<Cluster Time> in Track - Cluster Time (ns)")
        self.TH2F_Store[-1].GetYaxis().SetTitle("SVD EventT0 - <Cluster Time> in Track (ns)")
        self.TH2F_Index["th2f_aveClsTimeInTrk_vs_clsTime_inSVDEventT0"] = TH2F_Store_Count
        TH2F_Store_Count += 1

        # Resolution w.r.t. ECL TC Emax

        self.reso_TH2D_Histograms.append(TH2F("reso_eclTCEmax_vs_clsTimeTopRef_PreTracking",
                                              "(PreTracking) ECL E_{max}^{TC} vs Cluster Time - TOP Online EventT0",
                                              usedBin1, eclMinVal, eclMaxVal, usedBin2, -50, 50.))
        self.reso_TH2D_Histograms[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.reso_TH2D_Histograms[-1].GetYaxis().SetTitle("Cluster Time - TOP Online EventT0 (ns)")
        self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_clsTimeTopRef_PreTracking"] = reso_TH2D_Histograms_Count
        reso_TH2D_Histograms_Count += 1
        self.reso_TH2D_Histograms.append(TH2F("reso_eclTCEmax_vs_clsTimeTopRef_PostTracking",
                                              "(PostTracking) ECL E_{max}^{TC} vs Cluster Time - TOP Online EventT0",
                                              usedBin1, eclMinVal, eclMaxVal, usedBin2, -50, 50.))
        self.reso_TH2D_Histograms[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.reso_TH2D_Histograms[-1].GetYaxis().SetTitle("Cluster Time - TOP Online EventT0 (ns)")
        self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_clsTimeTopRef_PostTracking"] = reso_TH2D_Histograms_Count
        reso_TH2D_Histograms_Count += 1

        self.reso_TH2D_Histograms.append(TH2F("reso_eclTCEmax_vs_svdEventT0TopRef",
                                              "ECL E_{max}^{TC} vs SVD EventT0 - TOP Online EventT0",
                                              usedBin1, eclMinVal, eclMaxVal, usedBin2, -50, 50.))
        self.reso_TH2D_Histograms[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.reso_TH2D_Histograms[-1].GetYaxis().SetTitle("SVD EventT0 - TOP Online EventT0 (ns)")
        self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_svdEventT0TopRef"] = reso_TH2D_Histograms_Count
        reso_TH2D_Histograms_Count += 1

        self.reso_TH2D_Histograms.append(TH2F("reso_eclTCEmax_vs_totalTracks",
                                              "ECL E_{max}^{TC} vs Total SVD Tracks",
                                              usedBin1, eclMinVal, eclMaxVal, 36, -0.5, 35.5))
        self.reso_TH2D_Histograms[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.reso_TH2D_Histograms[-1].GetYaxis().SetTitle("Total SVD Tracks")
        self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_totalTracks"] = reso_TH2D_Histograms_Count
        reso_TH2D_Histograms_Count += 1

        self.reso_TH2D_Histograms.append(TH2F("reso_eclTCEmax_vs_totalClusters_PreTracking",
                                              "(PreTracking) ECL E_{max}^{TC} vs Total SVD Clusters",
                                              usedBin1, eclMinVal, eclMaxVal, 1000, 0.5, 1000.5))
        self.reso_TH2D_Histograms[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.reso_TH2D_Histograms[-1].GetYaxis().SetTitle("Total SVD Clusters")
        self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_totalClusters_PreTracking"] = reso_TH2D_Histograms_Count
        reso_TH2D_Histograms_Count += 1
        self.reso_TH2D_Histograms.append(TH2F("reso_eclTCEmax_vs_totalClusters_PostTracking",
                                              "(PostTracking) ECL E_{max}^{TC} vs Total SVD Clusters",
                                              usedBin1, eclMinVal, eclMaxVal, 1000, 0.5, 1000.5))
        self.reso_TH2D_Histograms[-1].GetXaxis().SetTitle("ECL E_{max}^{TC} (ADC)")
        self.reso_TH2D_Histograms[-1].GetYaxis().SetTitle("Total SVD Clusters")
        self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_totalClusters_PostTracking"] = reso_TH2D_Histograms_Count
        reso_TH2D_Histograms_Count += 1

    def event(self):
        ''' Event loop'''

        EventT0 = Belle2.PyStoreObj('EventT0')
        svdEventT0 = math.nan
        cdcEventT0 = math.nan
        svd = Belle2.Const.DetectorSet(Belle2.Const.SVD)
        cdc = Belle2.Const.DetectorSet(Belle2.Const.CDC)
        top = Belle2.Const.DetectorSet(Belle2.Const.TOP)
        if EventT0.hasTemporaryEventT0(svd):
            tmp = EventT0.getTemporaryEventT0s(Belle2.Const.SVD)
            svdEventT0 = tmp.back().eventT0
        if EventT0.hasTemporaryEventT0(cdc):
            tmp = EventT0.getTemporaryEventT0s(Belle2.Const.CDC)
            cdcEventT0 = tmp.back().eventT0

        if not math.isnan(svdEventT0):
            self.TH1F_Store[self.TH1F_Index["th1f_svdEventT0"]].Fill(svdEventT0)
            self.TH1F_Store[self.TH1F_Index["th1f_svdEventT0_State"]].Fill(1)
        else:
            self.TH1F_Store[self.TH1F_Index["th1f_svdEventT0_State"]].Fill(0)
        if not math.isnan(cdcEventT0):
            self.TH1F_Store[self.TH1F_Index["th1f_cdcEventT0"]].Fill(cdcEventT0)
            self.TH1F_Store[self.TH1F_Index["th1f_cdcEventT0_State"]].Fill(1)
        else:
            self.TH1F_Store[self.TH1F_Index["th1f_cdcEventT0_State"]].Fill(0)

        if not math.isnan(svdEventT0) and not math.isnan(cdcEventT0):
            self.TH2F_Store[self.TH2F_Index["th2f_svdEventT0_vs_cdcEventT0"]].Fill(svdEventT0, cdcEventT0)
            self.TH2F_Store[self.TH2F_Index["th2f_svdEventT0_vs_cdcEventT0_rot45pos"]].Fill(
                svdEventT0 * TMath.Cos(TMath.PiOver4()) + cdcEventT0 * TMath.Sin(TMath.PiOver4()),
                - svdEventT0 * TMath.Sin(TMath.PiOver4()) + cdcEventT0 * TMath.Cos(TMath.PiOver4()))
            self.TH2F_Store[self.TH2F_Index["th2f_svdEventT0_vs_cdcEventT0_rot45neg"]].Fill(
                svdEventT0 * TMath.Cos(-TMath.PiOver4()) + cdcEventT0 * TMath.Sin(-TMath.PiOver4()),
                - svdEventT0 * TMath.Sin(-TMath.PiOver4()) + cdcEventT0 * TMath.Cos(-TMath.PiOver4()))

        OnlineEventT0 = []
        if not args.isMC:
            OnlineEventT0 = Belle2.PyStoreArray('OnlineEventT0s')
        topOnlineEventT0 = math.nan
        for evt in OnlineEventT0:
            if evt.getOnlineEventT0Detector() == top:
                topOnlineEventT0 = evt.getOnlineEventT0()

        if not math.isnan(topOnlineEventT0) and not math.isnan(svdEventT0):
            self.TH1F_Store[self.TH1F_Index["th1f_svdEventT0topRef"]].Fill(svdEventT0 - topOnlineEventT0)

        TRGECLData = []
        if not args.isMC:
            TRGECLData = Belle2.PyStoreArray('TRGECLUnpackerStores')
        eclTCEmax = 0
        for trgHit in TRGECLData:
            hitWin = trgHit.getHitWin()
            if hitWin != 3 and hitWin != 4:
                continue
            if trgHit.getTCEnergy() > eclTCEmax:
                eclTCEmax = trgHit.getTCEnergy()

        if eclTCEmax:
            self.TH1F_Store[self.TH1F_Index["th1f_eclTCEmax"]].Fill(eclTCEmax)
            self.TH1F_Store[self.TH1F_Index["th1f_eclTCEmax_State"]].Fill(1)
            if not math.isnan(topOnlineEventT0) and not math.isnan(svdEventT0):
                self.reso_TH2D_Histograms[self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_svdEventT0TopRef"]].Fill(
                    eclTCEmax, svdEventT0 - topOnlineEventT0)
        else:
            self.TH1F_Store[self.TH1F_Index["th1f_eclTCEmax_State"]].Fill(0)

        # Pre Tracking

        clusterList = Belle2.PyStoreArray("SVDClusters")
        if eclTCEmax:
            self.reso_TH2D_Histograms[self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_totalClusters_PreTracking"]].Fill(
                eclTCEmax, len(clusterList))
        fillOnce = 0
        for d in clusterList:
            # print(type(d))
            clsTime = d.getClsTime()

            self.TH1F_Store[self.TH1F_Index["th1f_clsTime_PreTracking"]].Fill(clsTime)
            if not math.isnan(cdcEventT0):
                self.TH1F_Store[self.TH1F_Index["th1f_clsTimeCdcRef_PreTracking"]].Fill(clsTime - cdcEventT0)
            if not math.isnan(topOnlineEventT0) and eclTCEmax:
                self.reso_TH2D_Histograms[self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_clsTimeTopRef_PreTracking"]].Fill(
                    eclTCEmax, clsTime - topOnlineEventT0)

            groupIds = d.getTimeGroupId()
            groupInfo = d.getTimeGroupInfo()
            # print(len(groupIds), len(groupInfo))
            minId = 1000
            par0 = -1000
            par1 = -1000
            par2 = -1000
            for id in range(len(groupIds)):
                gr = groupIds[id]
                if gr < minId:
                    minId = gr
                    if len(groupInfo) > 0:
                        par0, par1, par2 = groupInfo[id]

            if len(groupIds) == 0:
                minId = -1

            self.TH2F_Store[self.TH2F_Index["th2f_timeGr_vs_clsTime"]].Fill(clsTime, minId)
            self.TH2F_Store[self.TH2F_Index["th2f_timeGr_vs_grAmp"]].Fill(par0, minId)
            self.TH2F_Store[self.TH2F_Index["th2f_timeGr_vs_grCenter"]].Fill(par1, minId)
            self.TH2F_Store[self.TH2F_Index["th2f_timeGr_vs_grWidth"]].Fill(par2, minId)

            if fillOnce == minId and fillOnce < 4:
                if not math.isnan(cdcEventT0):
                    hname = "th2f_sig" + str(fillOnce) + "GrMean_vs_cdcEventT0"
                    self.TH2F_Store[self.TH2F_Index[hname]].Fill(par1, cdcEventT0)
                    hname = "th2f_sig" + str(fillOnce) + "GrMean_vs_cdcEventT0_rot45neg"
                    self.TH2F_Store[self.TH2F_Index[hname]].Fill(
                        par1 * TMath.Cos(-TMath.PiOver4()) + cdcEventT0 * TMath.Sin(-TMath.PiOver4()),
                        - par1 * TMath.Sin(-TMath.PiOver4()) + cdcEventT0 * TMath.Cos(-TMath.PiOver4()))
                    hname = "th2f_sig" + str(fillOnce) + "GrMean_vs_cdcEventT0_rot45pos"
                    self.TH2F_Store[self.TH2F_Index[hname]].Fill(
                        par1 * TMath.Cos(TMath.PiOver4()) + cdcEventT0 * TMath.Sin(TMath.PiOver4()),
                        - par1 * TMath.Sin(TMath.PiOver4()) + cdcEventT0 * TMath.Cos(TMath.PiOver4()))
                fillOnce += 1

            if par1 > sigMin and par1 < sigMax:
                self.TH1F_Store[self.TH1F_Index["th1f_sigClsTime_PreTracking"]].Fill(clsTime)
            else:
                self.TH1F_Store[self.TH1F_Index["th1f_bkgClsTime_PreTracking"]].Fill(clsTime)

        # Post Tracking

        recoTrackList = Belle2.PyStoreArray('RecoTracks')
        nTracks = len(recoTrackList)
        nClusters = 0
        # print("nTracks ", nTracks)
        for recoTrk in recoTrackList:
            # print(type(recoTrk))
            if not recoTrk.wasFitSuccessful():
                nTracks -= 1
                continue

            trackList = recoTrk.getRelationsFrom('Tracks')
            # print("trackList ", len(trackList))
            if len(trackList) == 0:
                nTracks -= 1
                continue
            trkTime0 = trackList[0].getTrackTime()

            self.TH1F_Store[self.TH1F_Index["th1f_trackTime"]].Fill(trkTime0)
            if not math.isnan(cdcEventT0):
                self.TH1F_Store[self.TH1F_Index["th1f_trackTimeCDCRef"]].Fill(trkTime0 - cdcEventT0)

            clusterList = recoTrk.getRelationsFrom("SVDClusters")
            # print("SVD Hits ", len(clusterList))
            nClusters += len(clusterList)
            if len(clusterList) == 0:
                continue

            trkTime = 0
            for d in clusterList:
                trkTime += d.getClsTime()
            trkTime /= len(clusterList)

            for d in clusterList:
                clsTime = d.getClsTime()
                groupIds = d.getTimeGroupId()
                groupInfo = d.getTimeGroupInfo()

                if not math.isnan(svdEventT0):
                    self.TH2F_Store[self.TH2F_Index["th2f_trkTime_vs_clsTime_inSVDEventT0"]].Fill(
                        trkTime0 - clsTime, svdEventT0 - trkTime0)
                if not math.isnan(cdcEventT0):
                    self.TH2F_Store[self.TH2F_Index["th2f_trkTime_vs_clsTime_inCDCEventT0"]].Fill(
                        trkTime0 - clsTime, cdcEventT0 - trkTime0)
                    self.TH2F_Store[self.TH2F_Index["th2f_aveClsTimeInTrk_vs_cdcEventT0"]].Fill(trkTime, cdcEventT0)
                if not math.isnan(svdEventT0):
                    self.TH2F_Store[self.TH2F_Index["th2f_aveClsTimeInTrk_vs_clsTime_inSVDEventT0"]].Fill(
                        trkTime - clsTime, svdEventT0 - trkTime)
                if not math.isnan(cdcEventT0):
                    self.TH2F_Store[self.TH2F_Index["th2f_aveClsTimeInTrk_vs_clsTime_inCDCEventT0"]].Fill(
                        trkTime - clsTime, cdcEventT0 - trkTime)

                self.TH1F_Store[self.TH1F_Index["th1f_clsTime_PostTracking"]].Fill(clsTime)
                if not math.isnan(cdcEventT0):
                    self.TH1F_Store[self.TH1F_Index["th1f_clsTimeCdcRef_PostTracking"]].Fill(clsTime - cdcEventT0)
                if not math.isnan(topOnlineEventT0) and eclTCEmax:
                    self.reso_TH2D_Histograms[self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_clsTimeTopRef_PostTracking"]].Fill(
                        eclTCEmax, clsTime - topOnlineEventT0)

                minId = 1000
                par0 = -1000
                par1 = -1000
                par2 = -1000
                if len(groupIds) == 0:
                    minId = -1
                for id in range(len(groupIds)):
                    gr = groupIds[id]
                    if gr < minId:
                        minId = gr
                        if len(groupInfo) > 0:
                            par0, par1, par2 = groupInfo[id]

                self.TH2F_Store[self.TH2F_Index["th2f_timeGr_vs_clsTime_PostTracking"]].Fill(clsTime, minId)

                if par1 > sigMin and par1 < sigMax:
                    self.TH1F_Store[self.TH1F_Index["th1f_sigClsTime_PostTracking"]].Fill(clsTime)
                else:
                    self.TH1F_Store[self.TH1F_Index["th1f_bkgClsTime_PostTracking"]].Fill(clsTime)

        if eclTCEmax:
            self.reso_TH2D_Histograms[self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_totalTracks"]].Fill(eclTCEmax, nTracks)
            self.reso_TH2D_Histograms[self.reso_TH2D_Histogram_Index["reso_eclTCEmax_vs_totalClusters_PostTracking"]].Fill(
                eclTCEmax, nClusters)

    def terminate(self):
        '''write'''

        self.f.mkdir("objects")
        self.f.cd("objects")

        for hist in self.TH1F_Store:
            if "_State" in hist.GetName():
                hist.Scale(1./hist.GetSumOfWeights())
            hist.Write()

        for hist in self.TH2F_Store:
            hist.Write()

        for hist in self.reso_TH2D_Histograms:
            hist.Write()

        self.f.Close()


main = b2.create_path()

b2.set_random_seed(args.seed)

if args.isMC:
    # options for simulation:
    # expList = [1003]
    expList = [0]
    numEvents = 10
    # bkgFiles = glob.glob('/home/surya/products/basf2/bkg_files/*.root')  # Phase3 background
    bkgFiles = get_background_files()  # Phase3 background
    # bkgFiles = None  # uncomment to remove  background
    simulateJitter = False
    ROIfinding = False
    MCTracking = False
    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param('expList', expList)
    eventinfosetter.param('runList', [0])
    eventinfosetter.param('evtNumList', [numEvents])
    main.add_module(eventinfosetter)
    ge.add_evtgen_generator(path=main, finalstate='mixed')
    # main.add_module('EvtGenInput')

    sim.add_simulation(main, bkgfiles=bkgFiles)

    if args.is3sample:
        for m in main.modules():
            if m.name() == "SVDEventInfoSetter":
                m.param("daqMode", 1)
                m.param("relativeShift", 7)


else:
    # setup database
    b2conditions.reset()
    b2conditions.override_globaltags()
    b2conditions.globaltags = ["online"]
    b2conditions.prepend_globaltag("data_reprocessing_prompt")
    b2conditions.prepend_globaltag("patch_main_release-07")
    if args.CoG3TimeCalibration_bucket36:
        b2conditions.prepend_globaltag("svd_CoG3TimeCalibration_bucket36_withGrouping_pol3")
    if args.CoG3TimeCalibration_bucket32:
        b2conditions.prepend_globaltag("svd_CoG3TimeCalibration_bucket32_withGrouping_pol3")

    MCTracking = False


if args.OffOff:
    b2conditions.prepend_globaltag("tracking_TEST_SVDTimeSelectionOFFrev1_VXDTF2TimeFiltersOFFrev28")
if args.OnOn:
    b2conditions.prepend_globaltag("tracking_TEST_SVDTimeSelectionONrev5_VXDTF2TimeFiltersONrev27")
if args.OffOn:
    b2conditions.prepend_globaltag("tracking_TEST_SVDTimeSelectionOFFrev1_VXDTF2TimeFiltersONrev27")
if args.OnOff:
    b2conditions.prepend_globaltag("tracking_TEST_SVDTimeSelectionONrev5_VXDTF2TimeFiltersOFFrev28")


if not args.isMC:
    if args.test:
        main.add_module('RootInput', entrySequences=['0:100'])
    else:
        main.add_module('RootInput')

    main.add_module("Gearbox")
    main.add_module('Geometry', useDB=True)

    raw.add_unpackers(main)

    if args.is3sample:
        # change ListName
        for moda in main.modules():
            if moda.name() == 'SVDUnpacker':
                moda.param("svdShaperDigitListName", "SVDShaperDigits6Sample")
                moda.param("SVDEventInfo", "SVDEventInfo6Sample")

        # emulate 3-sample DAQ for events
        zsemulator = b2.register_module("SVD3SamplesEmulator")
        zsemulator.param("outputSVDShaperDigits", "SVDShaperDigits3SampleAll")
        zsemulator.param("outputSVDEventInfo", "SVDEventInfo")
        zsemulator.param("chooseStartingSample", False)
        zsemulator.param("chooseRelativeShift", True)
        zsemulator.param("relativeShift", 7)
        zsemulator.param("SVDShaperDigits", "SVDShaperDigits6Sample")
        zsemulator.param("SVDEventInfo", "SVDEventInfo6Sample")
        main.add_module(zsemulator)

        zsonline = b2.register_module("SVDZeroSuppressionEmulator")
        zsonline.param("ShaperDigits", "SVDShaperDigits3SampleAll")
        zsonline.param("ShaperDigitsIN", "SVDShaperDigits")
        main.add_module(zsonline)


# now do reconstruction:
trk.add_tracking_reconstruction(
    main,
    mcTrackFinding=MCTracking,
    append_full_grid_cdc_eventt0=True)

main.add_module(SVDGroupingPerformance())

for moda in main.modules():
    if moda.name() == 'SVDClusterizer':
        moda.param("returnClusterRawTime", args.isRawTime)
    if moda.name() == 'SVDTimeGrouping':
        if args.doSVDGrouping:
            moda.param('tRangeLow',  minTime)
            moda.param('tRangeHigh', maxTime)
            moda.param("expSignalLoc",    sigLoc)
            moda.param('signalRangeLow',  sigMin)
            moda.param('signalRangeHigh', sigMax)
            moda.param("factor", 2)
            moda.param("signalGroupSelection", args.signalGroupSelection)
            moda.param("formSuperGroup", args.formSuperGroup)
            moda.param("exponentialSort", args.exponentialSort)
        else:
            moda.param("factor", 0)
    if moda.name() == 'SVDSpacePointCreator':
        moda.param("useSVDGroupInfo", args.useSVDGroupInfo)


if args.executionTime:
    executionFileName = str(args.fileDir) + "SVDExecutionTime" + outputFileTag + ".root"
    main.add_module(SVDExtraEventStatisticsModule(executionFileName))

main.add_module('Progress')

b2.print_path(main)
b2.process(main)
print(b2.statistics)
