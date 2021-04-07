#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT
import b2test_utils
from basf2 import create_path, set_random_seed
from b2test_utils import configure_logging_for_tests
from modularAnalysis import variablesToNtuple

set_random_seed("1337")
# make logging a bit less verbose and more reproducible
configure_logging_for_tests()

###############################################################################
# add the same V2NT twice without specifying a different name
testpath = create_path()
testpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
testpath.add_module('ParticleLoader', decayStrings=['e+'])
variablesToNtuple("e+:all", ['electronID', 'p', 'isSignal'], path=testpath)
variablesToNtuple("e+:all", ['electronID', 'p', 'isSignal'], path=testpath)
with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(testpath)  # throws B2FATAL, *this* script needs to exit happily

###############################################################################
# add a V2NT for a non-existent particle list (the ParticleLoader isn't there)
testpath = create_path()
testpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
variablesToNtuple("e+", ['electronID', 'p', 'isSignal'], path=testpath)
with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(testpath)

###############################################################################
# correctly add two ntuples to the same file, with a new file in between
testpath = create_path()
testpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
testpath.add_module('ParticleLoader', decayStrings=['e+'])
testpath.add_module('ParticleLoader', decayStrings=['pi+'])
variablesToNtuple("e+:all", ['electronID', 'p', 'isSignal'], treename='firsttree',
                  filename="firstfile.root", path=testpath)

variablesToNtuple("", ['nTracks'], treename='intermediate',
                  filename="secondfile.root", path=testpath)

variablesToNtuple("pi+:all", ['electronID', 'p', 'isSignal'], treename='secondtree',
                  filename="firstfile.root", path=testpath)

with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(testpath)
    f1 = ROOT.TFile('firstfile.root')
    t1 = f1.Get('firsttree')
    t2 = f1.Get('secondtree')
    assert bool(t1), "firsttree isn't contained in file"
    assert bool(t2), "secondtree isn't contained in file"

    f2 = ROOT.TFile('secondfile.root')
    t = f2.Get('intermediate')
    assert bool(t), "intermediate isn't contained in file"
