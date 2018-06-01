#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# LFVZ' skims
# Ilya Komarov Jun 2018
#
######################################################
from basf2 import *
from modularAnalysis import *


def LFVZpVisibleList():

    lfvzp_list = []

    # Here we just want four gpood tracks to be reconstructed
    track_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 4'

    cutAndCopyList('mu+:lfvzp', 'mu+:all', track_cuts)
    cutAndCopyList('e+:lfvzp', 'e+:all', track_cuts)

    # Z' to lfv
    LFVZpVisChannel = 'mu+:lfvzp mu+:lfvzp e-:lfvzp e-:lfvzp'

    reconstructDecay('vpho:vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis)

    lfvzp_list.append('vpho:vislfvzp')

    return lfvzp_list
