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
processing of several input files
'''
import os
import basf2 as b2
from ROOT import TFile, TH1F, gROOT

gROOT.SetBatch(True)


filein = []

for i in range(1, 4):
    fname = f"histin6{i}.root"
    f = TFile(fname, "RECREATE")
    filein.append(fname)

    h_expno = TH1F("DQMInfo/expno", f"{i}", 1, 0, 1)
    h_runno = TH1F("DQMInfo/runno", f"{i}", 1, 0, 1)
    h_rtype = TH1F("DQMInfo/rtype", "null", 1, 0, 1)
    h_test = TH1F(f"TEST/test{i}", "", 1, 0, 1)
    h_nevent = TH1F("DAQ/Nevent", "", 1, 0, 1)
    for n in range(0, 10):
        h_nevent.Fill(n)

    f.Write()
    f.Close()

main = b2.create_path()

dqminput = b2.register_module('DQMHistAnalysisInputRootFile')
dqminput.param('FileList', filein)
dqminput.param('EventInterval', 0)
dqminput.param("EnableRunInfo", True)
main.add_module(dqminput)

main.add_module("DQMHistAutoCanvas")

dqmoutput = b2.register_module('DQMHistAnalysisOutputFile')
dqmoutput.param('OutputFolder', './')
# dqmoutput.param('Filename', fileout)
main.add_module(dqmoutput)

# Process all events
b2.process(main)


for i in range(1, 4):
    fileout = f"dqm_canvas_e000{i}r00000{i}.root"
    expected = ["DQMInfo/c_info", "DAQ/c_Nevent", "DQMInfo/c_expno", "DQMInfo/c_runno", "DQMInfo/c_rtype", f"TEST/c_test{i}"]
    print(f"== resulting file {fileout} content ==")
    f = TFile(fileout, "READ")
    for k in f.GetListOfKeys():
        o = k.ReadObj()
        print(o.ClassName(), k)
        if o.GetName() == "DQMInfo/c_info":
            if f"Exp {i}, Run {i}, RunType null" not in o.GetTitle():
                b2.B2ERROR(f"Run Info not found in {o.GetName()}: {o.GetTitle()}")
        if o.GetName() in expected:
            expected.remove(o.GetName())
    print("============================")
    if len(expected) > 0:
        b2.B2ERROR("missing items in outfile: ", expected)
    os.remove(fileout)

for f in filein:
    os.remove(f)
