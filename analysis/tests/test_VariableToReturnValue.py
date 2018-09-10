#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *
import ROOT
from ROOT import Belle2

filepath = 'analysis/tests/mdst7.root'
inputFile = Belle2.FileSystem.findFile(filepath)
if len(inputFile) == 0:
    sys.stderr.write(
        "TEST SKIPPED: input file " +
        filepath +
        " not found. You can retrieve it via 'wget https://www.desy.de/~scunliff/mdst7.root'\n")
    sys.exit(-1)

path = create_path()
path.add_module('RootInput', inputFileName=inputFile)

# Add path for high multiplicity events
mod = register_module('VariableToReturnValue')
mod.param('variable', 'nTracks')
high_multiplicity_path = create_path()
high_multiplicity_path.add_module('VariablesToNtuple', particleList='', variables=['nTracks'], fileName='highMultiplicity.root')
mod.if_value('>= 12', high_multiplicity_path, AfterConditionPath.CONTINUE)
path.add_module(mod)


with tempfile.TemporaryDirectory() as tempdir:
    os.chdir(tempdir)
    process(path)

    # Testing
    assert os.path.isfile('highMultiplicity.root'), "highMultiplicity.root wasn't created"
    f = ROOT.TFile('highMultiplicity.root')
    t = f.Get('ntuple')
    assert bool(t), "ntuple isn't contained in file"
    assert t.GetListOfBranches().Contains('nTracks'), "nTracks branch is missing"
    assert t.GetListOfBranches().Contains('__weight__'), "weight branch is missing"

    for event in t:
        assert event.nTracks >= 12, "Expected >= 12 tracks in selected high multiplicity events"
