#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics skim(s) for eell events
# Skim code: 10600600
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsList():

    Lists = []
    Lists += EELLList()

    return Lists


def EELLList():

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
