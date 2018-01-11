#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

gROOT.ProcessLine('struct VXDData {\
    int layer;\
    int ladder;\
    int sensor;\
    float measuredU;\
    float measuredV;\
    float measuredX;\
    float measuredY;\
    float measuredRho;\
    float measuredPhi;\
    float measuredZ;\
    float expectedU;\
    float expectedV;\
    float expectedX;\
    float expectedY;\
    float expectedRho;\
    float expectedPhi;\
    float expectedZ;\
 };')

gROOT.ProcessLine('struct TrackData {\
    float chi2;\
    float ndf;\
    float chiSquaredOverNdf;\
    float momentum;\
};')

from ROOT import VXDData, TrackData


class VXDHits(Module):

    """A module to analyse residuals in overlaps of ladders."""

    def __init__(self):
        """Initialize the module"""

        super(VXDHits, self).__init__()

        # self.file = open('Overlaps.txt','w')

        self.rootfile = ROOT.TFile('vxdhits.root', 'recreate')

        self.tree_VXDHits = ROOT.TTree('vxdhits', '')
        self.tree_track = ROOT.TTree('track', '')

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

        self.vxdData = VXDData()
        # Declare tree branches
        for key in VXDData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.vxdData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree_VXDHits.Branch(key, AddressOf(self.vxdData, key), key + formstring)

        self.trackData = TrackData()
        # Declare tree branches
        for key in TrackData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.trackData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree_track.Branch(key, AddressOf(self.trackData, key), key + formstring)

        self.TotalNumberOfHits = ROOT.TH1F('TotalNumberOfHits', '', 6, 0.5, 6.5)
        self.HitsVsLayer = ROOT.TH2F('HitsVsLayer', '', 6, 0.5, 6.5, 6, 0.5, 6.5)
        self.HitsVsSensor = ROOT.TH2F('HitsVsSensor', '', 6, 0.5, 6.5, 5, 0.5, 5.5)
        self.LayerVsSensor = ROOT.TH2F('LayerVsSensor', '', 6, 0.5, 6.5, 5, 0.5, 5.5)

    def beginRun(self):
        """Do nothing"""

    def event(self):

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

                if track.hasPXDHits() or track.hasSVDHits():

                    if track.getNumberOfSVDHits() % 2 == 0:
                        totalNumberOfHits = track.getNumberOfPXDHits() + track.getNumberOfSVDHits() / 2
                    else:
                        totalNumberOfHits = track.getNumberOfPXDHits() + (track.getNumberOfSVDHits() - 1) / 2

                    self.TotalNumberOfHits.Fill(totalNumberOfHits)
                    # print ('Total number of hits:', totalNumberOfHits)

                    self.trackData.chi2 = track.getTrackFitStatus().getChi2()
                    self.trackData.ndf = track.getTrackFitStatus().getNdf()
                    self.trackData.chiSquaredOverNdf = track.getTrackFitStatus().getChi2() / track.getTrackFitStatus().getNdf()

                    # print('Chi2/NDF:', self.trackData.chiSquaredOverNdf)
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
                    # print('Event', event, 'has SVD', nSvdHits, 'hit(s):')

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
                            # print('Hit information: #hit:',  n, 'sensor information:', layer, ladder, sensor, 'isU')
                            # measured[0] = svdHit.getPosition()
                            for l in range(0, len(track.getSVDHitList())):
                                svdHitA = track.getSVDHitList()[l]
                                sensorIDA = Belle2.VxdID(svdHitA.getSensorID())
                                if info == geoCache.get(sensorIDA):
                                    if svdHitA.isUCluster() == 0:
                                        # print('Hit information: #hit:',  n, 'sensor information:', layer, ladder, sensor)
                                        self.HitsVsLayer.Fill(totalNumberOfHits, layer)
                                        self.HitsVsSensor.Fill(totalNumberOfHits, sensor)
                                        self.LayerVsSensor.Fill(layer, sensor)
                        # else:
                            # print('Hit information: #hit:',  n, 'sensor information:', layer, ladder, sensor, 'isNotU')

    def terminate(self):
        """Close & write output files"""
        self.rootfile.cd()

        labelHits = ('', 'One', 'Two', 'Three', 'Four', 'Five', 'Six')
        labelLayers = ('', 'First', 'Second', 'Third', 'Fourth', 'Fifth', 'Sixth')
        labelSensors = ('', 'First', 'Second', 'Third', 'Fourth', 'Fifth')

        self.TotalNumberOfHitsCanvas = ROOT.TCanvas('TotalNumberOfHitsCanvas', '', 800, 600)
        for i in range(1, 7):
            self.TotalNumberOfHits.GetXaxis().SetBinLabel(i, labelHits[i])  # .c_str()
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
        self.TotalNumberOfHitsCanvas.Write()
        # self.TotalNumberOfHitsCanvas.Close()

        self.HitsVsLayerCanvas = ROOT.TCanvas('HitsVsLayerCanvas', '', 800, 600)
        for i in range(1, 7):
            self.HitsVsLayer.GetXaxis().SetBinLabel(i, labelHits[i])  # .c_str()
            self.HitsVsLayer.GetYaxis().SetBinLabel(i, labelLayers[i])  # .c_str()
        self.HitsVsLayer.GetXaxis().SetLabelSize(0.05)
        self.HitsVsLayer.GetYaxis().SetLabelSize(0.05)
        self.HitsVsLayer.GetXaxis().SetTitle("Number hits in VXD")
        self.HitsVsLayer.GetYaxis().SetTitle("Layer")
        self.HitsVsLayer.GetYaxis().CenterTitle()
        self.HitsVsLayer.GetXaxis().CenterTitle()
        self.HitsVsLayer.GetYaxis().SetTitleOffset(1.6)
        self.HitsVsLayer.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.HitsVsLayer.Draw('colz')
        self.HitsVsLayerCanvas.Write()
        # self.HitsVsLayerCanvas.Close()

        self.HitsVsSensorCanvas = ROOT.TCanvas('HitsVsSensorCanvas', '', 800, 600)
        for i in range(1, 7):
            self.HitsVsSensor.GetXaxis().SetBinLabel(i, labelHits[i])  # .c_str()
        for i in range(1, 6):
            self.HitsVsSensor.GetYaxis().SetBinLabel(i, labelSensors[i])  # .c_str()
        self.HitsVsSensor.GetXaxis().SetLabelSize(0.05)
        self.HitsVsSensor.GetYaxis().SetLabelSize(0.05)
        self.HitsVsSensor.GetXaxis().SetTitle("Number hits in VXD")
        self.HitsVsSensor.GetYaxis().SetTitle("Sensor")
        self.HitsVsSensor.GetYaxis().CenterTitle()
        self.HitsVsSensor.GetXaxis().CenterTitle()
        self.HitsVsSensor.GetYaxis().SetTitleOffset(1.6)
        self.HitsVsSensor.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.HitsVsSensor.Draw('colz')
        self.HitsVsSensorCanvas.Write()
        # self.HitsVsSensorCanvas.Close()

        self.LayerVsSensorCanvas = ROOT.TCanvas('LayerVsSensorCanvas', '', 800, 600)
        for i in range(1, 7):
            self.LayerVsSensor.GetXaxis().SetBinLabel(i, labelLayers[i])  # .c_str()
        for i in range(1, 6):
            self.LayerVsSensor.GetYaxis().SetBinLabel(i, labelSensors[i])  # .c_str()
        self.LayerVsSensor.GetXaxis().SetLabelSize(0.05)
        self.LayerVsSensor.GetYaxis().SetLabelSize(0.05)
        self.LayerVsSensor.GetXaxis().SetTitle("Layer")
        self.LayerVsSensor.GetYaxis().SetTitle("Sensor")
        self.LayerVsSensor.GetYaxis().CenterTitle()
        self.LayerVsSensor.GetXaxis().CenterTitle()
        self.LayerVsSensor.GetYaxis().SetTitleOffset(1.6)
        self.LayerVsSensor.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.LayerVsSensor.Draw('colz')
        self.LayerVsSensorCanvas.Write()
        # self.LayerVsSensorCanvas.Close()

        self.rootfile.Write()
        self.rootfile.Close()
