#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
import itertools
from basf2 import *

import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf
import datetime

gROOT.ProcessLine('struct OverlapsData {\
    int firstLayer;\
    int firstLadder;\
    int firstSensor;\
    int secondLayer;\
    int secondLadder;\
    int secondSensor;\
    float firstUHit;\
    float firstVHit;\
    float firstUResidual;\
    float firstVResidual;\
    float firstUExpected;\
    float firstVExpected;\
    float firstUSlope;\
    float firstVSlope;\
    float secondUHit;\
    float secondVHit;\
    float secondUResidual;\
    float secondVResidual;\
    float secondUExpected;\
    float secondVExpected;\
    float secondUSlope;\
    float secondVSlope;\
};')
gROOT.ProcessLine('struct OtherResidualsData {\
    int layer;\
    int ladder;\
    int sensor;\
    float uHit;\
    float vHit;\
    float uResidual;\
    float vResidual;\
    float uExpected;\
    float vExpected;\
    float uSlope;\
    float vSlope;\
};')

from ROOT import OverlapsData, OtherResidualsData


class CosmicAnalysis(Module):

    """A module to monitor VXD alignment and select data for overlap analysis to Trees (ROOT)."""

    def __init__(self, output):
        """Initialize the module"""
        self.output = output
        super(CosmicAnalysis, self).__init__()

        self.first_time = 0
        self.last_time = 0
        self.numberTracks = 0
        self.numberEvents = 0

        outputFile = self.output
        self.rootfile = ROOT.TFile(outputFile, 'recreate')

        self.tree_overlaps = ROOT.TTree('overlaps', '')
        self.tree_residuals = ROOT.TTree('otherResiduals', '')

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
        ROOT.gStyle.SetOptStat(1111)

        self.overlapsData = OverlapsData()
        for key in OverlapsData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.overlapsData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree_overlaps.Branch(key, AddressOf(self.overlapsData, key), key + formstring)

        self.otherResidualsData = OtherResidualsData()
        for key in OtherResidualsData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.otherResidualsData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree_residuals.Branch(key, AddressOf(self.otherResidualsData, key), key + formstring)

        self.NumberTracks = ROOT.TH1F('NumberTracks', '', 10, 0.0, 10)
        self.NumberTracksInPXD = ROOT.TH1F('NumberTracksInPXD', '', 2, 0.5, 2.5)
        self.TotalNumberOfHits = ROOT.TH1F('TotalNumberOfHits', '', 35, 0.5, 35.5)

        self.LayerVsLadder = ROOT.TH2F('LayerVsLadder', '', 6, 0.0, 6.0, 16, 0.0, 16.0)
        self.LadderVsSensor = ROOT.TH2F('LadderVsSensor', '', 16, 0.0, 16.0, 18, 0.0, 18.0)
        self.LayerVsSensor = ROOT.TH2F('LayerVsSensor', '', 6, 0.0, 6.0, 5, 0.0, 5.0)

        self.Chi2 = ROOT.TH1F('Chi2', '', 300, 0.0, 500)
        self.NDF = ROOT.TH1F('NDF', '', 200, 0.0, 200)
        self.Chi2OverNDF = ROOT.TH1F('Chi2OverNDF', '', 300, 0.0, 5)
        self.PValue = ROOT.TH1F('PValue', '', 400, 0.0, 1.0)
        self.Momentum = ROOT.TH1F('Momentum', '', 500, 0.0, 100)

        self.ZenithAngle = ROOT.TH1F('ZenithAngle', '', 90, 0, 90)

        self.HitsInLayer = ROOT.TH2F("HitsInLayer", '', 6, 0.0, 6.0, 4, 0.0, 4.0)
        self.NumberOfClustersPerSensor = ROOT.TH1F("NumberOfClustersPerSensor", '', 15, 0.0, 15.0)
        self.NumberOfSensorsPerTrack = ROOT.TH1F("NumberOfSensorsPerTrack", '', 15, 0.0, 15.0)

        self.ResidualU = ROOT.TH1F("ResidualU", '', 100, -400, 400)
        self.ResidualV = ROOT.TH1F("ResidualV", '', 100, -400, 400)

    def beginRun(self):
        """Do nothing"""

    def event(self):

        self.numberTracks += Belle2.PyStoreArray('RecoTracks').getEntries()
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        self.numberEvents += 1
        time = EventMetaData.getTime()
        utime = datetime.datetime.fromtimestamp(int(str(time)[:10]))
        # print(utime.strftime('%Y-%m-%d %H:%M:%S'))
        if self.first_time == 0 or self.first_time > time:
            self.first_time = time
        if time > self.last_time:
            self.last_time = time

        Tracks = Belle2.PyStoreArray('Tracks')

        RecoTracks = Belle2.PyStoreArray('RecoTracks')
        nRecoTracks = RecoTracks.getEntries()
        self.NumberTracks.Fill(nRecoTracks)
        geoCache = Belle2.VXD.GeoCache.getInstance()
        for track_index in range(nRecoTracks):
            track = RecoTracks[track_index]

            if track.wasFitSuccessful():
                if track.hasPXDHits():
                    self.NumberTracksInPXD.Fill(1)
                else:
                    self.NumberTracksInPXD.Fill(2)
                if track.hasSVDHits() or track.hasPXDHits():
                    totalNumberOfHits = track.getNumberOfSVDHits() + track.getNumberOfPXDHits() * 2
                    self.TotalNumberOfHits.Fill(totalNumberOfHits)

                    self.Chi2.Fill(track.getTrackFitStatus().getChi2())
                    self.NDF.Fill(track.getTrackFitStatus().getNdf())
                    # self.Chi2OverNDF.Fill(track.getTrackFitStatus().getChi2() / track.getTrackFitStatus().getNdf())
                    self.PValue.Fill(track.getTrackFitStatus().getPVal())

                    trackPointFirst = track.getMeasuredStateOnPlaneFromFirstHit()
                    trackPointLast = track.getMeasuredStateOnPlaneFromLastHit()
                    if trackPointFirst.getState() and trackPointLast.getState():
                        infoFirst = geoCache.get(Belle2.VxdID(track.getSortedSVDHitList()[0].getSensorID()))
                        infoLast = geoCache.get(Belle2.VxdID(track.getSortedSVDHitList()[-1].getSensorID()))

                        localFirst = ROOT.TVector3(trackPointFirst.getState()[3], trackPointFirst.getState()[4], 0)
                        localLast = ROOT.TVector3(trackPointLast.getState()[3], trackPointLast.getState()[4], 0)
                        vectorFirst = infoFirst.pointToGlobal(localFirst)
                        vectorLast = infoLast.pointToGlobal(localLast)

                        """
                        inverseTangensOfZenithAngle = math.atan(math.sqrt(math.pow(vectorLast[2]-vectorFirst[2],2)+
                                                                          math.pow(vectorLast[0]-vectorFirst[0],2))/abs(vectorLast[1]-vectorFirst[1]))
                        self.ZenithAngle.Fill(math.degrees(inverseTangensOfZenithAngle))
                        """

                    maximalLadderDifference = [7, 11, 6, 9, 11, 15]
                    sensorsInLayer = [2, 2, 2, 3, 4, 5]
                    storage = []
                    counter = []

                    if track.hasSVDHits():
                        nSvdHits = track.getNumberOfSVDHits()
                        for n in range(0, len(track.getSortedSVDHitList())):
                            svdHit = track.getSortedSVDHitList()[n]
                            sensorID = Belle2.VxdID(svdHit.getSensorID())
                            info = geoCache.get(sensorID)
                            layer = sensorID.getLayerNumber()
                            ladder = sensorID.getLadderNumber()
                            sensor = sensorID.getSensorNumber()

                            for recoInfoSVD in svdHit.getRelationsWith('RecoHitInformations'):
                                if track.getCreatedTrackPoint(recoInfoSVD):
                                    trackPointSVD = track.getCreatedTrackPoint(recoInfoSVD)
                                    if trackPointSVD.getFitterInfo():
                                        if trackPointSVD.getFitterInfo().getFittedState():
                                            stateSVD = trackPointSVD.getFitterInfo().getFittedState().getState()
                                            residualSVD = trackPointSVD.getFitterInfo().getResidual(0, False).getState()
                                            if svdHit.isUCluster():
                                                isU = svdHit.isUCluster()
                                                position = svdHit.getPosition()
                                                slope = stateSVD[1]
                                                trackPoint = stateSVD[3]
                                                residual = residualSVD[0] * Belle2.Unit.convertValueToUnit(1.0, "um")
                                                self.ResidualU.Fill(residual)
                                            else:
                                                isU = svdHit.isUCluster()
                                                position = svdHit.getPosition()
                                                slope = stateSVD[2]
                                                trackPoint = stateSVD[4]
                                                residual = residualSVD[0] * Belle2.Unit.convertValueToUnit(1.0, "um")
                                                self.ResidualV.Fill(residual)

                            self.LayerVsLadder.Fill(layer - 0.5, ladder - 0.5)
                            self.LadderVsSensor.Fill(ladder - 0.5, sum(sensorsInLayer[i]
                                                                       for i in range(0, layer - 1)) + sensor - 0.5)
                            self.LayerVsSensor.Fill(layer - 0.5, sensor - 0.5)

                            if n == 0:
                                counter.append([])
                                counter[0].append(sensorID)
                                counter[0].append(1)
                            elif counter[-1][0] == sensorID:
                                counter[-1][1] += 1
                            else:
                                counter.append([])
                                counter[-1].append(sensorID)
                                counter[-1].append(1)

                            if n == 0:
                                number = 0
                            elif storage[-1][0] == sensorID:
                                number += 1
                            else:
                                number = 0

                            storage.append([])
                            storage[-1].append(sensorID)
                            storage[-1].append(number)
                            storage[-1].append(isU)
                            storage[-1].append(position)
                            storage[-1].append(slope)
                            storage[-1].append(trackPoint)
                            storage[-1].append(residual)

                    if track.hasPXDHits():
                        nPxdHits = track.getNumberOfPXDHits()
                        for n in range(0, len(track.getSortedPXDHitList())):
                            pxdHit = track.getSortedPXDHitList()[n]
                            sensorID = Belle2.VxdID(pxdHit.getSensorID())
                            info = geoCache.get(sensorID)
                            layer = sensorID.getLayerNumber()
                            ladder = sensorID.getLadderNumber()
                            sensor = sensorID.getSensorNumber()
                            for recoInfoPXD in pxdHit.getRelationsWith('RecoHitInformations'):
                                if track.getCreatedTrackPoint(recoInfoPXD):
                                    trackPointPXD = track.getCreatedTrackPoint(recoInfoPXD)
                                    if trackPointPXD.getFitterInfo():
                                        if trackPointPXD.getFitterInfo().getFittedState():
                                            pxdFitterInfo = trackPointPXD.getFitterInfo()
                                            statePXD = trackPointPXD.getFitterInfo().getFittedState().getState()
                                            residualPXD = pxdFitterInfo.getResidual(0, False).getState().GetMatrixArray()

                                            positionU = pxdHit.getU()
                                            slopeU = statePXD[1]
                                            trackPointU = statePXD[3]
                                            residualU = residualPXD[0] * Belle2.Unit.convertValueToUnit(1.0, "um")
                                            self.ResidualU.Fill(residualU)

                                            positionV = pxdHit.getV()
                                            slopeV = statePXD[2]
                                            trackPointV = statePXD[4]
                                            residualV = residualPXD[1] * Belle2.Unit.convertValueToUnit(1.0, "um")
                                            self.ResidualV.Fill(residualV)

                            self.LayerVsLadder.Fill(layer - 0.5, ladder - 0.5)
                            self.LadderVsSensor.Fill(ladder - 0.5, sum(sensorsInLayer[i]
                                                                       for i in range(0, layer - 1)) + sensor - 0.5)
                            self.LayerVsSensor.Fill(layer - 0.5, sensor - 0.5)
                            if not counter:
                                counter.append([])
                                counter[0].append(sensorID)
                                counter[0].append(2)
                            elif counter[-1][0] == sensorID:
                                counter[-1][1] += 2
                            else:
                                counter.append([])
                                counter[-1].append(sensorID)
                                counter[-1].append(2)

                            storage.append([])
                            storage[-1].append(sensorID)
                            storage[-1].append(0)
                            storage[-1].append(True)
                            storage[-1].append(positionU)
                            storage[-1].append(slopeU)
                            storage[-1].append(trackPointU)
                            storage[-1].append(residualU)

                            storage.append([])
                            storage[-1].append(sensorID)
                            storage[-1].append(1)
                            storage[-1].append(False)
                            storage[-1].append(positionV)
                            storage[-1].append(slopeV)
                            storage[-1].append(trackPointV)
                            storage[-1].append(residualV)
                    """
                    print (end="\n")
                    for row in counter:
                        for column in row:
                            print (column, end=" ")
                        print(end="\n")
                    print (end="\n")

                    print (end="\n")
                    for row in storage:
                        for column in row:
                            print (column, end=" ")
                        print(end="\n")
                    print (end="\n")
                    """
                    self.NumberOfSensorsPerTrack.Fill(len(counter))
                    layerCounts = [0, 0, 0, 0, 0, 0, 0]
                    for row in counter:
                        layerCounts[row[0].getLayerNumber()] += 1
                        self.NumberOfClustersPerSensor.Fill(row[1])
                    for n in range(1, 7):
                        self.HitsInLayer.Fill(n - 0.5, layerCounts[n] - 0.5)

                    for a, b, c, d in itertools.combinations(storage, 4):
                        if str(Belle2.VxdID(a[0])) == str(Belle2.VxdID(b[0])):
                            if str(Belle2.VxdID(c[0])) == str(Belle2.VxdID(d[0])):
                                if Belle2.VxdID(a[0]).getLayerNumber() == Belle2.VxdID(c[0]).getLayerNumber():
                                    differenceInLadders = abs(
                                        Belle2.VxdID(
                                            a[0]).getLadderNumber() -
                                        Belle2.VxdID(
                                            c[0]).getLadderNumber())
                                    if differenceInLadders == 1 or differenceInLadders == maximalLadderDifference[Belle2.VxdID(
                                            a[0]).getLayerNumber() - 1]:
                                        """
                                        print('An overlap was founded!!!')
                                        print ("Before selection overlaps:", end="\n")
                                        #for row in [a,b,c,d]:
                                        for row in storage:
                                            for column in row:
                                                print (column, end=" ")
                                            print(end="\n")
                                        print(end="\n")
                                        """
                                        if a[2]:
                                            if not b[2]:
                                                self.overlapsData.firstLayer = Belle2.VxdID(a[0]).getLayerNumber()
                                                self.overlapsData.firstLadder = Belle2.VxdID(a[0]).getLadderNumber()
                                                self.overlapsData.firstSensor = Belle2.VxdID(a[0]).getSensorNumber()
                                                self.overlapsData.firstUHit = a[3]
                                                self.overlapsData.firstUSlope = a[4]
                                                self.overlapsData.firstUExpected = a[5]
                                                self.overlapsData.firstUResidual = a[6]
                                                self.overlapsData.firstVHit = b[3]
                                                self.overlapsData.firstVSlope = b[4]
                                                self.overlapsData.firstVExpected = b[5]
                                                self.overlapsData.firstVResidual = b[6]

                                        elif b[2]:
                                            if not a[2]:
                                                self.overlapsData.firstLayer = Belle2.VxdID(b[0]).getLayerNumber()
                                                self.overlapsData.firstLadder = Belle2.VxdID(b[0]).getLadderNumber()
                                                self.overlapsData.firstSensor = Belle2.VxdID(b[0]).getSensorNumber()
                                                self.overlapsData.firstUHit = b[3]
                                                self.overlapsData.firstUSlope = b[4]
                                                self.overlapsData.firstUExpected = b[5]
                                                self.overlapsData.firstUResidual = b[6]
                                                self.overlapsData.firstVHit = a[3]
                                                self.overlapsData.firstVSlope = a[4]
                                                self.overlapsData.firstVExpected = a[5]
                                                self.overlapsData.firstVResidual = a[6]

                                        if c[2]:
                                            if not d[2]:
                                                self.overlapsData.secondLayer = Belle2.VxdID(c[0]).getLayerNumber()
                                                self.overlapsData.secondLadder = Belle2.VxdID(c[0]).getLadderNumber()
                                                self.overlapsData.secondSensor = Belle2.VxdID(c[0]).getSensorNumber()
                                                self.overlapsData.secondUHit = c[3]
                                                self.overlapsData.secondUSlope = c[4]
                                                self.overlapsData.secondUExpected = c[5]
                                                self.overlapsData.secondUResidual = c[6]
                                                self.overlapsData.secondVHit = d[3]
                                                self.overlapsData.secondVSlope = d[4]
                                                self.overlapsData.secondVExpected = d[5]
                                                self.overlapsData.secondVResidual = d[6]

                                        elif d[2]:
                                            if not c[2]:
                                                self.overlapsData.secondLayer = Belle2.VxdID(d[0]).getLayerNumber()
                                                self.overlapsData.secondLadder = Belle2.VxdID(d[0]).getLadderNumber()
                                                self.overlapsData.secondSensor = Belle2.VxdID(d[0]).getSensorNumber()
                                                self.overlapsData.secondUHit = d[3]
                                                self.overlapsData.secondUSlope = d[4]
                                                self.overlapsData.secondUExpected = d[5]
                                                self.overlapsData.secondUResidual = d[6]
                                                self.overlapsData.secondVHit = c[3]
                                                self.overlapsData.secondVSlope = c[4]
                                                self.overlapsData.secondVExpected = c[5]
                                                self.overlapsData.secondVResidual = c[6]

                                        self.rootfile.cd()
                                        self.tree_overlaps.Fill()
                                        if a in storage:
                                            del storage[storage.index(a)]
                                        if b in storage:
                                            del storage[storage.index(b)]
                                        if c in storage:
                                            del storage[storage.index(c)]
                                        if d in storage:
                                            del storage[storage.index(d)]
                                        """
                                        print ("After selection overlaps:", end="\n")
                                        for row in storage:
                                            for column in row:
                                                print (column, end=" ")
                                            print(end="\n")
                                        print (end="\n")
                                        """

                    for a, b in itertools.combinations(storage, 2):
                        if str(Belle2.VxdID(a[0])) == str(Belle2.VxdID(b[0])):
                            if a[2]:
                                if not b[2]:
                                    self.otherResidualsData.layer = Belle2.VxdID(a[0]).getLayerNumber()
                                    self.otherResidualsData.ladder = Belle2.VxdID(a[0]).getLadderNumber()
                                    self.otherResidualsData.sensor = Belle2.VxdID(a[0]).getSensorNumber()
                                    self.otherResidualsData.uHit = a[3]
                                    self.otherResidualsData.uSlope = a[4]
                                    self.otherResidualsData.uExpected = a[5]
                                    self.otherResidualsData.uResidual = a[6]
                                    self.otherResidualsData.vHit = b[3]
                                    self.otherResidualsData.vSlope = b[4]
                                    self.otherResidualsData.vExpected = b[5]
                                    self.otherResidualsData.vResidual = b[6]

                            elif b[2]:
                                if not a[2]:
                                    self.otherResidualsData.layer = Belle2.VxdID(b[0]).getLayerNumber()
                                    self.otherResidualsData.ladder = Belle2.VxdID(b[0]).getLadderNumber()
                                    self.otherResidualsData.sensor = Belle2.VxdID(b[0]).getSensorNumber()
                                    self.otherResidualsData.uHit = b[3]
                                    self.otherResidualsData.uSlope = b[4]
                                    self.otherResidualsData.uExpected = b[5]
                                    self.otherResidualsData.uResidual = b[6]
                                    self.otherResidualsData.vHit = a[3]
                                    self.otherResidualsData.vSlope = a[4]
                                    self.otherResidualsData.vExpected = a[5]
                                    self.otherResidualsData.vResidual = a[6]

                            self.rootfile.cd()
                            self.tree_residuals.Fill()

    def terminate(self):

        time_diff = self.last_time - self.first_time
        print("Number of tracks: ", self.numberTracks)
        print("Number of events: ", self.numberEvents)
        print("Time in seconds: ", time_diff / 1e9)
        print("#RecoTracks/s: ", self.numberTracks / (time_diff / 1e9))
        print("#Events/s: ", self.numberEvents / (time_diff / 1e9))

        """Close & write output files"""
        self.rootfile.cd()

        labelLayers = ('', '1.*.*', '2.*.*', '3.*.*', '4.*.*', '5.*.*', '6.*.*')
        labelLadders = (
            '',
            '*.1.*',
            '*.2.*',
            '*.3.*',
            '*.4.*',
            '*.5.*',
            '*.6.*',
            '*.7.*',
            '*.8.*',
            '*.9.*',
            '*.10.*',
            '*.11.*',
            '*.12.*',
            '*.13.*',
            '*.14.*',
            '*.15.*',
            '*.16.*')
        labelSensorsInLayers = (
            '',
            '1.*.1',
            '1.*.2',
            '2.*.1',
            '2.*.2',
            '3.*.1',
            '3.*.2',
            '4.*.1',
            '4.*.2',
            '4.*.3',
            '5.*.1',
            '5.*.2',
            '5.*.3',
            '5.*.4',
            '6.*.1',
            '6.*.2',
            '6.*.3',
            '6.*.4',
            '6.*.5')
        labelSensors = ('', '*.*.1', '*.*.2', '*.*.3', '*.*.4', '*.*.5')

        self.TotalNumberOfHits.GetXaxis().SetLabelSize(0.05)
        self.TotalNumberOfHits.GetYaxis().SetTitle("Number of tracks")
        self.TotalNumberOfHits.GetXaxis().SetTitle("Number of VXD clusters in track")
        self.TotalNumberOfHits.GetYaxis().CenterTitle()
        self.TotalNumberOfHits.GetXaxis().CenterTitle()
        self.TotalNumberOfHits.GetYaxis().SetTitleOffset(1.3)
        self.TotalNumberOfHits.GetXaxis().SetTitleOffset(1.3)
        self.TotalNumberOfHits.SetFillStyle(3365)
        self.TotalNumberOfHits.SetFillColor(9)
        self.TotalNumberOfHits.SetLineColor(9)
        self.TotalNumberOfHits.Draw()

        self.NumberTracks.GetXaxis().SetTitle("Number of Tracks")
        self.NumberTracks.GetYaxis().SetTitle("Number of events")
        self.NumberTracks.GetYaxis().CenterTitle()
        self.NumberTracks.GetXaxis().CenterTitle()
        self.NumberTracks.GetYaxis().SetTitleOffset(1.3)
        self.NumberTracks.GetXaxis().SetTitleOffset(1.3)
        self.NumberTracks.SetFillStyle(3365)
        self.NumberTracks.SetFillColor(9)
        self.NumberTracks.SetLineColor(9)
        self.NumberTracks.Draw()

        self.NumberTracksInPXD.GetXaxis().SetBinLabel(1, "PXD+SVD")
        self.NumberTracksInPXD.GetXaxis().SetBinLabel(2, "SVD only")
        self.NumberTracksInPXD.GetXaxis().SetTitle("Tracks in detectors")
        self.NumberTracksInPXD.GetYaxis().SetTitle("Number of Tracks")
        self.NumberTracksInPXD.GetYaxis().CenterTitle()
        self.NumberTracksInPXD.GetXaxis().CenterTitle()
        self.NumberTracksInPXD.GetYaxis().SetTitleOffset(1.3)
        self.NumberTracksInPXD.GetXaxis().SetTitleOffset(1.3)
        self.NumberTracksInPXD.SetFillStyle(3365)
        self.NumberTracksInPXD.SetFillColor(9)
        self.NumberTracksInPXD.SetLineColor(9)
        self.NumberTracksInPXD.Draw()

        for i in range(1, 7):
            self.HitsInLayer.GetXaxis().SetBinLabel(i, labelLayers[i])
        self.HitsInLayer.GetXaxis().SetLabelSize(0.05)
        self.HitsInLayer.GetYaxis().SetLabelSize(0.05)
        self.HitsInLayer.GetXaxis().SetTitle("Layer")
        self.HitsInLayer.GetYaxis().SetTitle("Number hit in layer")
        self.HitsInLayer.GetYaxis().CenterTitle()
        self.HitsInLayer.GetXaxis().CenterTitle()
        self.HitsInLayer.GetYaxis().SetTitleOffset(1.3)
        self.HitsInLayer.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.HitsInLayer.Draw()
        ROOT.gPad.Update()
        self.HitsInLayer.FindObject("stats").SetOptStat(0)

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
        self.LayerVsSensor.GetYaxis().SetTitleOffset(1.3)
        self.LayerVsSensor.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.LayerVsSensor.Draw()
        ROOT.gPad.Update()
        self.LayerVsSensor.FindObject("stats").SetOptStat(0)

        for i in range(1, 7):
            self.LayerVsLadder.GetXaxis().SetBinLabel(i, labelLayers[i])
        for i in range(1, 17):
            self.LayerVsLadder.GetYaxis().SetBinLabel(i, labelLadders[i])
        self.LayerVsLadder.GetXaxis().SetLabelSize(0.05)
        self.LayerVsLadder.GetYaxis().SetLabelSize(0.05)
        self.LayerVsLadder.GetXaxis().SetTitle("Layer")
        self.LayerVsLadder.GetYaxis().SetTitle("Ladder")
        self.LayerVsLadder.GetYaxis().CenterTitle()
        self.LayerVsLadder.GetXaxis().CenterTitle()
        self.LayerVsLadder.GetYaxis().SetTitleOffset(1.3)
        self.LayerVsLadder.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.LayerVsLadder.Draw()
        ROOT.gPad.Update()
        self.LayerVsLadder.FindObject("stats").SetOptStat(0)

        for i in range(1, 17):
            self.LadderVsSensor.GetXaxis().SetBinLabel(i, labelLadders[i])

        for i in range(1, 19):
            self.LadderVsSensor.GetYaxis().SetBinLabel(i, labelSensorsInLayers[i])

        self.LadderVsSensor.GetXaxis().SetLabelSize(0.05)
        self.LadderVsSensor.GetYaxis().SetLabelSize(0.05)
        self.LadderVsSensor.GetXaxis().SetTitle("Ladder")
        self.LadderVsSensor.GetYaxis().SetTitle("Sensors in layers")
        self.LadderVsSensor.GetXaxis().SetTicks("U")
        self.LadderVsSensor.GetYaxis().SetTicks("U")
        self.LadderVsSensor.GetYaxis().CenterTitle()
        self.LadderVsSensor.GetXaxis().CenterTitle()
        self.LadderVsSensor.GetYaxis().SetTitleOffset(1.3)
        self.LadderVsSensor.GetXaxis().SetTitleOffset(1.3)
        ROOT.gPad.SetMargin(0.11, 0.11, 0.1, 0.1)
        self.LadderVsSensor.Draw()
        ROOT.gPad.Update()
        self.LadderVsSensor.FindObject("stats").SetOptStat(0)

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

        self.Chi2OverNDF.GetXaxis().SetTitle("#Chi^{2}/Degrees of freedom")
        self.Chi2OverNDF.GetYaxis().SetTitle("Number of tracks")
        self.Chi2OverNDF.GetYaxis().CenterTitle()
        self.Chi2OverNDF.GetXaxis().CenterTitle()
        self.Chi2OverNDF.GetYaxis().SetTitleOffset(1.3)
        self.Chi2OverNDF.GetXaxis().SetTitleOffset(1.3)
        self.Chi2OverNDF.SetFillStyle(3365)
        self.Chi2OverNDF.SetFillColor(9)
        self.Chi2OverNDF.SetLineColor(9)
        self.Chi2OverNDF.Draw()

        self.PValue.GetXaxis().SetTitle("P value")
        self.PValue.GetYaxis().SetTitle("Number of tracks")
        self.PValue.GetYaxis().CenterTitle()
        self.PValue.GetXaxis().CenterTitle()
        self.PValue.GetYaxis().SetTitleOffset(1.3)
        self.PValue.GetXaxis().SetTitleOffset(1.3)
        self.PValue.SetFillStyle(3365)
        self.PValue.SetFillColor(9)
        self.PValue.SetLineColor(9)
        self.PValue.Draw()

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

        self.NumberOfClustersPerSensor.GetXaxis().SetTitle("Number of SVD clusters in one sensor")
        self.NumberOfClustersPerSensor.GetYaxis().SetTitle("Number of entries")
        self.NumberOfClustersPerSensor.GetYaxis().CenterTitle()
        self.NumberOfClustersPerSensor.GetXaxis().CenterTitle()
        self.NumberOfClustersPerSensor.GetYaxis().SetTitleOffset(1.3)
        self.NumberOfClustersPerSensor.GetXaxis().SetTitleOffset(1.3)
        self.NumberOfClustersPerSensor.SetFillStyle(3365)
        self.NumberOfClustersPerSensor.SetFillColor(9)
        self.NumberOfClustersPerSensor.SetLineColor(9)
        self.NumberOfClustersPerSensor.Draw()

        self.NumberOfSensorsPerTrack.GetXaxis().SetTitle("Number of Space points in track")
        self.NumberOfSensorsPerTrack.GetYaxis().SetTitle("Number of tracks")
        self.NumberOfSensorsPerTrack.GetYaxis().CenterTitle()
        self.NumberOfSensorsPerTrack.GetXaxis().CenterTitle()
        self.NumberOfSensorsPerTrack.GetYaxis().SetTitleOffset(1.3)
        self.NumberOfSensorsPerTrack.GetXaxis().SetTitleOffset(1.3)
        self.NumberOfSensorsPerTrack.SetFillStyle(3365)
        self.NumberOfSensorsPerTrack.SetFillColor(9)
        self.NumberOfSensorsPerTrack.SetLineColor(9)
        self.NumberOfSensorsPerTrack.Draw()

        self.ResidualU.GetXaxis().SetTitle("Residual in U direction")
        self.ResidualU.GetYaxis().SetTitle("Counts")
        self.ResidualU.GetXaxis().CenterTitle()
        self.ResidualU.GetYaxis().CenterTitle()
        self.ResidualU.GetYaxis().SetTitleOffset(1.3)
        self.ResidualU.GetXaxis().SetTitleOffset(1.3)
        self.ResidualU.SetFillStyle(3365)
        self.ResidualU.SetLineColor(9)
        self.ResidualU.SetFillColor(9)
        self.ResidualU.Draw()

        self.ResidualV.GetXaxis().SetTitle("Residual in V direction")
        self.ResidualV.GetYaxis().SetTitle("Counts")
        self.ResidualV.GetXaxis().CenterTitle()
        self.ResidualV.GetYaxis().CenterTitle()
        self.ResidualV.GetYaxis().SetTitleOffset(1.3)
        self.ResidualV.GetXaxis().SetTitleOffset(1.3)
        self.ResidualV.SetFillStyle(3365)
        self.ResidualV.SetLineColor(9)
        self.ResidualV.SetFillColor(9)
        self.ResidualV.Draw()

        self.ZenithAngle.GetXaxis().SetTitle("Zenith angle [#circ]")
        self.ZenithAngle.GetYaxis().SetTitle("Counts")
        self.ZenithAngle.GetXaxis().CenterTitle()
        self.ZenithAngle.GetYaxis().CenterTitle()
        self.ZenithAngle.GetYaxis().SetTitleOffset(1.3)
        self.ZenithAngle.GetXaxis().SetTitleOffset(1.3)
        self.ZenithAngle.SetFillStyle(3365)
        self.ZenithAngle.SetLineColor(9)
        self.ZenithAngle.SetFillColor(9)
        self.ZenithAngle.Draw()

        self.rootfile.Write()
        self.rootfile.Close()
