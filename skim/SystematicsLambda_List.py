#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Sysetmatics Skims
# P. Urquijo, 1/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsLambdaList():

    Lists = []
    Lists += PFromLambdaList()

    return Lists


def PFromLambdaList():
    LambdaCuts = 'M < 1.2'

    fillParticleList('p+:all', '', enforceFitHypothesis=True)
    fillParticleList('pi-:all', '', enforceFitHypothesis=True)
    LambdaChannel = ['p+:all pi-:loose']

    LambdaList = []
    for chID, channel in enumerate(LambdaChannel):
        reconstructDecay('Lambda0:syst' + str(chID) + ' -> ' + channel, LambdaCuts, chID)
        vertexKFit('Lambda0:syst' + str(chID), 0.002)
        applyCuts('Lambda0:syst' + str(chID), '1.10<M<1.13')
        applyCuts('Lambda0:syst' + str(chID), 'formula(x*x+y*y)>0.0225')
        applyCuts('Lambda0:syst' + str(chID), 'formula(x*px+y*py)>0')
        applyCuts('Lambda0:syst' + str(chID), 'formula([x*px*x*px+2*x*px*y*py+y*py*y*py]/[[px*px+py*py]*[x*x+y*y]])>0.994009')
        applyCuts('Lambda0:syst' + str(chID), 'p>0.2')
        matchMCTruth('Lambda0:syst0')
        LambdaList.append('Lambda0:syst' + str(chID))

    return LambdaList
