#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# LFVZ'->invisible skim
# Ilya Komarov Jun 2018
#
######################################################
from basf2 import *
from modularAnalysis import *


def LFVZpInvisibleList():

    # This is skim for ee->emuZ'(->invisible) channel.

    lfvzp_list = []

    # Some loose PID cuts for leptons
    muID_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pidProbabilityExpert(13, all) > 0.1'
    eID_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pidProbabilityExpert(11, all) > 0.1'

    # We want exaclty 2 good tracks
    Event_cuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'

    cutAndCopyList('mu+:lfvzp', 'mu+:all', muID_cuts)
    cutAndCopyList('e+:lfvzp', 'e+:all', eID_cuts)

    # Z' to invisible
    LFVZpInvChannel = 'mu+:lfvzp e-:lfvzp'
    reconstructDecay('vpho:invlfvzp -> ' + LFVZpInvChannel, Event_cuts)
    lfvzp_list.append('vpho:invlfvzp')

    return lfvzp_list
