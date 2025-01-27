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
from ROOT import Belle2, TFile, TH1F, gROOT

gROOT.SetBatch(True)

filein = "histin5.root"
fileout = 'histout5.root'
statfile = "stats5.txt"

f = TFile(filein, "RECREATE")

h_expno = TH1F("DQMInfo/expno", "1", 1, 0, 1)
h_runno = TH1F("DQMInfo/runno", "1", 1, 0, 1)
h_rtype = TH1F("DQMInfo/rtype", "null", 1, 0, 1)
h_test = TH1F("TEST/test", "", 1, 0, 1)
h_nevent = TH1F("DAQ/Nevent", "", 1, 0, 1)
for n in range(0, 10):
    h_nevent.Fill(n)
h_expno.Write()
h_runno.Write()
h_rtype.Write()
h_test.Write()
h_nevent.Write()

f.Write()
f.Close()

Belle2.Environment.Instance().setNumberEventsOverride(3)

main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInput2')
dqminput.param('HistMemoryPath', filein)
dqminput.param('RefreshInterval', 0)
dqminput.param('StatFileName', statfile)
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
b2.B2INFO("== resulting file content ==")
f = TFile(fileout, "READ")
for k in f.GetListOfKeys():
    o = k.ReadObj()
    b2.B2INFO(o.ClassName(), k)
    if o.GetName() == "DQMInfo/c_info":
        if "Exp 1, Run 1, RunType null" not in o.GetTitle():
            b2.B2ERROR(f"Run Info not found in {o.GetName()}: {o.GetTitle()}")
    if o.GetName() in expected:
        expected.remove(o.GetName())
b2.B2INFO("============================")
if len(expected) > 0:
    b2.B2ERROR("missing items in outfile: ", expected)
b2.B2INFO("== resulting stat content ==")
with open(statfile, 'r') as f:
    b2.B2INFO(f.read())
b2.B2INFO("=========================")

os.remove(statfile)
os.remove(filein)
os.remove(fileout)
