#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
import multiprocessing
import ROOT
from basf2 import set_random_seed, create_path, process
from modularAnalysis import variablesToNtuple


def fork_process(*args, target=process):
    """Run function in forked child to eliminate side effects like B2FATAL"""
    # stolen from framework/tests/logging.py
    set_random_seed("1337")
    sub = multiprocessing.Process(target=target, args=args)
    sub.start()
    sub.join()


set_random_seed("1337")

###############################################################################
# add the same V2NT twice without specifying a different name
testpath = create_path()
testpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
testpath.add_module('ParticleLoader', decayStringsWithCuts=[('e+', '')])
variablesToNtuple("e+", ['electronID', 'p', 'isSignal'], path=testpath)
variablesToNtuple("e+", ['electronID', 'p', 'isSignal'], path=testpath)
with tempfile.TemporaryDirectory() as tempdir:
    os.chdir(tempdir)
    fork_process(testpath)  # throws B2FATAL, *this* script needs to exit happily

###############################################################################
# add a V2NT for a non-existent particle list (the ParticleLoader isn't there)
testpath = create_path()
testpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
variablesToNtuple("e+", ['electronID', 'p', 'isSignal'], path=testpath)
with tempfile.TemporaryDirectory() as tempdir:
    os.chdir(tempdir)
    fork_process(testpath)

###############################################################################
# correctly add two ntuples to the same file, with a new file in between
testpath = create_path()
testpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
testpath.add_module('ParticleLoader', decayStringsWithCuts=[('e+', '')])
testpath.add_module('ParticleLoader', decayStringsWithCuts=[('pi+', '')])
variablesToNtuple("e+", ['electronID', 'p', 'isSignal'], treename='firsttree',
                  filename="firstfile.root", path=testpath)

variablesToNtuple("", ['nTracks'], treename='intermediate',
                  filename="secondfile.root", path=testpath)

variablesToNtuple("pi+", ['electronID', 'p', 'isSignal'], treename='secondtree',
                  filename="firstfile.root", path=testpath)

with tempfile.TemporaryDirectory() as tempdir:
    os.chdir(tempdir)
    fork_process(testpath)
    f1 = ROOT.TFile('firstfile.root')
    t1 = f1.Get('firsttree')
    t2 = f1.Get('secondtree')
    assert bool(t1), "firsttree isn't contained in file"
    assert bool(t2), "secondtree isn't contained in file"

    f2 = ROOT.TFile('secondfile.root')
    t = f2.Get('intermediate')
    assert bool(t), "intermediate isn't contained in file"
