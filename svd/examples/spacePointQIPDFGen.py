#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2


class spacePointQIPDFGen(Module):

    """A module to generate PDF histograms from charge/size/time u/v SVD cluster combinations."""

    def __init__(self):
        """Initialize module"""
        super(spacePointQIPDFGen, self).__init__()

        self.Hist = ROOT.TFile('spacePointQIPDFs.root', 'RECREATE')

        self.signalHist = {}
        self.bkgHist = {}
        self.sensors = ['l3', 'trap', 'large']
        self.clusterMaxSize = 5
        for i in self.sensors:
            for j in range(1, self.clusterMaxSize + 1):
                for k in range(1, self.clusterMaxSize + 1):
                    signalName = 'signal' + str(i) + str(j) + str(k)
                    bkgName = 'bkg' + str(i) + str(j) + str(k)
                    self.signalHist[signalName] = ROOT.TH2F(signalName, "", 50, 0, 200000, 50, 0, 200000)
                    self.bkgHist[bkgName] = ROOT.TH2F(bkgName, "", 50, 0, 200000, 50, 0, 200000)

        self.signalSize = ROOT.TH2F('signalSize', '', 20, 0, 20, 20, 0, 20)
        self.bkgSize = ROOT.TH2F('bkgSize', '', 20, 0, 20, 20, 0, 20)
        self.signalTime = ROOT.TH2F('signalTime', '', 20, -100, 100, 20, -100, 100)
        self.bkgTime = ROOT.TH2F('bkgTime', '', 20, -100, 100, 20, -100, 100)

    def beginRun(self):
        """Empty"""

    def event(self):
        """Calculate probability of pairing signal clusters."""

        cluster = Belle2.PyStoreArray('SVDClusters')

        for clusters in cluster:
            if clusters.isUCluster() == 1:  # u cluster
                uHit = clusters.getRelatedTo('SVDTrueHits')
                for vClusters in cluster:
                    if vClusters.isUCluster() == 0:
                        vHit = vClusters.getRelatedTo('SVDTrueHits')
                        uSensor = clusters.getSensorID()
                        vSensor = vClusters.getSensorID()
                        if uSensor == vSensor:
                            if hasattr(uHit, 'getEnergyDep') and hasattr(vHit, 'getEnergyDep'):
                                uCharge = uHit.getEnergyDep()
                                vCharge = vHit.getEnergyDep()
                                uSim = uHit.getRelatedTo('SVDSimHits')
                                vSim = vHit.getRelatedTo('SVDSimHits')
                                uMC = uSim.getRelatedFrom('MCParticles')
                                vMC = vSim.getRelatedFrom('MCParticles')
                                """Remove secondary hits"""
                                if uMC.isPrimaryParticle() == 0 or vMC.isPrimaryParticle() == 0:
                                    continue

                                if uCharge == vCharge:
                                    uSize = clusters.getSize()
                                    vSize = vClusters.getSize()
                                    uChar = clusters.getCharge()
                                    vChar = vClusters.getCharge()
                                    uTime = clusters.getClsTime()
                                    vTime = vClusters.getClsTime()

                                    self.signalTime.Fill(uTime, vTime)
                                    self.signalSize.Fill(uSize, vSize)

                                    if uSize > self.clusterMaxSize:
                                        uSize = self.clusterMaxSize
                                    if vSize > self.clusterMaxSize:
                                        vSize = self.clusterMaxSize

                                    sensor = ''
                                    if uSensor.getLayerNumber() == 3:
                                        sensor = 'l3'
                                    if uSensor.getLayerNumber() != 3 and uSensor.getSensorNumber() == 1:
                                        sensor = 'trap'
                                    if uSensor.getLayerNumber() != 3 and uSensor.getSensorNumber() != 1:
                                        sensor = 'large'

                                    self.signalHist['signal' + sensor + str(uSize) + str(vSize)].Fill(uChar, vChar)

                            else:  # background hit
                                uSize = clusters.getSize()
                                vSize = vClusters.getSize()
                                uChar = clusters.getCharge()
                                vChar = vClusters.getCharge()
                                uTime = clusters.getClsTime()
                                vTime = vClusters.getClsTime()

                                self.bkgTime.Fill(uTime, vTime)
                                self.bkgSize.Fill(uSize, vSize)

                                if uSize > self.clusterMaxSize:
                                    uSize = self.clusterMaxSize
                                if vSize > self.clusterMaxSize:
                                    vSize = self.clusterMaxSize

                                    sensor = ''
                                if uSensor.getLayerNumber() == 3:
                                    sensor = 'l3'
                                if uSensor.getLayerNumber() != 3 and uSensor.getSensorNumber() == 1:
                                    sensor = 'trap'
                                if uSensor.getLayerNumber() != 3 and uSensor.getSensorNumber() != 1:
                                    sensor = 'large'

                                self.bkgHist['bkg' + sensor + str(uSize) + str(vSize)].Fill(uChar, vChar)

    def terminate(self):
        """Close output."""

        self.Hist.cd()

        # For cloning dimensions
        blankCharge = ROOT.TH2F('blankCharge', '', 50, 0, 200000, 50, 0, 200000)
        blankTime = ROOT.TH2F('blankTime', '', 20, -100, 100, 20, -100, 100)
        blankSize = ROOT.TH2F('blankSize', '', 20, 0, 20, 20, 0, 20)

        def calcProb(signal, background, signalHist):
            """Calculate the probability of a given bin being occupied by signal hit"""
            tempHist = signal.Clone("tempHist")
            tempHist.Add(background)
            signalHist.Add(signal)
            signalHist.Divide(tempHist)

        def calculateError(signal, background, errorHist):
            """Calculate the binning error"""
            imax = signal.GetXaxis().GetNbins()
            jmax = signal.GetYaxis().GetNbins()
            for i in range(1, imax + 1):
                for j in range(1, jmax + 1):
                    bkg = background.GetBinContent(i, j)
                    sig = signal.GetBinContent(i, j)
                    var = ((sig + 1) * (sig + 2)) / ((sig + bkg + 2) * (sig + bkg + 3)) - \
                        ((sig + 1) * (sig + 1)) / ((sig + bkg + 2) * (sig + bkg + 2))
                    err = math.sqrt(var)
                    errorHist.SetBinContent(i, j, err)

        sensorType = ''
        geometry = Belle2.VXD.GeoCache.getInstance()
        layers = geometry.getLayers(Belle2.VXD.SensorInfoBase.SVD)
        for layer in layers:
            ladders = geometry.getLadders(layer)
            for ladder in ladders:
                sensors = geometry.getSensors(ladder)
                for sensor in sensors:
                    for uClusSize in range(1, self.clusterMaxSize + 1):
                        for vClusSize in range(1, self.clusterMaxSize + 1):
                            if sensor.getLayerNumber() == 3:
                                sensorType = 'l3'
                            if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() == 1:
                                sensorType = 'trap'
                            if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() != 1:
                                sensorType = 'large'
                            signalName = 'signal' + sensorType + str(uClusSize) + str(vClusSize)
                            bkgName = 'bkg' + sensorType + str(uClusSize) + str(vClusSize)
                            probHist = blankCharge.Clone(str(sensor) + '.' + str(uClusSize) + '.' + str(vClusSize))
                            errorHist = blankCharge.Clone(str(sensor) + '.' + str(uClusSize) + '.' + str(vClusSize) + '_Error')

                            calcProb(self.signalHist[signalName], self.bkgHist[bkgName], probHist)
                            probHist.Write()
                            calculateError(self.signalHist[signalName], self.bkgHist[bkgName], errorHist)
                            errorHist.Write()

        timeProb = blankTime.Clone("timeProb")
        timeError = blankTime.Clone("timeError")
        calcProb(self.signalTime, self.bkgTime, timeProb)
        timeProb.Write()
        calculateError(self.signalTime, self.bkgTime, timeError)
        timeError.Write()

        sizeProb = blankSize.Clone("sizeProb")
        sizeError = blankSize.Clone("sizeError")
        calcProb(self.signalSize, self.bkgSize, sizeProb)
        sizeProb.Write()
        calculateError(self.signalSize, self.bkgSize, sizeError)
        sizeError.Write()

        self.Hist.Close()
