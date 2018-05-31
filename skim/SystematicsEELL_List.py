#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics skim(s) for radiative muon pairs
# Skim code: 10600600
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsList():

    Lists = []
    Lists += EEMuMuList()
    Lists += EEEEList()

    return Lists


def EEMuMuList():

    # Once expert PID will be more stable, we might want use them, but so far let's stick to what Torben
    # use for radiative pairs...
    # the tight selection starts with all muons, but they  must be cluster-matched and not be an electron
    MuonTightSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and clusterE > 0.0 and clusterE < 1.0'
    cutAndCopyList('mu+:skimtight', 'mu+:all', MuonTightSelection)

    # for the loose selection starts with all muons, but we accept tracks that
    # are not matched to a cluster, but if they are, they must not be an
    # electron
    MuonLooseSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and clusterE < 1.0'
    cutAndCopyList('mu+:skimloose', 'mu+:all', MuonLooseSelection)

    # create a list of possible selections
    eemumulist = []

    # the muon pair has low invariant mass and tracks are back-to-back-like
    EEMuMuSelection = 'M < 4 and useCMSFrame(daughterAngle(0,1)) < 0.75'
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    reconstructDecay('gamma:eemumu -> mu+:skimtight mu-:skimloose', EEMuMuSelection + " and " + eventCuts)
    eemumulist.append('gamma:eemumu')

    return eemumulist


def EEEEList():

    # Once expert PID will be more stable, we might want use them, but so far let's stick to ECL-only variables
    # the tight selection starts with all electrons matched to high-e cluster (to separate from eemumu)
    eTightSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and clusterE > 0.0 and clusterE > 1.'
    cutAndCopyList('e+:skimtight', 'e+:all', eTightSelection)

    # for the loose selection starts with all electrons, but we accept tracks that
    # are not matched to a cluster
    eLooseSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    cutAndCopyList('e+:skimloose', 'e+:all', eLooseSelection)

    # create a list of possible selections
    eeeelist = []

    # the electron pair has low invariant mass and tracks are back-to-back-like
    EEEESelection = 'M < 4 and useCMSFrame(daughterAngle(0,1)) < 0.75'
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    reconstructDecay('gamma:eeee -> e+:skimtight e-:skimloose', EEEESelection + " and " + eventCuts)
    eeeelist.append('gamma:eeee')

    return eeeelist
