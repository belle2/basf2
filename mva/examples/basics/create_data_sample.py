#!/usr/bin/env python3

# Thomas Keck 2016

# Create a simple data sample with some variables

import basf2 as b2
import modularAnalysis as ma
import vertex as vx
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
             'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))',
             'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
spectators = ['isSignal', 'M']


def reconstruction_path(inputfiles):
    path = b2.create_path()
    ma.inputMdstList('default', inputfiles, path=path)
    ma.fillParticleLists([('K-', 'kaonID > 0.5'), ('pi+', 'pionID > 0.5'),
                          ('gamma', '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.09] or '
                           '[clusterReg == 3 and E > 0.16]] and abs(clusterTiming) < 20 and clusterE9E25 > 0.7'
                           ' and minC2TDist > 35')],
                         path=path)
    ma.reconstructDecay('pi0 -> gamma gamma', '0.1 < M < 1.6', path=path)
    vx.kFit('pi0', 0.1, fit_type='massvertex', path=path)
    ma.reconstructDecay('D0 -> K- pi+ pi0', '1.8 < M < 1.9', path=path)
    vx.kFit('D0', 0.1, path=path)
    ma.applyCuts('D0', '1.7 < M < 1.9', path=path)
    ma.matchMCTruth('D0', path=path)
    ma.applyCuts('D0', 'isNAN(isSignal) == False', path=path)
    return path


if __name__ == "__main__":

    # Create a train, test and validation sample with different MC files
    path = reconstruction_path([b2.find_file('ccbar_sample_to_train.root', 'examples', False)])
    ma.variablesToNtuple('D0', variables + spectators, filename='train.root', treename='tree', path=path)
    b2.process(path, 100000)

    path = reconstruction_path([b2.find_file('ccbar_sample_to_test.root', 'examples', False)])
    ma.variablesToNtuple('D0', variables + spectators, filename='test.root', treename='tree', path=path)
    b2.process(path, 100000)

    path = reconstruction_path([b2.find_file('ccbar_sample_to_test.root', 'examples', False)])
    ma.variablesToNtuple('D0', variables + spectators, filename='validation.root', treename='tree', path=path)
    b2.process(path, 100000)
