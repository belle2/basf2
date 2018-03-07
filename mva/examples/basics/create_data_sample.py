#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

# Create a simple data sample with some variables

from basf2 import *
from modularAnalysis import *

variables = ['p', 'pt', 'pz', 'phi',
             'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
             'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
             'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
             'chiProb', 'dr', 'dz', 'dphi',
             'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
             'daughter(0, dphi)', 'daughter(1, dphi)',
             'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
             'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
             'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
             'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
             'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
             'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))',
             'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
spectators = ['isSignal', 'M']


def reconstruction_path(inputfiles):
    path = create_path()
    inputMdstList('MC7', inputfiles, path=path)
    fillParticleLists([('K-', 'kaonID > 0.5'), ('pi+', 'pionID > 0.5'),
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

    # Create a train, test and validation sample with different MC files
    # Add your root files here
    f = 'mdst_002001_prod00000789_task00004203.root'
    path = reconstruction_path([f])
    variablesToNTuple('D0', variables + spectators, filename='train.root', treename='tree', path=path)
    process(path)

    # Add your root files here
    path = reconstruction_path([f])
    variablesToNTuple('D0', variables + spectators, filename='test.root', treename='tree', path=path)
    process(path)

    # Add your root files here
    path = reconstruction_path([f])
    variablesToNTuple('D0', variables + spectators, filename='validation.root', treename='tree', path=path)
    process(path)
