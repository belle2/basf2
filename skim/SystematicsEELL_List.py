#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Systematics skim list for eell events
Skim LFN code:   10600600
Physics channel: ee → eell
Skim category: systematics (lepton ID)
"""
__author__ = "Ilya Komarov"

from basf2 import *
from modularAnalysis import *


def SystematicsList():

    Lists = []
    Lists += EELLList()

    return Lists


def EELLList():
    """
    Systematics skim for ee --> eell events for lepton PID

    Returns:
        list name of the skim candidates
    """

    # At skim level we avoid any PID-like requirements and just select events
    # with two good tracks coming from the interavtion region.
    eLooseSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    cutAndCopyList('e+:skimloose', 'e+:all', eLooseSelection)

    # create a list of possible selections
    eelllist = []

    # Lepon pair has low invariant mass and tracks are back-to-back-like
    EELLSelection = 'M < 4 and useCMSFrame(daughterAngle(0,1)) < 0.75'
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    reconstructDecay('gamma:eell -> e+:skimloose e-:skimloose', EELLSelection + " and " + eventCuts)
    eelllist.append('gamma:eell')

    return eelllist
