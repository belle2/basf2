#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# LFVZ'->visible skims
# Ilya Komarov Jun 2018
#
######################################################
from basf2 import *
from modularAnalysis import *


def LFVZpVisibleList():

    # This is skim  for ee->emuZ'(->emu) channel.

    lfvzp_list = []

    # Here we just want four gpood tracks to be reconstructed
    track_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 4'

    cutAndCopyList('e+:lfvzp', 'e+:all', track_cuts)

    # Z' to lfv: fully reconstructed
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp e-:lfvzp e-:lfvzp'

    reconstructDecay('vpho:vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis)

    lfvzp_list.append('vpho:vislfvzp')

    # Z' to lfv: part reco
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp e-:lfvzp'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 3'

    reconstructDecay('vpho:3tr_vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis)

    lfvzp_list.append('vpho:3tr_vislfvzp')

    # Z' to lfv: two same-sign tracks
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    reconstructDecay('vpho:2tr_vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis)

    lfvzp_list.append('vpho:2tr_vislfvzp')

    return lfvzp_list
