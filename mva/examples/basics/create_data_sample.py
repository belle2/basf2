#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

# Create a simple data sample with some variables

from basf2 import *
from modularAnalysis import *

variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
             'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
             'daughter(0, Kid)', 'daughter(0, piid)', 'daughterAngle(0, 1)']
spectators = ['isSignal', 'M']


def reconstruction_path(inputfiles):
    path = create_path()
    inputMdstList('MC6', inputfiles, path=path)
    fillParticleLists([('K-', 'Kid > 0.5'), ('pi+', 'piid > 0.5')], path=path)
    reconstructDecay('D0 -> K- pi+', '1.8 < M < 1.9', path=path)
    fitVertex('D0', 0.1, fitter='kfitter', path=path)
    applyCuts('D0', '1.8 < M < 1.9', path=path)
    matchMCTruth('D0', path=path)
    return path


if __name__ == "__main__":
    path = reconstruction_path(['/storage/jbod/tkeck/MC6/evtgen-charged/sub00/mdst_0000*.root'])
    variablesToNTuple('D0', variables + spectators, filename='train.root', treename='tree', path=path)
    process(path)

    path = reconstruction_path(['/storage/jbod/tkeck/MC6/evtgen-charged/sub00/mdst_0001*.root'])
    variablesToNTuple('D0', variables + spectators, filename='test.root', treename='tree', path=path)
    process(path)
