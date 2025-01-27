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
from ROOT import TFile, TH1F

filein = "histin3.root"
fileout = 'histout3.root'

f = TFile(filein, "RECREATE")

f.mkdir("DQMInfo")
f.cd("DQMInfo")
h_expno = TH1F("expno", "", 1, 0, 1)
h_runno = TH1F("runno", "", 1, 0, 1)
h_rtype = TH1F("rtype", "", 1, 0, 1)
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

main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInputRootFile')
dqminput.param('SelectHistograms', [])  # leave blank to include all folders
dqminput.param('FileList', [filein])
dqminput.param('EventInterval', 0)
dqminput.param("EnableRunInfo", False)
main.add_module(dqminput)

dqmoutput = b2.register_module('DQMHistAnalysisOutputFile')
dqmoutput.param('OutputFolder', './')
dqmoutput.param('Filename', fileout)
main.add_module(dqmoutput)

# Process all events
b2.process(main)

os.remove(filein)
os.remove(fileout)
