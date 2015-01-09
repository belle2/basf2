#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (December 2014)
#               A. Polsak (December 2014)
#
######################################################

from basf2 import *
from modularAnalysis import applyCuts
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList

import sys


# define function for easier manipulation of copyLists

def copyListFun(
    mmin,
    mmax,
    part,
    listname,
    ):

    mlist = 'copyLists(\'' + listname + '\', ['
    for i in range(mmin, mmax):
        mlist = mlist + '\'' + part + ':' + str(i) + '\','
    mlist = mlist + '\'' + part + ':' + str(mmax) + '\'])'
    eval(mlist)


def reconstructD0():  # reconstruct D0 decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('D0:1 -> K-:all pi+:all', '1.8 < M < 1.92', 1)
    reconstructDecay('D0:2 -> K-:all  pi+:all pi0:all', '1.8 < M < 1.92', 2)
    reconstructDecay('D0:3 -> K-:all  pi+:all pi0:all pi0:all',
                     '1.8 < M < 1.92', 3)
    reconstructDecay('D0:4 -> K-:all  pi+:all pi+:all pi-:all',
                     '1.8 < M < 1.92', 4)
    reconstructDecay('D0:5 -> K-:all  pi+:all pi+:all pi-:all pi0:all',
                     '1.8 < M < 1.92', 5)
    reconstructDecay('D0:6 -> pi-:all pi+:all', '1.8 < M < 1.92', 6)
    reconstructDecay('D0:7 -> pi+:all  pi-:all pi+:all pi-:all',
                     '1.8 < M < 1.92', 7)
    reconstructDecay('D0:8 -> pi+:all  pi-:all pi0:all', '1.8 < M < 1.92', 8)
    reconstructDecay('D0:9 -> pi+:all  pi-:all pi0:all pi0:all',
                     '1.8 < M < 1.92', 9)
    reconstructDecay('D0:10 -> K_S0:all pi0:all', '1.8 < M < 1.92', 10)
    reconstructDecay('D0:11 -> K_S0:all pi+:all pi-:all', '1.8 < M < 1.92', 11)
    reconstructDecay('D0:12 -> K_S0:all pi+:all pi-:all pi0:all',
                     '1.8 < M < 1.92', 12)
    reconstructDecay('D0:13 -> K-:all K+:all', '1.8 < M < 1.92', 13)
    reconstructDecay('D0:14 -> K-:all K+:all pi0:all', '1.8 < M < 1.92', 14)
    reconstructDecay('D0:15 -> K_S0:all K-:all K+:all', '1.8 < M < 1.92', 15)

    # rest of events
    reconstructDecay('D0:16 -> K-:all K+:all pi-:all pi+:all', '1.8 < M < 1.92'
                     , 16)
    mmin = 1
    mmax = 16

    # merge individual D0/D+ lists into one list for D0/D+
    copyListFun(mmin, mmax, 'D0', 'D0:all')
    # perform MC matching (MC truth asociation)
    matchMCTruth('D0:all')
    # taking only correctly reconstructed D0
    applyCuts('D0:all', 'isSignal>0.5')


def reconstructDP():  # reconstruct D+ decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('D+:1 -> K-:all pi+:all pi+:all', '1.8 < M < 1.92', 1)
    reconstructDecay('D+:2 -> K-:all pi+:all pi+:all pi0:all', '1.8 < M < 1.92'
                     , 2)
    reconstructDecay('D+:3 -> K-:all K+:all pi+:all', '1.8 < M < 1.92', 3)
    reconstructDecay('D+:4 -> K-:all K+:all pi+:all pi0:all', '1.8 < M < 1.92'
                     , 4)
    reconstructDecay('D+:5 -> pi+:all pi0:all', '1.8 < M < 1.92', 5)
    reconstructDecay('D+:6 -> pi+:all pi+:all pi-:all', '1.8 < M < 1.92', 6)
    reconstructDecay('D+:7 -> pi+:all pi+:all pi-:all pi0:all',
                     '1.8 < M < 1.92', 7)
    reconstructDecay('D+:8 -> K_S0:all pi+:all', '1.8 < M < 1.92', 8)
    reconstructDecay('D+:9 -> K_S0:all pi+:all pi0:all', '1.8 < M < 1.92', 9)
    reconstructDecay('D+:10 -> K_S0:all pi+:all pi+:all pi-:all',
                     '1.8 < M < 1.92', 10)
    reconstructDecay('D+:11 -> K_S0:all K_S0:all K+:all', '1.8 < M < 1.92', 11)
    mmin = 1
    mmax = 11

    # merge individual D0/D+ lists into one list for D0/D+
    copyListFun(mmin, mmax, 'D+', 'D+:all')
    # perform MC matching (MC truth asociation)
    matchMCTruth('D+:all')
    # taking only correctly reconstructed D+
    applyCuts('D+:all', 'isSignal>0.5')


def reconstructDSTP():  # reconstruct D*+ decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('D*+:1 -> D0:all pi+:all', 'massDifference(0) < 0.16', 1)
    reconstructDecay('D*+:2 -> D+:all pi0:all', 'massDifference(0) < 0.16', 2)
    reconstructDecay('D*+:3 -> D+:all gamma:all', 'massDifference(0) < 0.16',
                     3)
    mmin = 1
    mmax = 3

    # merge individual D*+ lists into one list for D*+
    copyListFun(mmin, mmax, 'D*+', 'D*+:all')
    # perform MC matching (MC truth asociation)
    matchMCTruth('D*+:all')
    # taking only correctly reconstructed D*+
    applyCuts('D*+:all', 'isSignal>0.5')


def reconstructDST0():  # reconstruct D*0 decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('D*0:1 -> D0:all pi0:all', 'massDifference(0) < 0.16', 1)
    reconstructDecay('D*0:2 -> D0:all gamma:all', 'massDifference(0) < 0.16',
                     2)
    mmin = 1
    mmax = 2
    # merge individual D*0 lists into one list for D*0
    copyListFun(mmin, mmax, 'D*0', 'D*0:all')
    # perform MC matching (MC truth asociation)
    matchMCTruth('D*0:all')
    # taking only correctly reconstructed D*0
    applyCuts('D*0:all', 'isSignal>0.5')


def reconstructDSP():  # reconstruct D_s+ decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('D_s+:1 -> K+:all K_S0:all', '1.91 < M < 2.03', 1)
    reconstructDecay('D_s+:2 -> K+:all pi+:all pi-:all', '1.91 < M < 2.03', 2)
    reconstructDecay('D_s+:3 -> K+:all K-:all pi+:all', '1.91 < M < 2.03', 3)
    reconstructDecay('D_s+:4 -> K+:all K-:all pi+:all pi0:all',
                     '1.91 < M < 2.03', 4)
    reconstructDecay('D_s+:5 -> K+:all K_S0:all pi+:all pi-:all',
                     '1.91 < M < 2.03', 5)
    reconstructDecay('D_s+:6 -> K-:all K_S0:all pi+:all pi+:all',
                     '1.91 < M < 2.03', 6)
    reconstructDecay('D_s+:7 -> K+:all K-:all pi+:all pi+:all pi-:all',
                     '1.91 < M < 2.03', 7)
    reconstructDecay('D_s+:8 -> pi+:all pi+:all pi-:all', '1.91 < M < 2.03', 8)
    reconstructDecay('D_s+:9 -> K_S0:all pi+:all', '1.91 < M < 2.03', 9)
    reconstructDecay('D_s+:10 -> K_S0:all pi+:all pi0:all', '1.91 < M < 2.03',
                     10)
    mmin = 1
    mmax = 10

    # merge individual D*0 lists into one list for D*0
    copyListFun(mmin, mmax, 'D_s+', 'D_s+:all')
    # perform MC matching (MC truth asociation),
    matchMCTruth('D_s+:all')
    # taking only correctly reconstructed D*0
    applyCuts('D_s+:all', 'isSignal>0.5')


def reconstructDSSTP():  # reconstruct D_s*+ decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('D_s*+:1 -> D_s+:all gamma:all',
                     'massDifference(0) < 0.16', 1)
    reconstructDecay('D_s*+:2 -> D_s+:all pi0:all', 'massDifference(0) < 0.16'
                     , 2)
    mmin = 1
    mmax = 2

    # merge individual D*0 lists into one list for
    copyListFun(mmin, mmax, 'D_s*+', 'D_s*+:all')
    # perform MC matching (MC truth asociation),
    matchMCTruth('D_s*+:all')
    # taking only correctly reconstructed
    applyCuts('D_s*+:all', 'isSignal>0.5')


