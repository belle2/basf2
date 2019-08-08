#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *
import b2test_utils
from modularAnalysis import *
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


def check(filename):
    """
    Verify results make sense.
    """
    ntuplefile = TFile(filename)
    ntuple = ntuplefile.Get('ntuple')

    if ntuple.GetEntries() == 0:
        B2FATAL("No D0s saved")

    if ntuple.GetEntries("significanceOfDistance < 0 && significanceOfDistance != -1") > 0:
        B2FATAL("Some entries have negative significanceOfDistance?")

    if ntuple.GetEntries("significanceOfDistance > 0") == 0:
        B2FATAL("significanceOfDistance never positive?")


testFile = tempfile.NamedTemporaryFile()

main = create_path()

inputMdst('default', b2test_utils.require_file('analysis/tests/mdst.root'), path=main)

fillParticleList('K-', '', path=main)
fillParticleList('pi+', '', path=main)

fillParticleList('gamma', '', path=main)
reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)
matchMCTruth('pi0', path=main)

# KFit
vertexKFit('pi0', 0.0, path=main)

reconstructDecay('D0 -> K- pi+', '', 0, path=main)
matchMCTruth('D0', path=main)

# Rave
vertexRave('D0', 0.0, path=main, silence_warning=True)

ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', testFile.name)
ntupler.param('variables', ['M', 'isSignal', 'distance', 'dr', 'dz', 'significanceOfDistance', 'pValue'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(main)
    check(testFile.name)
