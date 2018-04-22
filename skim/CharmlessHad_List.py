#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Had skims
# P. Urquijo, 6/Jan/2015
# C-L. Hsu 3/Jun/2016
# P. Urquijo 1/Oct/2016
######################################################

from basf2 import *
from modularAnalysis import *


def CharmlessHadList():
    btocharmlesscuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5 and nTracks>4'
    bd_PPChannels = ['pi0:skim pi0:skim',
                     'pi+:loose pi-:loose',
                     'K+:loose pi-:loose',
                     'pi+:loose K-:loose'
                     ]

    bd_PVChannels = ['rho+:loose pi-:loose',
                     'rho-:loose pi+:loose',
                     'K+:loose rho-:loose',
                     'rho+:loose K-:loose',
                     'rho0:loose pi0:skim',
                     'pi+:loose pi-:loose pi0:skim'
                     ]

    bd_VVChannels = ['rho+:loose rho-:loose',
                     'rho0:loose rho0:loose',
                     ]

    bu_PPChannels = ['pi-:loose pi0:skim',
                     'K-:loose pi0:skim'
                     ]

    bu_PVChannels = ['rho0:loose pi-:loose',
                     'rho-:loose pi0:skim'
                     ]

    bu_VVChannels = ['rho0:loose rho-:loose',
                     ]

    bu_PPPChannels = ['pi+:loose pi-:loose pi-:loose',
                      'K+:loose K-:loose pi-:loose',
                      'K+:loose K-:loose K-:loose',
                      'K+:loose pi-:loose pi-:loose',
                      'K-:loose pi+:loose pi-:loose',
                      'pi-:loose pi0:skim pi0:skim'
                      ]

    bdPPList = []
    for chID, channel in enumerate(bd_PPChannels):
        reconstructDecay('B0:HAD_b2PP' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        bdPPList.append('B0:HAD_b2PP' + str(chID))

    bdPVList = []
    for chID, channel in enumerate(bd_PVChannels):
        reconstructDecay('B0:HAD_b2PV' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        bdPVList.append('B0:HAD_b2PV' + str(chID))

    bdVVList = []
    for chID, channel in enumerate(bd_VVChannels):
        reconstructDecay('B0:HAD_b2VV' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        bdVVList.append('B0:HAD_b2VV' + str(chID))

    buPPList = []
    for chID, channel in enumerate(bu_PPChannels):
        reconstructDecay('B-:HAD_b2PP' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        buPPList.append('B-:HAD_b2PP' + str(chID))

    buPVList = []
    for chID, channel in enumerate(bu_PVChannels):
        reconstructDecay('B-:HAD_b2PV' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        buPVList.append('B-:HAD_b2PV' + str(chID))

    buVVList = []
    for chID, channel in enumerate(bu_VVChannels):
        reconstructDecay('B-:HAD_b2VV' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        buVVList.append('B-:HAD_b2VV' + str(chID))

    buPPPList = []
    for chID, channel in enumerate(bu_PPPChannels):
        reconstructDecay('B+:HAD_b2PPP' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        buPPPList.append('B+:HAD_b2PPP' + str(chID))

    hadLists = bdPPList + bdPVList + bdVVList + buPPList + buPVList + buVVList + buPPPList
    return hadLists
