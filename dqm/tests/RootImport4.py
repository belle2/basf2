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
import basf2 as b2
from ROOT import TFile, TH1F

filein = "histin4.root"
fileout = 'histout4.root'

f = TFile(filein, "RECREATE")

h_test = TH1F("TEST/test", "", 1, 0, 1)
h_test.Write()

f.Write()
f.Close()

main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInputRootFile')
dqminput.param('SelectHistograms', [])  # leave blank to include all folders
dqminput.param('FileList', [filein])
dqminput.param('EventInterval', 0)
dqminput.param("AddRunControlHist", True)
main.add_module(dqminput)

dqmoutput = b2.register_module('DQMHistAnalysisOutputFile')
dqmoutput.param('OutputFolder', './')
dqmoutput.param('Filename', fileout)
main.add_module(dqmoutput)

# Process all events
b2.process(main)
