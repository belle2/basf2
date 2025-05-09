#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
DQM RootImport test
Test import for Offline root file
with histogram name include subdir prefix
Test exp/run/type overide of existing histograms
'''
import os
import basf2 as b2
from ROOT import TFile, TH1F, gROOT

gROOT.SetBatch(True)

filein = "histin5.root"
fileout = 'histout5.root'

f = TFile(filein, "RECREATE")

h_expno = TH1F("DQMInfo/expno", "1", 1, 0, 1)
h_runno = TH1F("DQMInfo/runno", "1", 1, 0, 1)
h_rtype = TH1F("DQMInfo/rtype", "null", 1, 0, 1)
h_test = TH1F("TEST/test", "", 1, 0, 1)
h_nevent = TH1F("DAQ/Nevent", "", 1, 0, 1)
for n in range(0, 10):
    h_nevent.Fill(n)

f.Write()
f.Close()

main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInputRootFile')
dqminput.param('FileList', [filein])
dqminput.param('Experiment', 2)
dqminput.param('RunType', 'beam')
dqminput.param('RunList', [2])
dqminput.param('FillNEvent', 11)
dqminput.param('EventInterval', 0)
dqminput.param("EnableRunInfo", True)
main.add_module(dqminput)

main.add_module("DQMHistAutoCanvas")

dqmoutput = b2.register_module('DQMHistAnalysisOutputFile')
dqmoutput.param('OutputFolder', './')
dqmoutput.param('Filename', fileout)
main.add_module(dqmoutput)

# Process all events
b2.process(main)

expected = ["DQMInfo/c_info", "DAQ/c_Nevent", "DQMInfo/c_expno", "DQMInfo/c_runno", "DQMInfo/c_rtype", "TEST/c_test"]
print("== resulting file content ==")
f = TFile(fileout, "READ")
for k in f.GetListOfKeys():
    o = k.ReadObj()
    print(o.ClassName(), k)
    if o.GetName() == "DQMInfo/c_info":
        if "Exp 2, Run 2, RunType beam" not in o.GetTitle():
            b2.B2ERROR(f"Run Info not found in {o.GetName()}: {o.GetTitle()}")
    if o.GetName() in expected:
        expected.remove(o.GetName())
print("============================")
if len(expected) > 0:
    b2.B2ERROR("missing items in outfile: ", expected)

os.remove(filein)
os.remove(fileout)
