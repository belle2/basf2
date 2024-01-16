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
from ROOT import Belle2, TFile
import time

# -------------------------------------------------------------------------------------
# Extract histograms from the payload TOPCalPhotonYields and save them into a root file
#
# Usage: basf2 checkDB-photonYields.py expNo runNo [globalTag/localDB]
#        (default globalTag = data_reprocessing_prompt)
# -------------------------------------------------------------------------------------

if len(sys.argv) < 3:
    print("usage: basf2", sys.argv[0], "expNo runNo [globalTag/localDB]")
    sys.exit()

expNo = int(sys.argv[1])
runNo = int(sys.argv[2])
globalTag = "data_reprocessing_prompt"
if len(sys.argv) > 3:
    globalTag = sys.argv[3]


class SaveHisto(b2.Module):
    ''' Saving histograms from the payload to a root file'''

    def initialize(self):
        ''' initialize: saving histograms implemented here '''

        db = Belle2.PyDBObj("TOPCalPhotonYields")
        if not db:
            return

        fileName = "photonYields-" + time.strftime("%Y-%m-%d", time.localtime(db.getTimeStamp())) + ".root"
        f = TFile.Open(fileName, "recreate")

        print("- global tag:", globalTag)
        print("- time of measurement:",
              time.strftime("%d %b %Y %H:%M:%S", time.localtime(db.getTimeStamp())), "(mean), ",
              round(db.getTimeStampStd() / 3600 / 24, 2), "days (rms)")
        for slot in range(1, 17):
            h = db.getPhotonYields(slot)
            if h:
                h.Write()
            h = db.getBackgroundYields(slot)
            if h:
                h.Write()
            h = db.getAlphaRatio(slot)
            if h:
                h.Write()
            h = db.getActivePixels(slot)
            if h:
                h.Write()
            h = db.getPulseHeights(slot)
            if h:
                h.Write()
            h = db.getMuonZ(slot)
            if h:
                h.Write()
        f.Close()
        print("--> histograms saved to:", fileName)


b2.set_log_level(b2.LogLevel.ERROR)

if '.txt' in globalTag:
    b2.conditions.append_testing_payloads(globalTag)
else:
    b2.conditions.append_globaltag(globalTag)

main = b2.create_path()
main.add_module('EventInfoSetter', evtNumList=[1], runList=[runNo], expList=[expNo])
main.add_module(SaveHisto())
b2.process(main)
