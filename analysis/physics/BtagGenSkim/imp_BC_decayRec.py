#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (December 2014)
#               A. Polsak (December 2014)
######################################################

from basf2 import *
from modularAnalysis import applyCuts
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList

import sys


# define function for easier manipulation of copyLists

def copyListFun(mmin, mmax, listname):
    mlist = 'copyLists(\'' + listname + '\', ['
    for i in range(mmin, mmax):
        mlist = mlist + '\'B+:' + str(i) + '\','
    mlist = mlist + '\'B+:' + str(mmax) + '\'])'
    eval(mlist)


def reconstructBPD0YPS0(q):  # reconstruct B+ -> anti-D0 Y+(S=0) decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B+:1  -> anti-D0:all pi+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 1)
    reconstructDecay('B+:2  -> anti-D0:all pi+:all pi+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 2)
    reconstructDecay('B+:3  -> anti-D0:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 3)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B+:4  -> anti-D0:all pi+:all pi+:all pi-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 4)
    reconstructDecay('B+:5  -> anti-D0:all pi+:all pi-:all pi0:all pi0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 5)
    reconstructDecay('B+:6  -> anti-D0:all pi+:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 6)
    reconstructDecay('B+:7  -> anti-D0:all pi+:all pi-:all pi+:all pi-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 7)
    reconstructDecay('B+:8  -> anti-D0:all pi+:all K+:all K-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 8)
    reconstructDecay('B+:9  -> anti-D0:all K+:all K-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 9)
    reconstructDecay('B+:10 -> anti-D0:all pi+:all K+:all K-:all pi0:all pi0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 10)
    reconstructDecay('B+:11 -> anti-D0:all pi+:all pi-:all K+:all K-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 11)
    reconstructDecay('B+:12 -> anti-D0:all pi+:all K_S0:all K_S0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 12)
    reconstructDecay('B+:13 -> anti-D0:all pi+:all pi-:all K_S0:all K_S0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 13)
    reconstructDecay('B+:14 -> anti-D0:all pi+:all pi0:all pi0:all K_S0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 14)
    reconstructDecay('B+:15 -> anti-D0:all pi+:all K_S0:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 15)
    mmin = 1
    mmax = 15

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B+:16 -> anti-D0:all pi+:all K+:all K-:all pi0:all pi0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 16)
        mmax = 16

    # merge individual B+ lists into one list
    copyListFun(mmin, mmax, 'B+:antiD0Y+S0')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B+:antiD0Y+S0')
    # taking only correctly reconstructed B+
    applyCuts('B+:antiD0Y+S0', 'isSignal>0.5')
    # prevent double counting of different channels with same particle output
    applyCuts('B+:antiD0Y+S0',
              'abs(daughter(0,genMotherPDG)) < 423 or abs(daughter(0,genMotherPDG)) > 423'
              )


def reconstructBPD0YPS1(q):  # reconstruct B+ -> anti-D0 Y+(S=1) decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B+:17 -> anti-D0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 17)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B+:18 -> anti-D0:all pi+:all pi-:all K+:all pi0:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 18)
    reconstructDecay('B+:19 -> anti-D0:all pi+:all pi+:all pi-:all pi0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 19)
    reconstructDecay('B+:20 -> anti-D0:all pi+:all pi-:all K+:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 20)
    reconstructDecay('B+:21 -> anti-D0:all K+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 21)
    reconstructDecay('B+:22 -> anti-D0:all pi+:all pi+:all pi-:all pi-:all K+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 22)
    reconstructDecay('B+:23 -> anti-D0:all pi+:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 23)
    reconstructDecay('B+:24 -> anti-D0:all pi+:all pi0:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 24)
    reconstructDecay('B+:25 -> anti-D0:all pi+:all pi+:all pi-:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 25)
    mmin = 17
    mmax = 25

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B+:26 -> anti-D0:all pi+:all K+:all K-:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 26)
        reconstructDecay('B+:27 -> anti-D0:all pi+:all pi-:all K+:all K+:all K-:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 27)
        reconstructDecay('B+:28 -> anti-D0:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 28)
        reconstructDecay('B+:29 -> anti-D0:all pi+:all K+:all K-:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 29)
        reconstructDecay('B+:30 -> anti-D0:all K+:all pi0:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 30)
        reconstructDecay('B+:31 -> anti-D0:all pi-:all K+:all K+:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 31)
        reconstructDecay('B+:32 -> anti-D0:all K+:all K+:all K-:all pi0:all ',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 32)
        reconstructDecay('B+:33 -> anti-D0:all pi+:all pi-:all K+:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 33)
        reconstructDecay('B+:34 -> anti-D0:all K+:all K+:all K-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 34)
        reconstructDecay('B+:35 -> anti-D0:all K+:all K+:all K-:all pi0:all pi0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 35)
        reconstructDecay('B+:36 -> anti-D0:all pi-:all K+:all K+:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 36)
        reconstructDecay('B+:37 -> anti-D0:all K+:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 37)
        reconstructDecay('B+:38 -> anti-D0:all K+:all pi0:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 38)
        reconstructDecay('B+:39 -> anti-D0:all K+:all K_S0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 39)
        reconstructDecay('B+:40 -> anti-D0:all K+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 40)
        mmax = 40

    # merge individual B+ lists into one list
    copyListFun(mmin, mmax, 'B+:antiD0Y+S1')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B+:antiD0Y+S1')
    # taking only correctly reconstructed B+
    applyCuts('B+:antiD0Y+S1', 'isSignal>0.5')
    # prevent double counting of different channels with same particle output
    applyCuts('B+:antiD0Y+S1',
              'abs(daughter(0,genMotherPDG)) < 423 or abs(daughter(0,genMotherPDG)) > 423'
              )


def reconstructBPDST0YPS0(q):  # reconstruct B+ -> anti-D*0 Y+(S=0) decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B+:41  -> anti-D*0:all pi+:all pi+:all pi-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 41)
    reconstructDecay('B+:42  -> anti-D*0:all pi+:all pi+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 42)
    reconstructDecay('B+:43  -> anti-D*0:all pi+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 43)
    reconstructDecay('B+:44  -> anti-D*0:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 44)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B+:45  -> anti-D*0:all pi+:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 45)
    reconstructDecay('B+:46  -> anti-D*0:all pi+:all pi-:all pi0:all pi0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 46)
    reconstructDecay('B+:47  -> anti-D*0:all pi+:all pi-:all pi+:all pi-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 47)
    reconstructDecay('B+:48  -> anti-D*0:all pi+:all K+:all K-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 48)
    reconstructDecay('B+:49  -> anti-D*0:all K+:all K-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 49)
    reconstructDecay('B+:50  -> anti-D*0:all pi+:all K+:all K-:all pi0:all pi0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 50)
    reconstructDecay('B+:51  -> anti-D*0:all pi+:all pi-:all K+:all K-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 51)
    reconstructDecay('B+:52  -> anti-D*0:all pi+:all K_S0:all K_S0:all pi0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 52)
    reconstructDecay('B+:53  -> anti-D*0:all pi+:all pi0:all pi0:all K_S0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 53)
    reconstructDecay('B+:54  -> anti-D*0:all pi+:all pi-:all K_S0:all K_S0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 54)
    reconstructDecay('B+:55  -> anti-D*0:all pi+:all K_S0:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 55)
    mmin = 41
    mmax = 55

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B+:56  -> anti-D*0:all pi+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 56)
        mmax = 56

    # merge individual B+ lists into one list
    copyListFun(mmin, mmax, 'B+:antiD*0Y+S0')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B+:antiD*0Y+S0')
    # taking only correctly reconstructed B+
    applyCuts('B+:antiD*0Y+S0', 'isSignal>0.5')


def reconstructBPDST0YPS1(q):  # reconstruct B+ -> anti-D*0 Y+(S=1) decays
    # decay modes in Full Event Interpretation Report (2014)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B+:57 -> anti-D*0:all pi+:all pi-:all K+:all pi0:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 57)
    reconstructDecay('B+:58 -> anti-D*0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 58)
    reconstructDecay('B+:59 -> anti-D*0:all K+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 59)
    reconstructDecay('B+:60 -> anti-D*0:all pi+:all pi+:all pi-:all pi0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 60)
    reconstructDecay('B+:61 -> anti-D*0:all pi+:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 61)
    reconstructDecay('B+:62 -> anti-D*0:all pi+:all pi-:all K+:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 62)
    mmin = 57
    mmax = 62

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B+:63 -> anti-D*0:all pi+:all pi0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 63)
        reconstructDecay('B+:64 -> anti-D*0:all pi+:all pi0:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 64)
        reconstructDecay('B+:65 -> anti-D*0:all pi+:all pi+:all pi-:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 65)
        reconstructDecay('B+:66 -> anti-D*0:all pi+:all pi+:all pi-:all pi-:all K+:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 66)
        reconstructDecay('B+:67 -> anti-D*0:all pi+:all K+:all K-:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 67)
        reconstructDecay('B+:68 -> anti-D*0:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 68)
        reconstructDecay('B+:69 -> anti-D*0:all pi+:all K+:all K-:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 69)
        reconstructDecay('B+:70 -> anti-D*0:all pi+:all pi-:all K+:all K+:all K-:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 70)
        reconstructDecay('B+:71 -> anti-D*0:all K+:all K+:all K-:all pi0:all '
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 71)
        reconstructDecay('B+:72 -> anti-D*0:all pi-:all K+:all K+:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 72)
        reconstructDecay('B+:73 -> anti-D*0:all K+:all K+:all K-:all pi0:all pi0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 73)
        reconstructDecay('B+:74 -> anti-D*0:all K+:all K+:all K-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 74)
        reconstructDecay('B+:75 -> anti-D*0:all pi+:all pi-:all K+:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 75)
        reconstructDecay('B+:76 -> anti-D*0:all pi-:all K+:all K+:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 76)
        reconstructDecay('B+:77 -> anti-D*0:all K+:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 77)
        reconstructDecay('B+:78 -> anti-D*0:all K+:all pi0:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 78)
        reconstructDecay('B+:79 -> anti-D*0:all K+:all K_S0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 79)
        reconstructDecay('B+:80 -> anti-D*0:all K+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 80)
        mmax = 80

    # merge individual B+ lists into one list
    copyListFun(mmin, mmax, 'B+:antiD*0Y+S1')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B+:antiD*0Y+S1')
    # taking only correctly reconstructed B+
    applyCuts('B+:antiD*0Y+S1', 'isSignal>0.5')


def reconstructBPccbarYPS1(q):  # reconstruct B+ -> ccbar Y+(S=1) decays
    # experimentally clean decay modes
    reconstructDecay('B+:81 -> J/psi:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 81)
    reconstructDecay('B+:82 -> J/psi:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 82)
    reconstructDecay('B+:83 -> J/psi:all K_S0:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 83)
    reconstructDecay('B+:84 -> J/psi:all K+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 84)
    reconstructDecay('B+:85 -> J/psi:all K+:all pi-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 85)
    reconstructDecay('B+:86 -> J/psi:all K+:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 86)
    reconstructDecay('B+:87 -> J/psi:all K_S0:all pi0:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 87)

    reconstructDecay('B+:88 -> psi(2S):all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 88)
    reconstructDecay('B+:89 -> psi(2S):all K_S0:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 89)
    reconstructDecay('B+:90 -> psi(2S):all K+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 90)
    reconstructDecay('B+:91 -> psi(2S):all K+:all pi-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 91)
    reconstructDecay('B+:92 -> psi(2S):all K_S0:all pi0:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 92)
    reconstructDecay('B+:93 -> psi(2S):all K+:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 93)
    mmin = 81
    mmax = 93

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B+:94 -> chi_c0:all K+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 94)
        reconstructDecay('B+:95 -> chi_c0:all K_S0:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 95)
        reconstructDecay('B+:96 -> chi_c0:all K+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 96)
        reconstructDecay('B+:97 -> chi_c0:all K+:all pi-:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 97)
        reconstructDecay('B+:98 -> chi_c0:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 98)
        reconstructDecay('B+:99 -> chi_c0:all K_S0:all pi+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 99)

        reconstructDecay('B+:100 -> chi_c1:all K+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 100)
        reconstructDecay('B+:101 -> chi_c1:all K_S0:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 101)
        reconstructDecay('B+:102 -> chi_c1:all K+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 102)
        reconstructDecay('B+:103 -> chi_c1:all K+:all pi-:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 103)
        reconstructDecay('B+:104 -> chi_c1:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 104)
        reconstructDecay('B+:105 -> chi_c1:all K_S0:all pi+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 105)

        reconstructDecay('B+:106 -> chi_c2:all K+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 106)
        reconstructDecay('B+:107 -> chi_c2:all K_S0:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 107)
        reconstructDecay('B+:108 -> chi_c2:all K+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 108)
        reconstructDecay('B+:109 -> chi_c2:all K+:all pi-:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 109)
        reconstructDecay('B+:110 -> chi_c2:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 110)
        reconstructDecay('B+:111 -> chi_c2:all K_S0:all pi+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 111)
        mmax = 111

    # merge individual B+ lists into one list
    copyListFun(mmin, mmax, 'B+:ccbarYPS1')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B+:ccbarYPS1')
    # taking only correctly reconstructed B+
    applyCuts('B+:ccbarYPS1', 'isSignal>0.5')


def reconstructBPmisc(q):  # miscellaneous modes
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B+:112 -> anti-D*0:all D*0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 112)
    reconstructDecay('B+:113 -> anti-D0:all D*0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 113)
    reconstructDecay('B+:114 -> anti-D*0:all D0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 114)
    reconstructDecay('B+:115 -> anti-D0:all D0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 115)
    reconstructDecay('B+:116 -> anti-D0:all D_s+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 116)
    reconstructDecay('B+:117 -> anti-D*0:all D_s+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 117)
    reconstructDecay('B+:118 -> anti-D0:all D_s*+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 118)
    reconstructDecay('B+:119 -> anti-D0:all D+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 119)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B+:120 -> D-:all pi+:all pi+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 120)
    reconstructDecay('B+:121 -> anti-D*0:all D_s*+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 121)
    reconstructDecay('B+:122 -> D-:all pi+:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 122)
    mmin = 112
    mmax = 122

    # merge individual B+ lists into one list
    copyListFun(mmin, mmax, 'B+:misc')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B+:misc')
    # taking only correctly reconstructed B+
    applyCuts('B+:misc', 'isSignal>0.5')


