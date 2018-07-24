#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2

import numpy as np


class clusterQIPDFGen(Module):

    """A module to generate PDF histograms for SVD clusters."""

    def __init__(self):
        """Initialize module"""
        super(clusterQIPDFGen, self).__init__()

        self.Hist = ROOT.TFile('clusterQIPDFs.root', 'RECREATE')

        self.signalHist = {}
        self.bkgHist = {}
        self.sensors = ['l3', 'trap', 'large']
        self.side = ['u', 'v']
        self.clusterMaxSize = 5
        for i in self.sensors:
            for j in range(1, self.clusterMaxSize + 1):
                for k in self.side:
                    signalName = k + 'Signal' + str(i) + str(j)
                    bkgName = k + 'Bkg' + str(i) + str(j)
                    self.signalHist[signalName] = ROOT.TH2F(signalName, "", 40, -100, 100, 100, 0, 200000)
                    self.bkgHist[bkgName] = ROOT.TH2F(bkgName, "", 40, -100, 100, 100, 0, 200000)

    def beginRun(self):
        """Empty"""

    def event(self):
        """Calculate probability of clusters originating for signal hit."""

        clusters = Belle2.PyStoreArray('SVDClusters')

        for cluster in clusters:
            if cluster.isUCluster() == 1:  # u cluster
                uHit = cluster.getRelatedTo('SVDTrueHits')
                clusterCharge = cluster.getCharge()
                clusterTime = cluster.getClsTime()
                clusterSize = cluster.getSize()
                sensor = cluster.getSensorID()
                if hasattr(uHit, 'getEnergyDep'):
                    uSim = uHit.getRelatedTo('SVDSimHits')
                    uMC = uSim.getRelatedFrom('MCParticles')
                    if hasattr(uMC, 'isPrimaryParticle') and uMC.isPrimaryParticle() == 1:

                        if clusterSize > self.clusterMaxSize:
                            clusterSize = self.clusterMaxSize

                        sensorName = ''
                        if sensor.getLayerNumber() == 3:
                            sensorName = 'l3'
                        if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() == 1:
                            sensorName = 'trap'
                        if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() != 1:
                            sensorName = 'large'

                        self.signalHist['uSignal' + sensorName + str(clusterSize)].Fill(clusterTime, clusterCharge)

                else:  # U-side background hits
                    if clusterSize > self.clusterMaxSize:
                        clusterSize = self.clusterMaxSize

                    sensorName = ''
                    if sensor.getLayerNumber() == 3:
                        sensorName = 'l3'
                    if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() == 1:
                        sensorName = 'trap'
                    if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() != 1:
                        sensorName = 'large'

                    self.bkgHist['uBkg' + sensorName + str(clusterSize)].Fill(clusterTime, clusterCharge)

            else:  # v-clusters
                vHit = cluster.getRelatedTo('SVDTrueHits')
                clusterCharge = cluster.getCharge()
                clusterTime = cluster.getClsTime()
                clusterSize = cluster.getSize()
                sensor = cluster.getSensorID()

                if hasattr(vHit, 'getEnergyDep'):
                    vSim = vHit.getRelatedTo('SVDSimHits')
                    vMC = vSim.getRelatedFrom('MCParticles')
                    if hasattr(vMC, 'isPrimaryParticle') and vMC.isPrimaryParticle() == 1:

                        if clusterSize > self.clusterMaxSize:
                            clusterSize = self.clusterMaxSize

                        sensorName = ''
                        if sensor.getLayerNumber() == 3:
                            sensorName = 'l3'
                        if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() == 1:
                            sensorName = 'trap'
                        if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() != 1:
                            sensorName = 'large'

                        self.signalHist['vSignal' + sensorName + str(clusterSize)].Fill(clusterTime, clusterCharge)
                else:  # v-side Background
                    if clusterSize > self.clusterMaxSize:
                        clusterSize = self.clusterMaxSize

                    sensorName = ''
                    if sensor.getLayerNumber() == 3:
                        sensorName = 'l3'
                    if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() == 1:
                        sensorName = 'trap'
                    if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() != 1:
                        sensorName = 'large'

                    self.bkgHist['vBkg' + sensorName + str(clusterSize)].Fill(clusterTime, clusterCharge)

    def terminate(self):
        """Close output."""
        self.Hist.cd()

        # For cloning dimensions
        blankHist = ROOT.TH2F('blankHist', '', 40, -100, 100, 100, 0, 200000)

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
                    for clusSize in range(1, self.clusterMaxSize + 1):
                        for side in self.side:
                            if sensor.getLayerNumber() == 3:
                                sensorType = 'l3'
                            if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() == 1:
                                sensorType = 'trap'
                            if sensor.getLayerNumber() != 3 and sensor.getSensorNumber() != 1:
                                sensorType = 'large'
                            signalName = side + 'Signal' + sensorType + str(clusSize)
                            bkgName = side + 'Bkg' + sensorType + str(clusSize)
                            probHist = blankHist.Clone(str(sensor) + '.' + str(side) + '.' + str(clusSize))
                            errorHist = blankHist.Clone(str(sensor) + '.' + str(side) + '.' + str(clusSize) + '_Error')

                            calcProb(self.signalHist[signalName], self.bkgHist[bkgName], probHist)
                            probHist.Write()
                            calculateError(self.signalHist[signalName], self.bkgHist[bkgName], errorHist)
                            errorHist.Write()

        self.Hist.Close()
