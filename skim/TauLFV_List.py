#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skims
# P. Urquijo, 6/Jan/2016
# update 9/9/2016
#
######################################################
from basf2 import *
from modularAnalysis import *
from stdV0s import stdKshorts


def TauLFVList():
    tauLFVCuts = '1.58 < Mbc < 1.98 and abs(deltaE) < 1.0'

    tau_lgamma_Channels = ['e+:loose gamma:loose',
                           'mu+:loose gamma:loose'
                           ]

    tau_lll_Channels = ['e-:loose e-:loose e+:loose',
                        'mu-:loose mu-:loose mu+:loose',
                        'mu-:loose e-:loose e+:loose',
                        'e-:loose mu-:loose mu+:loose',
                        'e-:loose e-:loose mu+:loose',
                        'mu-:loose mu-:loose e+:loose'
                        ]

    tau_lP0_Channels = ['e+:loose pi0:skim',
                        'mu+:loose pi0:skim',
                        'e+:loose eta:loose',
                        'mu+:loose eta:loose',
                        'e+:loose eta\':loose',
                        'mu+:loose eta\':loose',
                        'e+:loose K_S0:all',
                        'mu+:loose K_S0:all'
                        ]

    tau_lS0_Channels = ['e+:loose f_0:loose',
                        'mu+:loose f_0:loose'
                        ]

    tau_lV0_Channels = ['e+:loose rho0:loose',
                        'mu+:loose rho0:loose',
                        'e+:loose K*0:loose',
                        'mu+:loose K*0:loose',
                        'e+:loose anti-K*0:loose',
                        'mu+:loose anti-K*0:loose',
                        'e+:loose phi:loose',
                        'mu+:loose phi:loose',
                        'e+:loose omega:loose',
                        'mu+:loose omega:loose'
                        ]

    tau_lhh_Channels = ['e+:loose pi-:loose pi+:loose',
                        'mu+:loose pi-:loose pi+:loose',
                        'e-:loose pi+:loose pi+:loose',
                        'mu-:loose pi+:loose pi+:loose',
                        'e+:loose K-:loose K+:loose',
                        'mu+:loose K-:loose K+:loose',
                        'e-:loose K+:loose K+:loose',
                        'mu-:loose K+:loose K+:loose',
                        'e+:loose K-:loose pi+:loose',
                        'mu+:loose K-:loose pi+:loose',
                        'e-:loose K+:loose pi+:loose',
                        'mu-:loose K+:loose pi+:loose',
                        'e-:loose K_S0:all K_S0:all',
                        'mu-:loose K_S0:all K_S0:all'
                        ]

    tau_bnv_Channels = ['mu+:loose mu+:loose anti-p-:loose',
                        'mu-:loose mu+:loose p+:loose',
                        'anti-p-:loose pi+:loose pi+:loose',
                        'p+:loose pi-:loose pi+:loose',
                        'anti-p-:loose pi+:loose K+:loose',
                        'p+:loose pi-:loose K+:loose'
                        ]

    tau_lgamma_list = []
    for chID, channel in enumerate(tau_lgamma_Channels):
        reconstructDecay('tau+:LFV_lgamma' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lgamma_list.append('tau+:LFV_lgamma' + str(chID))

    tau_lll_list = []
    for chID, channel in enumerate(tau_lll_Channels):
        reconstructDecay('tau+:LFV_lll' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lll_list.append('tau+:LFV_lll' + str(chID))

    tau_lP0_list = []
    for chID, channel in enumerate(tau_lP0_Channels):
        reconstructDecay('tau+:LFV_lP0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lP0_list.append('tau+:LFV_lP0' + str(chID))

    tau_lS0_list = []
    for chID, channel in enumerate(tau_lS0_Channels):
        reconstructDecay('tau+:LFV_lS0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lS0_list.append('tau+:LFV_lS0' + str(chID))

    tau_lV0_list = []
    for chID, channel in enumerate(tau_lV0_Channels):
        reconstructDecay('tau+:LFV_lV0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lV0_list.append('tau+:LFV_lV0' + str(chID))

    tau_lhh_list = []
    for chID, channel in enumerate(tau_lhh_Channels):
        reconstructDecay('tau+:LFV_lhh' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lhh_list.append('tau+:LFV_lhh' + str(chID))

    tau_bnv_list = []
    for chID, channel in enumerate(tau_bnv_Channels):
        reconstructDecay('tau+:LFV_bnv' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_bnv_list.append('tau+:LFV_bnv' + str(chID))

    tau_lfv_lists = tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list
    return tau_lfv_lists
