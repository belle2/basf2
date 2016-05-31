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


def reconstruction_path():
    path = create_path()
    inputMdstList(['/local/scratch/MC/MC5/Charged02/mdst_00200*'], path=path)
    fillParticleLists([('K-', 'Kid > 0.5'), ('pi+', 'piid > 0.5')], path=path)
    reconstructDecay('D0 -> K- pi+', '1.8 < M < 1.9', path=path)
    fitVertex('D0', 0.1, path=path)
    matchMCTruth('D0', path=path)
    return path


if __name__ == "__main__":
    path = reconstruction_path()
    variablesToNTuple('D0', variables + spectators, filename='train.root', treename='tree', path=path)
    process(path)
