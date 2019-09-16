#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""

Skim list building functions for EWP inclusive skims:
B->Xgamma, B->Xll, B->Xll (LFV modes)

"""

__authors__ = [
    "Trevor Shillington"
]

from basf2 import *
from modularAnalysis import *


def B2XgammaList(path):
    """Build the skim list for B --> X(s,d) gamma decays"""

    # event level cuts: R2 and require a minimum number of tracks + decent photons
    fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    # Apply event cuts R2 < 0.5 and nTracks >= 3
    applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

    # Apply gamma cuts clusterE9E21 > 0.9 and 1.4 < E_gamma < 3.4 GeV (in CMS frame)
    cutAndCopyList('gamma:ewp', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

    reconstructDecay('B+:gamma -> gamma:ewp', '', path=path)

    BtoXgammaList = ['B+:gamma']

    return BtoXgammaList


def B2XllList(path):
    """Build the skim list for B --> X(s,d) l+ l- decays"""

    # event level cuts: R2 and require a minimum number of tracks
    fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    # Apply event cuts R2 < 0.5 and nTracks >= 3
    applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

    # Apply electron cut p > 0.395 GeV
    # Apply muon cuts p > 0.395 GeV, muonID > 0.5 and electronID < 0.1 (to avoid double counting)
    cutAndCopyList('e+:ewp', 'e+:loose', 'p > 0.395', path=path)
    cutAndCopyList('mu+:ewp', 'mu+:loose', 'muonID > 0.5 and p > 0.395 and electronID < 0.1', path=path)

    # Apply dilepton cut E_ll > 1.5 GeV (in CMS frame)
    E_dilep_cut = 'formula(daughter(0, useCMSFrame(E))+daughter(1, useCMSFrame(E))) > 1.5'

    # B+ reconstruction:
    # oppositely charged leptons
    reconstructDecay('B+:ch1 -> e+:ewp e-:ewp', E_dilep_cut, dmID=1, path=path)
    reconstructDecay('B+:ch2 -> mu+:ewp mu-:ewp', E_dilep_cut, dmID=2, path=path)
    # same charge leptons
    reconstructDecay('B+:ch3 -> e+:ewp e+:ewp', E_dilep_cut, dmID=3, path=path)
    reconstructDecay('B+:ch4 -> mu+:ewp mu+:ewp', E_dilep_cut, dmID=4, path=path)

    copyLists('B+:xll', ['B+:ch1', 'B+:ch2', 'B+:ch3', 'B+:ch4'], path=path)

    BptoXllList = ['B+:xll']

    return BptoXllList


def B2XllListLFV(path):
    # Build the skim list for B -> X ll decays (LFV modes only, inclusive)

    # Create lists for buildEventShape (basically all tracks and clusters)
    cutAndCopyList('pi+:eventShapeForSkims', 'pi+:all', 'pt> 0.1', path=path)
    cutAndCopyList('gamma:eventShapeForSkims', 'gamma:all', 'E > 0.1', path=path)

    # buildEventShape to access R2
    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    # Apply event cuts R2 < 0.5 and nTracks >= 3
    applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

    # Apply electron cut p > 0.395 GeV
    # Apply muon cuts p > 0.395 GeV, muonID > 0.5 and electronID < 0.1 (to avoid double counting)
    cutAndCopyList('e+:ewp', 'e+:loose', 'p > 0.395', path=path)
    cutAndCopyList('mu+:ewp', 'mu+:loose', 'muonID > 0.5 and p > 0.395 and electronID < 0.1', path=path)

    # Apply dilepton cut E_ll > 1.5 GeV (in CMS frame)
    E_dilep_cut = 'formula(daughter(0, useCMSFrame(E))+daughter(1, useCMSFrame(E))) > 1.5'

    # B+ reconstruction:
    # oppositely charged leptons
    reconstructDecay('B+:lfvch1 -> e+:ewp mu-:ewp', E_dilep_cut, dmID=1, path=path)
    reconstructDecay('B+:lfvch2 -> mu+:ewp e-:ewp', E_dilep_cut, dmID=2, path=path)
    # same charge leptons
    reconstructDecay('B+:lfvch3 -> e+:ewp mu+:ewp', E_dilep_cut, dmID=3, path=path)

    copyLists('B+:lfv', ['B+:lfvch1', 'B+:lfvch2', 'B+:lfvch3'], path=path)

    BtoXllListLFV = ['B+:lfv']

    return BtoXllListLFV
