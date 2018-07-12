#!/usr/bin/env/python3
# -*- coding: utf-8 -*-

#####################################
#
# Stuck? Ask for help at questions.belle2.org
#
#   Hadronic B Skim
# R.Cheaib, 30/05/2017
#
# This script illustrates an example of how a skimming script should be
# written.
# This skim script reconstructs hadronic B candidates from the decay to a D meson#and a pion.
######################################

from basf2 import *
from modularAnalysis import *

# Here you define the particle list you want to reconstruct
# First we reconstruct the D meson


def D0ToYpZm():
    # define the D meson mass cut
    Dcuts = '1.8 < InvM <1.9'
    # below are the channels for the D0 decay
    D0channels = ['K-:loose  pi+:loose',
                  'pi+:loose pi-:loose'
                  ]
    # here we define the D0 list, and assign an ID to each channel that is being reconstructed
    D0List = []
    for chID, channel in enumerate(D0channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def BHadronic():
    # define the B meson cuts
    Bcuts = '5.24<Mbc<5.29 '
    # call the function to reconstruct the D0 meson and return the D0 meson list
    D0List = D0ToYpZm()
    # define the B meson list
    BList = []
    # reconstruct the different channels for the B meson and append to the B meson list
    for chID, channel in enumerate(D0List):
        reconstructDecay('B+:' + str(chID) + ' -> pi+:all ' + channel, Bcuts, chID)
    copyLists('B+:all', ['B+:0', 'B+:1'])
# return the B meson list
    BList.append('B+:all')
    return BList
