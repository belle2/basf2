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


def SystematicsRadEEList(prescale_all='0.2', prescale_fwd_electron='0.02'):
    """
    Build the list of radiative electron pairs for photon systematics. In
    particular this is for the endcaps where we have no track triggers, we
    require one cluster-matched electron (the other is not required to match
    a cluster). No selection on the photon as the sample must be unbiased.

    As this retains a lot of bhabha events (by construction) we allow for
    prescaling (and prefer prescaled rather than a biased sampe by requiring
    any selection on the photon or too much of a cut on the recoil momentum)

    Parameters:
        prescale (str): the prescale for this skim

    Returns:
        list name of the skim candidates
    """
    # require a pair of good electrons one of which must be cluster-matched
    # with 3 GeV of energy
    goodtrack = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    goodtrackwithcluster = '%s and clusterE > 3.0' % goodtrack
    cutAndCopyList('e+:skimtight', 'e+:all', goodtrackwithcluster)
    cutAndCopyList('e+:skimloose', 'e+:all', goodtrack)

    # a minimum momentum of 75 MeV/c recoiling against the pair,
    # and require that the recoil is within the CDC acceptance
    recoil = 'pRecoil > 0.075 and 0.296706 < pRecoilTheta < 2.61799'  # GeV/c, rad
    reconstructDecay('vpho:radee -> e+:skimtight e-:skimloose', recoil)

    # apply event cuts (exactly two clean tracks in the event, and prescale
    # the whole event regardless of where the electron went)
    event_cuts = '[nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2]'  # cm, cm
    event_cuts += ' and [eventRandom <= %s]' % prescale_all

    # now prescale the *electron* (e-) in the forward endcap (for bhabhas)
    # note this is all done with cut strings to circumnavigate BII-3607
    fwd_encap_border = '0.5480334'  # rad (31.4 deg)
    electron_is_first = 'daughter(0, charge) < 0'
    first_in_fwd_endcap = 'daughter(0, Theta) < %s' % fwd_encap_border
    first_not_in_fwd_endcap = 'daughter(0, Theta) > %s' % fwd_encap_border
    electron_is_second = 'daughter(1, charge) < 0'
    second_in_fwd_endcap = 'daughter(1, Theta) < %s' % fwd_encap_border
    second_not_in_fwd_endcap = 'daughter(1, Theta) > %s' % fwd_encap_border
    passes_prescale = 'eventRandom <= %s' % prescale_fwd_electron
    #
    # four possible scenarios:
    # 1) electron first in the decaystring and in fwd endcap: prescale these
    prescale_logic = '[%s and %s and %s]' \
        % (electron_is_first, first_in_fwd_endcap, passes_prescale)
    # 2) electron second in string and in fwd endcap: prescale these
    prescale_logic += ' or [%s and %s and %s]' \
        % (electron_is_second, second_in_fwd_endcap, passes_prescale)
    # 3) electron first in string and not in fwd endcap (no prescale)
    prescale_logic += ' or [%s and %s]' % (electron_is_first, first_not_in_fwd_endcap)
    # 4) electron second in string and not in fwd endcap (no prescale)
    prescale_logic += ' or [%s and %s]' % (electron_is_second, second_not_in_fwd_endcap)

    # final candidate building with cuts and prescales
    prescale_logic = '[%s]' % prescale_logic
    applyCuts('vpho:radee', event_cuts + ' and ' + prescale_logic)
    return ['vpho:radee']
