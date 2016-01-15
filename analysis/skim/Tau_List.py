#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from basf2 import *
from modularAnalysis import *


def TauLFVList():
    tauLFVCuts = '1.58 < Mbc < 1.98 and abs(deltaE) < 1.0'

    tauLFVChannels = ['e+:loose gamma:all',
                      'mu+:loose gamma:all',
                      'e-:loose e-:loose e+:loose',
                      'mu-:loose mu-:loose mu+:loose',
                      'mu-:loose e-:loose e+:loose',
                      'e-:loose mu-:loose mu+:loose',
                      'e-:loose e-:loose mu+:loose',
                      'mu-:loose mu-:loose e+:loose',
                      'e+:loose pi0:all',
                      'mu+:loose pi0:all',
                      'e+:loose eta:loose',
                      'mu+:loose eta:loose',
                      'e+:loose eta\':loose',
                      'mu+:loose eta\':loose',
                      'e+:loose f_0:loose',
                      'mu+:loose f_0:loose',
                      'e+:loose rho0:loose',
                      'mu+:loose rho0:loose',
                      'e+:loose K*0:loose',
                      'mu+:loose K*0:loose',
                      'e+:loose anti-K*0:loose',
                      'mu+:loose anti-K*0:loose',
                      'e+:loose phi:loose',
                      'mu+:loose phi:loose',
                      'e+:loose omega:loose',
                      'mu+:loose omega:loose',
                      'e+:loose pi-:loose pi+:loose',
                      'mu+:loose pi-:loose pi+:loose',
                      'e-:loose pi+:loose pi+:loose',
                      'mu-:loose pi+:loose pi+:loose']

    tauLFVlist = []
    for chID, channel in enumerate(tauLFVChannels):
        reconstructDecay('tau+:LFV' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tauLFVlist.append('tau+:LFV' + str(chID))

    return tauLFVlist
