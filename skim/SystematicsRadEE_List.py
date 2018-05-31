#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics skim(s) for radiative electron pairs
# Skim code: 10600500XX
# Sam Cunliffe (sam.cunliffe@desy.de), 2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsRadEEList():
    """
    Build the list of radiative electron pairs for photon systematics. In
    particular this is for the endcaps where we have no track triggers, we
    require one cluster-matched electron (the other is not required to match
    a cluster). No selection on the photon as the sample must be unbiased.

    As this retains a lot of bhabha events (by construction) we allow for
    prescaling (and prefer prescaled rather than a biased sampe by requiring
    any selection on the photon or too much of a cut on the recoil momentum)
    """
    #########################
    # PRESCALE for this skim
    prescale = '1.0'
    #########################

    # require a pair of good electrons one of which must be cluster-matched
    # with 3 GeV of energy
    goodtrack = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    cutAndCopyList('e+:skimtight', 'e+:all', goodtrack + ' and clusterE > 3.0')
    cutAndCopyList('e+:skimloose', 'e+:all', goodtrack)

    # a recoil momentum of the pair and that the recoil is within the CDC acceptance
    recoil = 'pRecoil > 0.075 and pRecoilTheta > 0.296706 and pRecoilTheta < 2.61799'
    reconstructDecay('vpho:radee -> e+:skimtight e-:skimloose', recoil)

    # apply event cuts (exactly two clean tracks in the event, and the prescale)
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    eventCuts += ' and eventRandom <= ' + prescale
    applyCuts('vpho:radee', eventCuts)

    return ['vpho:radee']
