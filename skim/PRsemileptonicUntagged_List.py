#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Skim for partial reconstruction analysis
# L. Cremaldi, R. Cheaib, R. Godang 4/Oct/2017
#
######################################################


from basf2 import *
from modularAnalysis import *


def PRList():

    cutAndCopyList('e+:PR1', 'e+:all', 'useCMSFrame(p) > 1.50 and eid > 0.5')
    cutAndCopyList('mu+:PR1', 'mu+:all', 'useCMSFrame(p) > 1.50 and muid > 0.5')
    cutAndCopyList('pi-:PR1', 'pi-:all', 'piid>0.5 and muid<0.2 and 0.060<useCMSFrame(p)<0.220')

    cutAndCopyList('e+:PR2', 'e+:all', '0.600 < useCMSFrame(p) <= 1.50 and eid > 0.5')
    cutAndCopyList('mu+:PR2', 'mu+:all', '0.350 < useCMSFrame(p) <= 1.50 and muid > 0.5')
    cutAndCopyList('pi-:PR2', 'pi-:all', 'piid>0.5 and muid<0.2 and 0.060<useCMSFrame(p)<0.160')

    reconstructDecay('B0:L1 ->  pi-:PR1 e+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 1)
    applyCuts('B0:L1', 'R2EventLevel<0.5 and nTracks>4')

    reconstructDecay('B0:L2 ->  pi-:PR1 mu+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 2)
    applyCuts('B0:L2', 'R2EventLevel<0.5 and nTracks>4')

    reconstructDecay('B0:L3 ->  pi-:PR2 e+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 3)
    applyCuts('B0:L3', 'R2EventLevel<0.5 and nTracks>4')

    reconstructDecay('B0:L4 ->  pi-:PR2 mu+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 4)
    applyCuts('B0:L4', 'R2EventLevel<0.5')

    PRList = ['B0:L1', 'B0:L2']

    return PRList
