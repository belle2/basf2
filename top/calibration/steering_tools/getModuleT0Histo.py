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
from ROOT import Belle2
from ROOT import TFile, TH1F

# ------------------------------------------------------------------------
# Save module T0 constants as a histogram into a root file
#
# usage: basf2 getModuleT0Histo.py expNo runNo globalTag/localDB
# -------------------------------------------------------------------------

argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "expNo runNo globalTag/localDB")
    sys.exit()

expNo = int(argvs[1])
runNo = int(argvs[2])
tag = argvs[3]


class SaveModuleT0(b2.Module):
    ''' Save module T0 from database into a root file as histogram '''

    def initialize(self):
        ''' initialize '''

        #: payload
        self.db = Belle2.PyDBObj('TOPCalModuleT0')

        file_name = 'moduleT0_DB-' + 'e' + f'{expNo:04d}' + '-r' + f'{runNo:05d}' + '.root'
        #: output file name
        self.file = TFile.Open(file_name, 'recreate')
        #: histogram
        self.h = TH1F('moduleT0', tag + '; slot number; module T0 [ns]', 16, 0.5, 16.5)

    def event(self):
        ''' event processing '''

        for slot in range(1, 17):
            self.h.SetBinContent(slot, self.db.getT0(slot))
            self.h.SetBinError(slot, self.db.getT0Error(slot))
            if self.db.isCalibrated(slot):
                print('slot', slot, 'status = calibrated')
            elif self.db.isUnusable(slot):
                print('slot', slot, 'status = unusable')
            else:
                print('slot', slot, 'status = default')

    def terminate(self):
        ''' terminate '''

        self.file.Write()
        self.file.Close()


# Database
if '.txt' in tag:
    b2.conditions.append_testing_payloads(tag)
else:
    b2.conditions.append_globaltag(tag)

# Create path
main = b2.create_path()

# Set number of events to generate
evtList = [1]
runList = [runNo]
expList = [expNo]

# Event info setter
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': evtList, 'runList': runList, 'expList': expList})
main.add_module(eventinfosetter)

# Save module T0 from database into a root file as histogram
main.add_module(SaveModuleT0())

# Process events
b2.process(main)
