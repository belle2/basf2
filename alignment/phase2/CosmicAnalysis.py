#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

gROOT.ProcessLine('struct TrackData {\
    float chi2;\
    float ndf;\
    float chiSquaredOverNdf;\
    float momentum;\
};')

gROOT.ProcessLine('struct DEDXData {\
    float acdCount;\
    float dE;\
    float path;\
    float momentumCDC;\
    int numberOfHits;\
    float distanceKLM;\
};')

from ROOT import TrackData, DEDXData


class CosmicAnalysis(Module):

    """A module to analyse residuals in overlaps of ladders."""

    def __init__(self):
        """Initialize the module"""

        super(CosmicAnalysis, self).__init__()

        self.rootfile = ROOT.TFile('cosmicAnalysis.root', 'recreate')

        self.tree_track = ROOT.TTree('track', '')
        self.tree_DEDX = ROOT.TTree('dedx', '')

        ROOT.gStyle.Reset()
        ROOT.gStyle.SetCanvasColor(0)
        ROOT.gStyle.SetStatBorderSize(1)
        ROOT.gStyle.SetStatColor(0)
        ROOT.gStyle.SetTitleColor(1)
        ROOT.gStyle.SetCanvasColor(0)
        ROOT.gStyle.SetPadColor(0)
        ROOT.gStyle.SetPadBorderMode(0)
        ROOT.gStyle.SetCanvasBorderMode(0)
        ROOT.gStyle.SetFrameBorderMode(0)
        ROOT.gStyle.SetOptStat(0)

        self.trackData = TrackData()
        # Declare tree branches
        for key in TrackData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.trackData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree_track.Branch(key, AddressOf(self.trackData, key), key + formstring)

        self.dedxData = DEDXData()
        # Declare tree branches
        for key in DEDXData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.dedxData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree_DEDX.Branch(key, AddressOf(self.dedxData, key), key + formstring)

        self.TotalNumberOfHits = ROOT.TH1F('TotalNumberOfHits', '', 6, 0.5, 6.5)
        self.HitsVsLayer = ROOT.TH2F('HitsVsLayer', '', 6, 0.5, 6.5, 6, 0.5, 6.5)
        self.HitsVsSensor = ROOT.TH2F('HitsVsSensor', '', 6, 0.5, 6.5, 5, 0.5, 5.5)
        self.LayerVsSensor = ROOT.TH2F('LayerVsSensor', '', 6, 0.5, 6.5, 5, 0.5, 5.5)
        self.Chi2 = ROOT.TH1F('Chi2', '', 300, 0.0, 500)
        self.NDF = ROOT.TH1F('NDF', '', 200, 0.0, 200)
        self.Chi2OverNDF = ROOT.TH1F('Chi2OverNDF', '', 300, 0.0, 5)
        self.Momentum = ROOT.TH1F('Momentum', '', 500, 0.0, 1000)
        self.ADCCountOverNumberOfHitsInCDC = ROOT.TH1F('ADCCountOverNumberOfHitsInCDC', '', 200, 0.0, 300)
        self.ADCCountOverNumberOfHitsInCDCVsMomentum = ROOT.TProfile(
            'ADCCountOverNumberOfHitsInCDCVsMomentum', '', 200, 0.0, 300, 0.0, 1000)
        self.MomentumVsADCCountOverNumberOfHitsInCDC = ROOT.TProfile(
            'MomentumVsADCCountOverNumberOfHitsInCDC', '', 100, 0.0, 100, 0.0, 1000)

    def beginRun(self):
        """Do nothing"""

    def event(self):
        # Study dEdx (CDC) as prediction of momentum
        cdcDedxTracks = Belle2.PyStoreArray('CDCDedxTracks')
        nCDCDedxTracks = cdcDedxTracks.getEntries()
        for DEDXTrack_index in range(nCDCDedxTracks):
            dedxTrack = cdcDedxTracks[DEDXTrack_index]
            self.dedxData.momentumCDC = dedxTrack.getMomentum()
            self.dedxData.acdCount = 0
            self.dedxData.dE = 0
            self.dedxData.path = 0
            self.dedxData.numberOfHits = dedxTrack.size()
            KLMClusters = Belle2.PyStoreArray('KLMClusters')
            nKLMClusters = KLMClusters.getEntries()
            if nKLMClusters == 2:
                for nHit in range(dedxTrack.size()):
                    self.dedxData.acdCount = self.dedxData.acdCount + dedxTrack.getADCCount(nHit)
                    self.dedxData.dE = self.dedxData.dE + dedxTrack.getDE(nHit)
                    self.dedxData.path = self.dedxData.path + dedxTrack.getPath(nHit)
                    self.dedxData.distanceKLM = math.sqrt(
                        math.pow(
                            KLMClusters[0].getClusterPosition().Z() -
                            KLMClusters[1].getClusterPosition().Z(),
                            2))

                self.ADCCountOverNumberOfHitsInCDC.Fill(self.dedxData.acdCount / dedxTrack.size())
                self.MomentumVsADCCountOverNumberOfHitsInCDC.Fill(
                    self.dedxData.momentumCDC, self.dedxData.acdCount / dedxTrack.size())
                self.ADCCountOverNumberOfHitsInCDCVsMomentum.Fill(
                    self.dedxData.acdCount / dedxTrack.size(), self.dedxData.momentumCDC)

                self.rootfile.cd()
                self.tree_DEDX.Fill()

        # We are using RecoTracks to finding overlaps.
        RecoTracks = Belle2.PyStoreArray('CosmicRecoTracks')
        nRecoTracks = RecoTracks.getEntries()
        geoCache = Belle2.VXD.GeoCache.getInstance()
        for track_index in range(nRecoTracks):
            track = RecoTracks[track_index]

            if track.wasFitSuccessful():

                TrackFitResults = Belle2.PyStoreArray('CosmicTrackFitResults')
                nTrackFitResults = TrackFitResults.getEntries()
                if nTrackFitResults == 1:
                    self.trackData.momentum = TrackFitResults[0].getMomentum().Mag()
                    self.Momentum.Fill(TrackFitResults[0].getMomentum().Mag())

                if track.hasPXDHits() or track.hasSVDHits():

                    if track.getNumberOfSVDHits() % 2 == 0:
                        totalNumberOfHits = track.getNumberOfPXDHits() + track.getNumberOfSVDHits() / 2
                    else:
                        totalNumberOfHits = track.getNumberOfPXDHits() + (track.getNumberOfSVDHits() - 1) / 2

                    self.TotalNumberOfHits.Fill(totalNumberOfHits)
                    print('Total number of hits:', totalNumberOfHits)

                    self.Chi2.Fill(track.getTrackFitStatus().getChi2())
                    self.NDF.Fill(track.getTrackFitStatus().getNdf())
                    self.Chi2OverNDF.Fill(track.getTrackFitStatus().getChi2() / track.getTrackFitStatus().getNdf())

                    self.trackData.chi2 = track.getTrackFitStatus().getChi2()
                    self.trackData.ndf = track.getTrackFitStatus().getNdf()
                    self.trackData.chiSquaredOverNdf = track.getTrackFitStatus().getChi2() / track.getTrackFitStatus().getNdf()
                    print('Chi2/NDF:', self.trackData.chiSquaredOverNdf, 'Chi2:', self.trackData.chi2)
                    self.rootfile.cd()
                    self.tree_track.Fill()

                # Check overlaps in PXD ladders & Check track in RecoTracks
                if track.hasPXDHits():

                    # Print number of PXD hits
                    EventMetaData = Belle2.PyStoreObj('EventMetaData')
                    event = EventMetaData.getEvent()
                    nPxdHits = track.getNumberOfPXDHits()
                    print('Event', event, 'has PXD', nPxdHits, 'hit(s):')

                    # First loop over PXD Hits
                    for n in range(0, len(track.getPXDHitList())):
                        pxdHit = track.getPXDHitList()[n]
                        sensorID = Belle2.VxdID(pxdHit.getSensorID())
                        info = geoCache.get(sensorID)
                        layer = sensorID.getLayerNumber()
                        ladder = sensorID.getLadderNumber()
                        sensor = sensorID.getSensorNumber()
                        print('Hit information: #hit:', n, 'sensor information:', layer, ladder, sensor)
                        self.HitsVsLayer.Fill(totalNumberOfHits, layer)
                        self.HitsVsSensor.Fill(totalNumberOfHits, sensor)
                        self.LayerVsSensor.Fill(layer, sensor)

                # Check overlaps in SVD ladders
                if track.hasSVDHits():

                    # Print number of SVD hits
                    EventMetaData = Belle2.PyStoreObj('EventMetaData')
                    event = EventMetaData.getEvent()
                    nSvdHits = track.getNumberOfSVDHits()
                    print('Event', event, 'has SVD', nSvdHits, 'hit(s):')

                    # First loop over SVD Hits
                    measured = ROOT.TVector3(0, 0, 0)
                    expected = ROOT.TVector3(0, 0, 0)
                    for n in range(0, len(track.getSVDHitList())):
                        svdHit = track.getSVDHitList()[n]
                        sensorID = Belle2.VxdID(svdHit.getSensorID())
                        info = geoCache.get(sensorID)
                        layer = sensorID.getLayerNumber()
                        ladder = sensorID.getLadderNumber()
                        sensor = sensorID.getSensorNumber()

                        if svdHit.isUCluster():
                            print('Hit information: #hit:', n, 'sensor information:', layer, ladder, sensor, 'isU')
                            for l in range(0, len(track.getSVDHitList())):
                                svdHitA = track.getSVDHitList()[l]
                                sensorIDA = Belle2.VxdID(svdHitA.getSensorID())
                                if info == geoCache.get(sensorIDA):
                                    if svdHitA.isUCluster() == 0:
                                        print('Hit information: #hit:', n, 'sensor information:', layer, ladder, sensor)
                                        self.HitsVsLayer.Fill(totalNumberOfHits, layer)
                                        self.HitsVsSensor.Fill(totalNumberOfHits, sensor)
                                        self.LayerVsSensor.Fill(layer, sensor)
                        # else:
                            # print('Hit information: #hit:',  n, 'sensor information:', layer, ladder, sensor, 'isNotU')
            else:
                print('Fit was not successful')

    def terminate(self):
        """Close & write output files"""
        self.rootfile.cd()

        labelHits = ('', 'One', 'Two', 'Three', 'Four', 'Five', 'Six')
        labelLayers = ('', 'First', 'Second', 'Third', 'Fourth', 'Fifth', 'Sixth')
        labelSensors = ('', 'First', 'Second', 'Third', 'Fourth', 'Fifth')

        for i in range(1, 7):
            self.TotalNumberOfHits.GetXaxis().SetBinLabel(i, labelHits[i])
        self.TotalNumberOfHits.GetXaxis().SetLabelSize(0.05)
        self.TotalNumberOfHits.GetYaxis().SetTitle("Number of tracks in VXD")
        self.TotalNumberOfHits.GetXaxis().SetTitle("Number of hits in VXD")
        self.TotalNumberOfHits.GetYaxis().CenterTitle()
        self.TotalNumberOfHits.GetXaxis().CenterTitle()
        self.TotalNumberOfHits.GetYaxis().SetTitleOffset(1.3)
        self.TotalNumberOfHits.GetXaxis().SetTitleOffset(1.3)
        self.TotalNumberOfHits.SetFillStyle(3365)
        self.TotalNumberOfHits.SetFillColor(9)
        self.TotalNumberOfHits.SetLineColor(9)
        self.TotalNumberOfHits.Draw()
        # self.TotalNumberOfHits.Write()

        for i in range(1, 7):
            self.HitsVsLayer.GetXaxis().SetBinLabel(i, labelHits[i])
            self.HitsVsLayer.GetYaxis().SetBinLabel(i, labelLayers[i])
        self.HitsVsLayer.GetXaxis().SetLabelSize(0.05)
        self.HitsVsLayer.GetYaxis().SetLabelSize(0.05)
        self.HitsVsLayer.GetXaxis().SetTitle("Number hits in VXD")
        self.HitsVsLayer.GetYaxis().SetTitle("Layer")
        self.HitsVsLayer.GetYaxis().CenterTitle()
        self.HitsVsLayer.GetXaxis().CenterTitle()
        self.HitsVsLayer.GetYaxis().SetTitleOffset(1.6)
        self.HitsVsLayer.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.HitsVsLayer.Draw()
        # self.HitsVsLayer.Write()

        for i in range(1, 7):
            self.HitsVsSensor.GetXaxis().SetBinLabel(i, labelHits[i])
        for i in range(1, 6):
            self.HitsVsSensor.GetYaxis().SetBinLabel(i, labelSensors[i])
        self.HitsVsSensor.GetXaxis().SetLabelSize(0.05)
        self.HitsVsSensor.GetYaxis().SetLabelSize(0.05)
        self.HitsVsSensor.GetXaxis().SetTitle("Number hits in VXD")
        self.HitsVsSensor.GetYaxis().SetTitle("Sensor")
        self.HitsVsSensor.GetYaxis().CenterTitle()
        self.HitsVsSensor.GetXaxis().CenterTitle()
        self.HitsVsSensor.GetYaxis().SetTitleOffset(1.6)
        self.HitsVsSensor.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.HitsVsSensor.Draw()
        # self.HitsVsSensor.Write()

        for i in range(1, 7):
            self.LayerVsSensor.GetXaxis().SetBinLabel(i, labelLayers[i])
        for i in range(1, 6):
            self.LayerVsSensor.GetYaxis().SetBinLabel(i, labelSensors[i])
        self.LayerVsSensor.GetXaxis().SetLabelSize(0.05)
        self.LayerVsSensor.GetYaxis().SetLabelSize(0.05)
        self.LayerVsSensor.GetXaxis().SetTitle("Layer")
        self.LayerVsSensor.GetYaxis().SetTitle("Sensor")
        self.LayerVsSensor.GetYaxis().CenterTitle()
        self.LayerVsSensor.GetXaxis().CenterTitle()
        self.LayerVsSensor.GetYaxis().SetTitleOffset(1.6)
        self.LayerVsSensor.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.LayerVsSensor.Draw()
        # self.LayerVsSensor.Write()

        self.Chi2.GetXaxis().SetTitle("#Chi^{2}")
        self.Chi2.GetYaxis().SetTitle("Number of tracks")
        self.Chi2.GetYaxis().CenterTitle()
        self.Chi2.GetXaxis().CenterTitle()
        self.Chi2.GetYaxis().SetTitleOffset(1.3)
        self.Chi2.GetXaxis().SetTitleOffset(1.3)
        self.Chi2.SetFillStyle(3365)
        self.Chi2.SetFillColor(9)
        self.Chi2.SetLineColor(9)
        self.Chi2.Draw()
        # self.Chi2.Write()

        self.NDF.GetXaxis().SetTitle("Degrees of freedom")
        self.NDF.GetYaxis().SetTitle("Number of tracks")
        self.NDF.GetYaxis().CenterTitle()
        self.NDF.GetXaxis().CenterTitle()
        self.NDF.GetYaxis().SetTitleOffset(1.3)
        self.NDF.GetXaxis().SetTitleOffset(1.3)
        self.NDF.SetFillStyle(3365)
        self.NDF.SetFillColor(9)
        self.NDF.SetLineColor(9)
        self.NDF.Draw()
        # self.NDF.Write()

        self.Chi2OverNDF.SetTitle("#Chi^{2}/Degrees of freedom")
        self.Chi2OverNDF.GetYaxis().SetTitle("Number of tracks")
        self.Chi2OverNDF.GetYaxis().CenterTitle()
        self.Chi2OverNDF.GetXaxis().CenterTitle()
        self.Chi2OverNDF.GetYaxis().SetTitleOffset(1.3)
        self.Chi2OverNDF.GetXaxis().SetTitleOffset(1.3)
        self.Chi2OverNDF.SetFillStyle(3365)
        self.Chi2OverNDF.SetFillColor(9)
        self.Chi2OverNDF.SetLineColor(9)
        self.Chi2OverNDF.Draw()
        # self.Chi2OverNDF.Write()

        self.Momentum.GetXaxis().SetTitle("Momentum [GeVc^{-1}]")
        self.Momentum.GetYaxis().SetTitle("Number of tracks")
        self.Momentum.GetYaxis().CenterTitle()
        self.Momentum.GetXaxis().CenterTitle()
        self.Momentum.GetYaxis().SetTitleOffset(1.3)
        self.Momentum.GetXaxis().SetTitleOffset(1.3)
        self.Momentum.SetFillStyle(3365)
        self.Momentum.SetFillColor(9)
        self.Momentum.SetLineColor(9)
        self.Momentum.Draw()
        # self.Momentum.Write()

        self.ADCCountOverNumberOfHitsInCDC.GetXaxis().SetTitle("ADC Count [ADC] / number of hit")
        self.ADCCountOverNumberOfHitsInCDC.GetYaxis().SetTitle("Number of tracks")
        self.ADCCountOverNumberOfHitsInCDC.GetYaxis().CenterTitle()
        self.ADCCountOverNumberOfHitsInCDC.GetXaxis().CenterTitle()
        self.ADCCountOverNumberOfHitsInCDC.GetYaxis().SetTitleOffset(1.3)
        self.ADCCountOverNumberOfHitsInCDC.GetXaxis().SetTitleOffset(1.3)
        self.ADCCountOverNumberOfHitsInCDC.SetFillStyle(3365)
        self.ADCCountOverNumberOfHitsInCDC.SetFillColor(9)
        self.ADCCountOverNumberOfHitsInCDC.SetLineColor(9)
        self.ADCCountOverNumberOfHitsInCDC.Draw()
        # self.ADCCountOverNumberOfHitsInCDC.Write()

        function = ROOT.TF1("function", "[0]+[1]*log(x)")
        function.SetParameters(0.0, 0.0)
        self.MomentumVsADCCountOverNumberOfHitsInCDC.GetYaxis().SetTitle("ADC Count [ADC] / number of hit")
        self.MomentumVsADCCountOverNumberOfHitsInCDC.GetXaxis().SetTitle("Momentum [GeVc^{-1}]")
        self.MomentumVsADCCountOverNumberOfHitsInCDC.GetYaxis().CenterTitle()
        self.MomentumVsADCCountOverNumberOfHitsInCDC.GetXaxis().CenterTitle()
        self.MomentumVsADCCountOverNumberOfHitsInCDC.GetYaxis().SetTitleOffset(1.3)
        self.MomentumVsADCCountOverNumberOfHitsInCDC.GetXaxis().SetTitleOffset(1.3)
        self.MomentumVsADCCountOverNumberOfHitsInCDC.Fit(function)
        self.MomentumVsADCCountOverNumberOfHitsInCDC.Draw()
        # self.MomentumVsADCCountOverNumberOfHitsInCDC.Write()

        inverse_function = ROOT.TF1("inverse_function", "exp((-[0]+x)/[1])", 0, 115)
        inverse_function.SetParameter(0, function.GetParameter(0))
        inverse_function.SetParameter(1, function.GetParameter(1))
        constant = ROOT.TF1("constant", "[0]", 115, 300)
        constant.SetParameter(0, 20)
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.Fit(inverse_function, "R+")
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.Fit(constant, "R+")
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.GetXaxis().SetTitle("ADC Count [ADC] / number of hit")
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.GetYaxis().SetTitle("Momentum [GeVc^{-1}]")
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.GetYaxis().CenterTitle()
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.GetXaxis().CenterTitle()
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.GetYaxis().SetTitleOffset(1.3)
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.GetXaxis().SetTitleOffset(1.3)
        self.ADCCountOverNumberOfHitsInCDCVsMomentum.Draw()
        # self.ADCCountOverNumberOfHitsInCDCVsMomentum.Write()

        print('Parameters of fitted function y = exp((-p0+x)/p1) + p2, where x ~ ADCCounts/NumberOfCDCHits, y ~ momentum')
        print('p0 =', inverse_function.GetParameter(0), 'p1 =', inverse_function.GetParameter(1), 'p2 =', constant.GetParameter(0))

        self.rootfile.Write()
        self.rootfile.Close()
