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
from ROOT import Belle2, TCanvas, gStyle, gROOT
import time

# --------------------------------------------------------------------------------------------
# Make a plot of alpha-ratio histograms saved in database which shows optically decoupled PMTs
#
# Usage: basf2 alphaRatioPlots.py expNo runNo [globalTag/localDB]
#        (default globalTag = data_reprocessing_prompt)
# --------------------------------------------------------------------------------------------

if len(sys.argv) < 3:
    print("usage: basf2", sys.argv[0], "expNo runNo [globalTag/localDB]")
    sys.exit()

expNo = int(sys.argv[1])
runNo = int(sys.argv[2])
globalTag = "data_reprocessing_prompt"
if len(sys.argv) > 3:
    globalTag = sys.argv[3]


class MakePlots(b2.Module):
    ''' Makes a plot of alpha-ratio histograms '''

    def initialize(self):
        ''' initialize: implementation'''

        gROOT.SetBatch(True)

        db = Belle2.PyDBObj("TOPCalPhotonYields")
        if not db:
            return

        print("- global tag:", globalTag)
        print("- time of measurement:",
              time.strftime("%d %b %Y %H:%M:%S", time.localtime(db.getTimeStamp())), "(mean), ",
              round(db.getTimeStampStd() / 3600 / 24, 2), "days (rms)")

        canvas = TCanvas("c1", "alpha ratio", 2000, 1500)
        canvas.Divide(2, 8)
        gStyle.SetOptStat(0)
        gStyle.SetTitleFontSize(0.18)
        for slot in range(1, 17):
            canvas.cd(slot)
            pad = canvas.GetPad(slot)
            pad.SetLeftMargin(0.02)
            pad.SetRightMargin(0.02)
            pad.SetTopMargin(0.17)
            pad.SetBottomMargin(0.03)
            h = db.getAlphaRatio(slot)
            if not h:
                continue
            h.SetTitle("slot " + str(slot))
            h.SetMinimum(0)
            h.SetMaximum(1.2)
            h.Draw("col a")
        fileName = 'alphaRatio-' + time.strftime("%Y-%m-%d", time.localtime(db.getTimeStamp())) + '.png'
        canvas.SaveAs(fileName)
        print("--> plot saved as:", fileName)


b2.set_log_level(b2.LogLevel.ERROR)

if '.txt' in globalTag:
    b2.conditions.append_testing_payloads(globalTag)
else:
    b2.conditions.append_globaltag(globalTag)

main = b2.create_path()
main.add_module('EventInfoSetter', evtNumList=[1], runList=[runNo], expList=[expNo])
main.add_module(MakePlots())
b2.process(main)
