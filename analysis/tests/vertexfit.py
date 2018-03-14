#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *
from modularAnalysis import *
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


testFile = tempfile.NamedTemporaryFile()

main = create_path()

inputMdst('MC5', Belle2.FileSystem.findFile('analysis/tests/mdst.root'), path=main)

fillParticleList('K-', '', path=main)
fillParticleList('pi+', '', path=main)

fillParticleList('gamma', '', path=main)
reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)
matchMCTruth('pi0', path=main)

# KFit
fitVertex('pi0', 0.0, '', 'kfitter', 'vertex', path=main)

reconstructDecay('D0 -> K- pi+', '', 0, path=main)
matchMCTruth('D0', path=main)

# Rave
fitVertex('D0', 0.0, '', 'rave', 'vertex', path=main)

ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', testFile.name)
ntupler.param('variables', ['M', 'isSignal', 'distance', 'dr', 'dz', 'significanceOfDistance', 'pValue'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

process(main)

print(statistics)


def check():
    """
    Verify results make sense.
    """
    ntuplefile = TFile(testFile.name)
    ntuple = ntuplefile.Get('ntuple')

    if ntuple.GetEntries() == 0:
        B2FATAL("No D0s saved")

# these are empty?
#    if ntuple.GetEntries("pValue < 0") > 0:
#        B2FATAL("Some entries have negative pValue?")
#
#    if ntuple.GetEntries("pValue > 0") == 0:
#        B2FATAL("All fits failed?")
#
    if ntuple.GetEntries("significanceOfDistance < 0 && significanceOfDistance != -1") > 0:
        B2FATAL("Some entries have negative significanceOfDistance?")

    if ntuple.GetEntries("significanceOfDistance > 0") == 0:
        B2FATAL("significanceOfDistance never positive?")
    # TODO what else to check?


check()

print("Test passed, cleaning up.")
