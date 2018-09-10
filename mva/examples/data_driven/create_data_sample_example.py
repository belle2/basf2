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
             'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)', 'daughter(2, M)',
             'daughter(0, atcPIDBelle(3,2))', 'daughter(1, atcPIDBelle(3,2))',
             'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
             'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
             'daughter(2, daughter(0, clusterLAT))', 'daughter(2, daughter(1, clusterLAT))',
             'daughter(2, daughter(0, clusterHighestE))', 'daughter(2, daughter(1, clusterHighestE))',
             'daughter(2, daughter(0, clusterNHits))', 'daughter(2, daughter(1, clusterNHits))',
             'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
             'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))',
             'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
spectators = ['isSignal', 'M', 'expNum', 'evtNum', 'runNum', 'mcErrors']

import b2biiConversion
import ROOT
from ROOT import Belle2
ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)


def reconstruction_path(inputfiles):
    path = create_path()
    b2biiConversion.convertBelleMdstToBelleIIMdst(None, applyHadronBJSkim=True, path=path)
    setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)
    fillParticleLists([('K-', 'atcPIDBelle(3,2) > 0.2 and dr < 2 and abs(dz) < 4'),
                       ('pi+', 'atcPIDBelle(3,2) < 0.2 and dr < 2 and abs(dz) < 4')], path=path)
    massVertexKFit('pi0:mdst', 0.1, path=path)
    reconstructDecay('D0 -> K- pi+ pi0:mdst', '1.7 < M < 2.0', path=path)
    fitVertex('D0', 0.1, fitter='kfitter', path=path)
    applyCuts('D0', '1.7 < M < 2.0', path=path)
    matchMCTruth('D0', path=path)
    return path


if __name__ == "__main__":
    path = reconstruction_path([])
    variablesToNtuple('D0', variables + spectators, filename='validation.root', treename='tree', path=path)
    process(path)
