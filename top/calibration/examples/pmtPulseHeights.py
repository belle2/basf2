#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
from ROOT import Belle2, TFile, TCanvas, gDirectory, TH1F, gROOT
import time

# ---------------------------------------------------------------------------------------------------------
# Extract pulse height distributions from the payload TOPCalPhotonYields and save canvases into a root file
#
# Usage: basf2 pmtPulseHeights.py expNo runNo [globalTag/localDB]
#        (default globalTag = data_reprocessing_prompt)
# ---------------------------------------------------------------------------------------------------------

if len(sys.argv) < 3:
    print("usage: basf2", sys.argv[0], "expNo runNo [globalTag/localDB]")
    sys.exit()

expNo = int(sys.argv[1])
runNo = int(sys.argv[2])
globalTag = "data_reprocessing_prompt"
if len(sys.argv) > 3:
    globalTag = sys.argv[3]


class SaveCanvases(b2.Module):
    ''' Extract pulse height distributions from TOPCalPhotonYields and save canvases into a root file '''

    def makePlot(self, h, slot, pmt):
        '''
        Makes a plot of pulse-height distributions of pixels of a single PMT and saves the canvas
        :param h: a 2D histogram of pulse-height vs. pixel
        :param slot: slot number
        :param pmt: PMT number
        '''

        name = 's' + f'{slot:02d}' + '_pmt' + f'{pmt:02d}'
        canvas = TCanvas(name, "slot " + str(slot) + " pmt " + str(pmt), 1000, 1000)
        canvas.Divide(4, 4)
        meanPH = 0
        N = 0
        for pmtPix in range(16):
            col = ((pmt - 1) % 16) * 4 + pmtPix % 4
            row = int((pmt - 1) / 16) * 4 + int(pmtPix / 4)
            pix = row * 64 + col + 1
            py = h.ProjectionY("px_" + str(slot) + "_" + str(pix), pix, pix)
            py.SetTitle("PMT pixel " + str(pmtPix + 1))
            irow = int(pmtPix / 4)
            icol = pmtPix % 4
            canvas.cd(icol + (3 - irow) * 4 + 1)
            py.Draw()
            if py.GetEntries() > 0:
                n = py.Integral()
                meanPH += py.GetMean() * n
                N += n
        canvas.Write()
        if N > 0:
            meanPH /= N
        return meanPH

    def initialize(self):
        ''' initialize: implementation '''

        gROOT.SetBatch(True)

        db = Belle2.PyDBObj("TOPCalPhotonYields")
        if not db:
            return

        fileName = "pmtPulseHeights-" + time.strftime("%Y-%m-%d", time.localtime(db.getTimeStamp())) + ".root"
        f = TFile.Open(fileName, "recreate")

        print("- global tag:", globalTag)
        print("- time of measurement:",
              time.strftime("%d %b %Y %H:%M:%S", time.localtime(db.getTimeStamp())), "(mean), ",
              round(db.getTimeStampStd() / 3600 / 24, 2), "days (rms)")
        oldDir = gDirectory
        histos = []
        for slot in range(1, 17):
            oldDir.mkdir("slot" + f'{slot:02d}').cd()
            h = db.getPulseHeights(slot)
            if not h:
                continue
            hmean = TH1F('meanPH', 'slot ' + str(slot) + '; PMT number; mean pulse-height', 32, 0.5, 32.5)
            for pmt in range(1, 33):
                meanPH = self.makePlot(h, slot, pmt)
                hmean.SetBinContent(pmt, meanPH)
            histos.append(hmean)
            oldDir.cd("..")

        canvas = TCanvas("meanPH", "Mean pulse-heights", 1000, 1000)
        canvas.Divide(4, 4)
        for i, h in enumerate(histos):
            canvas.cd(i + 1)
            h.Draw()
        canvas.Write()
        f.Close()
        print("--> canvases saved to:", fileName)


b2.set_log_level(b2.LogLevel.ERROR)

if '.txt' in globalTag:
    b2.conditions.append_testing_payloads(globalTag)
else:
    b2.conditions.append_globaltag(globalTag)

main = b2.create_path()
main.add_module('EventInfoSetter', evtNumList=[1], runList=[runNo], expList=[expNo])
main.add_module(SaveCanvases())
b2.process(main)
