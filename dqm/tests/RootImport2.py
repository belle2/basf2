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
from ROOT import TFile, TH1F, gROOT

gROOT.SetBatch(True)

filein = "histin3.root"
fileout = 'histout3.root'

f = TFile(filein, "RECREATE")

f.mkdir("DQMInfo")
f.cd("DQMInfo")
h_expno = TH1F("expno", "1", 1, 0, 1)
h_runno = TH1F("runno", "1", 1, 0, 1)
h_rtype = TH1F("rtype", "null", 1, 0, 1)
f.cd("..")

f.mkdir("TEST")
f.cd("TEST")
h_test = TH1F("test", "", 1, 0, 1)
f.cd("..")

f.mkdir("DAQ")
f.cd("DAQ")
h_nevent = TH1F("Nevent", "", 1, 0, 1)
for n in range(0, 10):
    h_nevent.Fill(n)
f.cd("..")

f.Write()
f.Close()

main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInputRootFile')
dqminput.param('SelectHistograms', [])  # leave blank to include all folders
dqminput.param('FileList', [filein])
dqminput.param('EventInterval', 0)
dqminput.param("AddRunControlHist", False)
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
        if "Exp 1, Run 1, RunType null" not in o.GetTitle():
            b2.B2ERROR(f"Run Info not found in {o.GetName()}: {o.GetTitle()}")
    if o.GetName() in expected:
        expected.remove(o.GetName())
print("============================")
if len(expected) > 0:
    b2.B2ERROR("missing items in outfile: ", expected)

os.remove(filein)
os.remove(fileout)
