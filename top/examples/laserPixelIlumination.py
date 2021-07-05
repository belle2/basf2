#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import os
import sys
from ROOT import Belle2
from ROOT import TH2F, TFile

# ------------------------------------------------------------------------
# example of making histograms of pixel hits coming from individual fibers
# needs as input the file produced by top/analysis/simLaserCalibration.py
# ------------------------------------------------------------------------

inputFile = 'laserSimulation.root'
if not os.path.exists(inputFile):
    b2.B2ERROR(inputFile + ': file not found')
    b2.B2INFO('File can be generated with top/analysis/simLaserCalibration.py')
    sys.exit()


class Histogrammer(b2.Module):

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
                        b2.B2ERROR('wrong decoding of fiber number: ' + str(k + 1))
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
main = b2.create_path()

# Input
roinput = b2.register_module('RootInput')
roinput.param('inputFileName', inputFile)
main.add_module(roinput)

# Histogrammer
main.add_module(Histogrammer())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
