#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics skim(s) for radiative muon pairs
# Skim code: 10600500
# Torben Ferber (torben.ferber@desy.de), 2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsRadMuMuList():

    Lists = []
    Lists += RadMuMuList()

    return Lists


def RadMuMuList():

    # the tight selection starts with all muons, but they  must be cluster-matched and not be an electron
    MuonTightSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0 and clusterE > 0.0 and clusterE < 1.0'
    cutAndCopyList('mu+:skimtight', 'mu+:all', MuonTightSelection)

    # for the loose selection starts with all muons, but we accept tracks that
    # are not matched to a cluster, but if they are, they must not be an
    # electron
    MuonLooseSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0 and clusterE < 1.0'
    cutAndCopyList('mu+:skimloose', 'mu+:all', MuonLooseSelection)

    # create a list of possible selections
    radmumulist = []

    # selection ID0:
    # the radiative muon pair must be selected without looking at the photon. exclude events with more than two good tracks
    RadMuMuSelection = 'pRecoil > 0.075 and pRecoilTheta > 0.296706 and pRecoilTheta < 2.61799'
    RadMuMuPairChannel = 'mu+:skimtight mu-:skimloose'
    chID = 0
    reconstructDecay('vpho:radmumu' + str(chID) + ' -> ' + RadMuMuPairChannel, RadMuMuSelection, chID)
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    applyCuts('vpho:radmumu' + str(chID), eventCuts)
    radmumulist.append('vpho:radmumu' + str(chID))

    # selection Id1:
    # todo: include pair conversions?

    return radmumulist
