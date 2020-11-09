#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
import ROOT
import b2test_utils

inputFile = b2test_utils.require_file('mdst12.root', 'validation')
path = basf2.create_path()
path.add_module('RootInput', inputFileName=inputFile)

# Add path for high multiplicity events
mod = basf2.register_module('VariableToReturnValue')
mod.param('variable', 'nTracks')
high_multiplicity_path = basf2.create_path()
high_multiplicity_path.add_module('VariablesToNtuple', particleList='', variables=['nTracks'], fileName='highMultiplicity.root')
mod.if_value('>= 12', high_multiplicity_path, basf2.AfterConditionPath.CONTINUE)
path.add_module(mod)


with b2test_utils.clean_working_directory():
    basf2.process(path)

    # Testing
    assert os.path.isfile('highMultiplicity.root'), "highMultiplicity.root wasn't created"
    f = ROOT.TFile('highMultiplicity.root')
    t = f.Get('ntuple')
    assert bool(t), "ntuple isn't contained in file"
    assert t.GetListOfBranches().Contains('nTracks'), "nTracks branch is missing"
    assert t.GetListOfBranches().Contains('__weight__'), "weight branch is missing"

    for event in t:
        assert event.nTracks >= 12, "Expected >= 12 tracks in selected high multiplicity events"
