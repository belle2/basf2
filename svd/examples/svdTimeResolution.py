#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################################
#
# Example Script on simulation or real data
#
# Use:
# Single Job: basf2 -i <RAWDATAFILE> script -- --fileDir temp/ --fileTag test [...]
#
# Outputs from multiple jobs can be merged using tools like hadd, then can be fitted using
# basf2 script -- --readHistogramFromLocalFile --fileName <filename>
#
###########################################################################################

import basf2 as b2
from basf2 import conditions as b2conditions
import rawdata as raw
from tracking import add_tracking_reconstruction
import simulation as sim
import argparse
from background import get_background_files
import generators as ge

from ROOT import Belle2, TH1F, TFile, TCanvas, TPad, TMath, TPaveStats, TF1
import math
import collections.abc


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--seed", default="1",
                    help="seed")
parser.add_argument("--fileDir", default="./",
                    help="Output File Dir")
parser.add_argument("--fileTag", default="test",
                    help="Output File Tag")
parser.add_argument("--moduleName", default="SVDTimeResolution",
                    help="Module Prefix")
parser.add_argument("--isMC", action="store_true",
                    help="Use Simulation")
parser.add_argument("--isCoG6", action="store_true",
                    help="CoG6 cluster time")
parser.add_argument("--isELS3", action="store_true",
                    help="ELS3 cluster time")
parser.add_argument("--CoG3TimeCalibration_bucket36", action="store_true",
                    help="SVD Time calibration")
parser.add_argument("--CoG3TimeCalibration_bucket32", action="store_true",
                    help="SVD Time calibration")
parser.add_argument("--is3sample", action="store_true",
                    help="Emulate SVD 3 samples")
parser.add_argument("--relativeShift", type=int, default=7,
                    help="3-samples relative shift")
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
parser.add_argument("--readHistogramFromLocalFile", action="store_true",
                    help="Read pre-generated histogram to fit for double gaus")
parser.add_argument("--fileName", default="test.root",
                    help="Input filename")
args = parser.parse_args()
b2.B2INFO(f"Steering file args = {args}")

if args.readHistogramFromLocalFile:
    args.isMC = True

if args.test:
    b2.set_log_level(b2.LogLevel.DEBUG)
    b2.set_debug_level(1)


outputFileTag = ""
if args.isMC:
    outputFileTag += "_MC"
if args.is3sample:
    outputFileTag += "_emulated3sample_relShift" + str(args.relativeShift)
if args.isCoG6:
    outputFileTag += "_CoG6"
if args.isELS3:
    outputFileTag += "_ELS3"
if args.OffOff:
    outputFileTag += "_OffOff"
if args.OnOn:
    outputFileTag += "_OnOn"
if args.OffOn:
    outputFileTag += "_OffOn"
if args.OnOff:
    outputFileTag += "_OnOff"
outputFileTag += "_" + str(args.fileTag)


maxStrip = 6
maxType = 3


def get_sensor_type(layer, sensor):
    ''' returns the sensor types'''

    if layer == 3:
        return 0
    else:
        if sensor == 1:
            return 1
        else:
            return 2


def fGaus(x, par):
    f = par[0]
    a = par[1]
    b = par[2]
    return f*TMath.Gaus(x[0], a, b)

# Normalized function = (-N*(1-f)*%e^(-(x-c)^2/(2*d^2))-N*f*%e^(-(x-a)^2/(2*b^2)))/(sqrt(2)*sqrt(%pi)*N*((d-b)*f-d))
# mean = ((c*d-a*b)*f-c*d)/((d-b)*f-d)
# STD = ((d^3+c^2*d-b^3-a^2*b)*f-d^3-c^2*d)/((d-b)*f-d)


def fDoubleGaus(x, par):
    N = par[0]
    f = par[1]
    a = par[2]
    b = par[3]
    c = par[4]
    d = par[5]
    return N*f*TMath.Gaus(x[0], a, b) + N*(1-f)*TMath.Gaus(x[0], c, d)


def fDoubleGausMean(par):
    f = par[1]
    a = par[2]
    b = par[3]
    c = par[4]
    d = par[5]
    return ((c*d-a*b)*f-c*d)/((d-b)*f-d)


def fDoubleGausSTD(par):
    f = par[1]
    a = par[2]
    b = par[3]
    c = par[4]
    d = par[5]
    return ((d*d*d+c*c*d-b*b*b-a*a*b)*f-d*d*d-c*c*d)/((d-b)*f-d)


def printArray(someArray):
    if isinstance(someArray, collections.abc.Sequence):
        if isinstance(someArray[0], collections.abc.Sequence):
            return "[" + ','.join(f"{printArray(item)}" for item in someArray) + "]"
        else:
            return "\n[" + ','.join(f"{item:.6f}" for item in someArray) + "]"
    else:
        return f"{someArray:.6f}"


class SVDTimeResolution(b2.Module):
    ''' Plot SVD Clusters'''

    def initialize(self):
        '''define histograms'''

        #: \cond Doxygen_suppress
        self.c1 = TCanvas("c1", "c1", 640, 480)
        self.onePad = TPad("onePad", "onePad", 0, 0, 1, 1)
        self.onePad.SetFillColor(0)
        self.onePad.SetBorderMode(0)
        self.onePad.SetBorderSize(2)
        self.onePad.SetRightMargin(0.1339713)
        self.onePad.SetBottomMargin(0.15)
        self.onePad.SetFrameBorderMode(0)
        self.onePad.SetFrameBorderMode(0)
        self.onePad.SetNumber(1)
        self.onePad.Draw()

        self.c = TCanvas("c", "c", 640, 480)
        # Subpads
        self.topPad = TPad("topPad", "topPad", 0, 0.5, 1, 1)
        self.btmPad = TPad("btmPad", "btmPad", 0, 0, 1, 0.5)
        self.topPad.SetMargin(0.1, 0.1, 0, 0.149)
        self.btmPad.SetMargin(0.1, 0.1, 0.303, 0)
        self.topPad.SetNumber(1)
        self.btmPad.SetNumber(2)
        self.topPad.Draw()
        self.btmPad.Draw()

        self.outputFileName = str(args.fileDir) + str(args.moduleName) + outputFileTag

        self.outPDF = []
        self.outPDF.append(self.outputFileName + ".pdf")
        for item in self.outPDF:
            self.c1.Print(item + "[")

        if args.readHistogramFromLocalFile:
            self.inFile = TFile(str(args.fileName), "READ")
            return

        self.f = TFile(self.outputFileName + ".root", "RECREATE")

        self.TH1F_Index = {}
        self.TH1F_Store = []
        TH1F_Store_Count = 0

        for sType in range(maxType):
            for side in ["U", "V"]:
                for strip in range(maxStrip):
                    name = "th1f_residual_type" + str(sType) + "_" + side + "_size" + str(strip+1)
                    title = "Time Residual in type" + str(sType) + "_" + side + "_size" + str(strip+1)
                    self.TH1F_Store.append(TH1F(name, title, 500, -25, 25))
                    self.TH1F_Store[-1].GetXaxis().SetTitle("SVD Cluster Time Residual (ns)")
                    self.TH1F_Index[name] = TH1F_Store_Count
                    TH1F_Store_Count += 1
        #: \endcond

    def event(self):
        ''' Event loop'''

        if args.readHistogramFromLocalFile:
            return

        EventT0 = Belle2.PyStoreObj('EventT0')
        cdcEventT0 = math.nan
        cdc = Belle2.Const.DetectorSet(Belle2.Const.CDC)
        if EventT0.hasTemporaryEventT0(cdc):
            tmp = EventT0.getTemporaryEventT0s(Belle2.Const.CDC)
            cdcEventT0 = tmp.back().eventT0

        svdEventInfo = Belle2.PyStoreObj('SVDEventInfo')
        if not svdEventInfo:
            svdEventInfo = Belle2.PyStoreObj('SVDEventInfoSim')

        clusterList = Belle2.PyStoreArray("SVDClustersFromTracks")
        nClusters = len(clusterList)
        for clsItem in range(nClusters):
            d = clusterList[clsItem]

            clsTime = d.getClsTime()
            clsSize = d.getSize()
            isUCluster = d.isUCluster()
            clusterSide = "_U" if isUCluster == 1 else "_V"
            layerNumber = d.getSensorID().getLayerNumber()
            sensorNumber = d.getSensorID().getSensorNumber()
            sensorType = get_sensor_type(layerNumber, sensorNumber)

            fillSize = clsSize
            if clsSize > maxStrip:
                fillSize = maxStrip
            histName = "th1f_residual_type" + str(sensorType) + clusterSide + "_size" + str(fillSize)
            self.TH1F_Store[self.TH1F_Index[histName]].Fill(clsTime - cdcEventT0)

    def terminate(self):
        '''write'''

        if not args.readHistogramFromLocalFile:
            self.f.cd()
            for item in self.TH1F_Store:
                item.Write()

        sigma1_vals = []
        sigma_vals = []

        for sType in range(maxType):
            sigma1_vals.append([[0] * maxStrip, [0] * maxStrip])
            sigma_vals.append([[0] * maxStrip, [0] * maxStrip])
            print("sigma_vals", sigma_vals)

            for side in [0, 1]:
                sideChar = "U" if side == 1 else "V"
                for strip in range(maxStrip):
                    name = "th1f_residual_type" + str(sType) + "_" + sideChar + "_size" + str(strip+1)
                    item = self.TH1F_Store[self.TH1F_Index[name]] if not args.readHistogramFromLocalFile else self.inFile.Get(name)

                    self.onePad.cd()
                    self.onePad.SetGrid()
                    item.Draw()

                    fitGV = TF1("fitGV", fGaus, -25, 25, 3)
                    fitGV.SetParameter(0, 100000.)
                    fitGV.SetParameter(1, 0.)
                    fitGV.SetParameter(2, 3.)
                    status = item.Fit("fitGV", "S0")
                    if int(status):
                        continue
                    parV = []
                    parV.append(fitGV.GetParameter(0))
                    parV.append(1.)
                    parV.append(fitGV.GetParameter(1))
                    parV.append(math.fabs(fitGV.GetParameter(2) * 0.5))
                    parV.append(fitGV.GetParameter(1))
                    parV.append(math.fabs(fitGV.GetParameter(2) * 10.))
                    fitV = TF1("fitV", fDoubleGaus, -25, 25, 6)
                    fitV.SetParName(0, "N")
                    fitV.SetParName(1, "f")
                    fitV.SetParName(2, "#mu_{1}")
                    fitV.SetParName(3, "#sigma_{1}")
                    fitV.SetParName(4, "#mu_{2}")
                    fitV.SetParName(5, "#sigma_{2}")
                    for parval in range(len(parV)):
                        fitV.SetParameter(parval, parV[parval])
                    fitV.SetParLimits(1, 0.6, 1.)
                    # fitV.SetParLimits(2, -20, 20)
                    # fitV.SetParLimits(3, 1., 10.)
                    # fitV.SetParLimits(4, -20, 20)
                    # fitV.SetParLimits(5, 1., 30.)
                    status = item.Fit("fitV", "S")
                    if int(status):
                        continue
                    for parval in range(len(parV)):
                        parV[parval] = fitV.GetParameter(parval)
                    meanV = fDoubleGausMean(parV)
                    sigmaV = TMath.Sqrt(fDoubleGausSTD(parV) - meanV*meanV)
                    print("mean", meanV, "sigma", sigmaV)

                    sigma1_vals[sType][side][strip] = parV[3]
                    sigma_vals[sType][side][strip] = sigmaV

                    ptstats = TPaveStats(0.55, 0.6, 0.85, 0.88, "brNDC")
                    ptstats.SetName("stats")
                    ptstats.SetBorderSize(1)
                    ptstats.SetFillColor(0)
                    ptstats.SetFillStyle(0)
                    ptstats.SetTextAlign(12)
                    ptstats.SetTextFont(42)
                    ptstats.SetTextColor(1)
                    ptstats.SetOptStat(11)
                    ptstats.SetOptFit(111)
                    ptstats.Draw()
                    item.GetListOfFunctions().Add(ptstats)
                    ptstats.SetParent(item)
                    ptstats = TPaveStats(0.55, 0.5, 0.85, 0.6, "brNDC")
                    ptstats.SetName("stats1")
                    ptstats.SetBorderSize(1)
                    ptstats.SetFillColor(0)
                    ptstats.SetFillStyle(0)
                    ptstats.SetTextAlign(12)
                    ptstats.SetTextFont(42)
                    ptstats.SetTextColor(2)
                    ptstats.SetOptStat(11)
                    ptstats.Draw()
                    ptstats.AddText("#mu " + str(meanV))
                    ptstats.AddText("#sigma " + str(sigmaV))

                    self.c1.Print(self.outPDF[0], "Title:" + item.GetName())

        print("sigma1_vals", sigma1_vals)
        print("sigma_vals", sigma_vals)

        print("sigma1_vals")
        print(printArray(sigma1_vals))
        print("sigma_vals")
        print(printArray(sigma_vals))

        if not args.readHistogramFromLocalFile:
            self.f.Close()

        for item in self.outPDF:
            self.c1.Print(item + "]")


main = b2.create_path()

b2.set_random_seed(args.seed)

if args.isMC:
    # options for simulation:
    expList = [0]
    numEvents = 10
    if not args.readHistogramFromLocalFile:
        numEvents = 0
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

    if not args.readHistogramFromLocalFile:
        ge.add_evtgen_generator(path=main, finalstate='mixed')
        sim.add_simulation(main, bkgfiles=bkgFiles)

    if args.is3sample:
        for m in main.modules():
            if m.name() == "SVDEventInfoSetter":
                m.param("daqMode", 1)
                m.param("relativeShift", args.relativeShift)
                m.param("useDB", False)


else:
    # setup database
    b2conditions.reset()
    b2conditions.override_globaltags()
    b2conditions.globaltags = ["online"]
    b2conditions.prepend_globaltag("data_reprocessing_prompt")
    b2conditions.prepend_globaltag("patch_main_release-07")
    b2conditions.prepend_globaltag("patch_main_release-08")
    if args.CoG3TimeCalibration_bucket36:
        b2conditions.prepend_globaltag("svd_CoG3TimeCalibration_bucket36_withGrouping_pol3")
    if args.CoG3TimeCalibration_bucket32:
        b2conditions.prepend_globaltag("svd_CoG3TimeCalibration_bucket32_withGrouping_pol3")

    MCTracking = False

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
        zsemulator.param("relativeShift", args.relativeShift)
        zsemulator.param("SVDShaperDigits", "SVDShaperDigits6Sample")
        zsemulator.param("SVDEventInfo", "SVDEventInfo6Sample")
        main.add_module(zsemulator)

        zsonline = b2.register_module("SVDZeroSuppressionEmulator")
        zsonline.param("ShaperDigits", "SVDShaperDigits3SampleAll")
        zsonline.param("ShaperDigitsIN", "SVDShaperDigits")
        main.add_module(zsonline)


if not args.readHistogramFromLocalFile:
    add_tracking_reconstruction(main, components=['SVD', 'CDC'],
                                append_full_grid_cdc_eventt0=True)

    # Select Strips form Tracks
    fil = b2.register_module('SVDShaperDigitsFromTracks')
    fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
    main.add_module(fil)

    clusterizer = b2.register_module('SVDClusterizer')
    clusterizer.set_name('post_SVDClusterizer')
    clusterizer.param('ShaperDigits', 'SVDShaperDigitsFromTracks')
    clusterizer.param('Clusters', 'SVDClustersFromTracks')
    main.add_module(clusterizer)


main.add_module(SVDTimeResolution())

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
