#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
DQM Import test
'''
import os
import basf2 as b2
from ROOT import Belle2, TFile, TH1F

filein = "histin6.root"
fileout = 'histout6.root'

f = TFile(filein, "RECREATE")

f.mkdir("DQMInfo")
f.cd("DQMInfo")
h_expno = TH1F("expno", "1", 1, 0, 1)
h_runno = TH1F("runno", "1", 1, 0, 1)
h_rtype = TH1F("rtype", "null", 1, 0, 1)
h_expno.Write()
h_runno.Write()
h_rtype.Write()
f.cd("..")

f.mkdir("TEST")
f.cd("TEST")
h_test = TH1F("test", "", 1, 0, 1)
h_test.Write()

f.Write()
f.Close()


class Ender(b2.Module):
    """Force to end processing after three events."""

    #: Event counter
    eventnr = 0
    #: Event Meta Data
    evtmetadata = Belle2.PyStoreObj('EventMetaData')

    def event(self):
        """
        Called for each event, force end after three calls
        """
        self.eventnr += 1
        if self.eventnr == 3:
            self.evtmetadata.obj().setEndOfData()


main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInput2')
dqminput.param('HistMemoryPath', filein)
dqminput.param('RefreshInterval', 0)
dqminput.param('StatFileName', "stats.txt")
dqminput.param("EnableRunInfo", False)
main.add_module(dqminput)

main.add_module(Ender())

dqmoutput = b2.register_module('DQMHistAnalysisOutputFile')
dqmoutput.param('OutputFolder', './')
dqmoutput.param('Filename', fileout)
main.add_module(dqmoutput)

# Process all events
b2.process(main)

os.remove(filein)
os.remove(fileout)
