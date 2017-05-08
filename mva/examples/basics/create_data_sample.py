#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

# Create a simple data sample with some variables

from basf2 import *
from modularAnalysis import *

variables = ['p', 'pt', 'pz',
             'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
             'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
             'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
             'chiProb', 'dr', 'dz',
             'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
             'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
             'daughter(0, Kid)', 'daughter(0, piid)',
             'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
spectators = ['isSignal', 'M']


def reconstruction_path(inputfiles):
    path = create_path()
    inputMdstList('MC6', inputfiles, path=path)
    fillParticleLists([('K-', 'Kid > 0.5'), ('pi+', 'piid > 0.5'),
                       ('gamma', 'goodGamma == 1 and abs(clusterTiming) < 20 and clusterE9E25 > 0.7 and minC2HDist > 35')],
                      path=path)
    reconstructDecay('pi0 -> gamma gamma', '0.1 < M < 1.6', path=path)
    massVertexKFit('pi0', 0.1, path=path)
    reconstructDecay('D0 -> K- pi+ pi0', '1.8 < M < 1.9', path=path)
    fitVertex('D0', 0.1, fitter='kfitter', path=path)
    applyCuts('D0', '1.7 < M < 1.9', path=path)
    matchMCTruth('D0', path=path)
    return path


if __name__ == "__main__":
    path = reconstruction_path(['/storage/jbod/tkeck/MC6/evtgen-charged/sub00/mdst_0000*.root'])
    variablesToNTuple('D0', variables + spectators, filename='train.root', treename='tree', path=path)
    process(path)

    path = reconstruction_path(['/storage/jbod/tkeck/MC6/evtgen-charged/sub00/mdst_0001*.root'])
    variablesToNTuple('D0', variables + spectators, filename='test.root', treename='tree', path=path)
    process(path)
