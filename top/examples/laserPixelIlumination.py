#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH2F, TFile

# ------------------------------------------------------------------------
# example of making histograms of pixel hits coming from individual fibers
# needs as input the file produced by top/tools/simLaserCalibration.py
# ------------------------------------------------------------------------

inputFile = 'laserSimulation.root'
if not os.path.exists(inputFile):
    print(inputFile + ': file not found')
    sys.exit()


class Histogrammer(Module):

    ''' A module to histogram pixel hits from individual fibers'''

    #: 2D histograms
    hist = [TH2F('fiber' + str(k + 1), 'Pixel hit distribution from fiber No.' + str(k + 1),
                 64, 0.5, 64.5, 8, 0.5, 8.5) for k in range(9)]
    #: pixel ID
    pixelID = 482
    #: histogram
    propTime = TH2F('propTime', 'Photon propagation times for pixel ' + str(pixelID) +
                    ' vs. fiber number', 9, 0.5, 9.5, 200, 0.0, 1.0)
    propTime.GetXaxis().SetTitle('fiber number')
    propTime.GetYaxis().SetTitle('propagation time [ns]')

    def event(self):
        ''' Event processor: fill histograms '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            simhits = digit.getRelationsWith('TOPSimHits')  # several simhits can contribute
            for simhit in simhits:
                photon = simhit.getRelated('TOPSimPhotons')  # one or none possible
                if photon:
                    k = int((photon.getEmissionPoint().X() + 22.5) / 5.5)
                    if k >= 0 and k < 9:
                        self.hist[k].Fill(digit.getPixelCol(), digit.getPixelRow())
                    else:
                        B2ERROR('wrong decoding of fiber number: ' + str(k + 1))
                    if digit.getPixelID() == self.pixelID:
                        t = photon.getDetectionTime() - photon.getEmissionTime()
                        self.propTime.Fill(k + 1, t)

    def terminate(self):
        ''' Write histograms to file '''

        tfile = TFile('laserPixelIlumination.root', 'recreate')
        for k in range(9):
            self.hist[k].Write()
        self.propTime.Write()
        tfile.Close()


# Create path
main = create_path()

# Input
roinput = register_module('RootInput')
roinput.param('inputFileName', inputFile)
main.add_module(roinput)

# Histogrammer
main.add_module(Histogrammer())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
