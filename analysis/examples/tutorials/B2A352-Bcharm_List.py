#!/usr/bin/env/python3
# -*- coding: utf-8 -*-

#####################################
#
# Stuck? Ask for help at questions.belle2.org
#
#   Charm B Skim
# R.Cheaib, 30/05/2017
# This script  reconstructs  B mesons
# via semi-leptonic decays into D mesons.
# The reconstructed B mesons are appended to a particle list
# which is then written to the skim output udst file.
######################################

from basf2 import *
from modularAnalysis import *


def Bcharm():
    # apply momentum cut on the lepton lists.
    # the e-:all and mu-:all lists have already been called in the Bcharm_Skim_Standalone.py file.
    cutAndCopyList('e-:charm', 'e-:all', 'p>0.50', True)
    cutAndCopyList('mu-:charm', 'mu-:all', 'p>0.50', True)
# apply Mbc and delta E cuts on the B meson
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE)<0.5'
    # define the B+ channels
    Bpluschannels = ['D0:all e+:charm',
                     'D0:all mu+:charm',
                     'D*0:all e+:charm',
                     'D*0:all mu+:charm'
                     ]

    # define the B0 channels
    B0channels = ['D+:all e-:charm',
                  'D+:all mu-:charm',
                  'D*+:all e-:charm',
                  'D*+:all mu-:charm'
                  ]
    # define a bplus list and append all reconstructed B+ candidates to it
    bplusList = []
    for chID, channel in enumerate(Bpluschannels):
        reconstructDecay('B+:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        bplusList.append('B+:SL' + str(chID))
    # define a B0 list and append all reconstructed B0 candidates to it
    b0List = []
    for chID, channel in enumerate(B0channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        b0List.append('B0:SL' + str(chID))
    # combine B+ and B0 lists
    allLists = b0List + bplusList
    return allLists
