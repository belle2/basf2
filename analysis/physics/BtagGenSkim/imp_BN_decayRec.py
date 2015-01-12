#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
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
        mlist = mlist + '\'B0:' + str(i) + '\','
    mlist = mlist + '\'B0:' + str(mmax) + '\'])'
    eval(mlist)


def reconstructB0DMYPS0(q):  # reconstruct B0 -> D- Y+(S=0) decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B0:1  -> D-:all pi+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 1)
    reconstructDecay('B0:2  -> D-:all pi+:all pi+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 2)
    reconstructDecay('B0:3  -> D-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 3)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B0:4  -> D-:all pi+:all pi+:all pi-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 4)
    reconstructDecay('B0:5  -> D-:all pi+:all pi-:all pi0:all pi0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 5)
    reconstructDecay('B0:6  -> D-:all pi+:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 6)
    reconstructDecay('B0:7  -> D-:all pi+:all K+:all K-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 7)
    reconstructDecay('B0:8  -> D-:all K+:all K-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 8)
    reconstructDecay('B0:9  -> D-:all pi+:all K+:all K-:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 9)
    reconstructDecay('B0:10 -> D-:all pi+:all pi-:all pi+:all pi-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 10)
    reconstructDecay('B0:11 -> D-:all pi+:all pi-:all K+:all K-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 11)
    reconstructDecay('B0:12 -> D-:all pi+:all K_S0:all K_S0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 12)
    reconstructDecay('B0:13 -> D-:all pi+:all pi0:all pi0:all K_S0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 13)
    reconstructDecay('B0:14 -> D-:all pi+:all K_S0:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 14)
    reconstructDecay('B0:15 -> D-:all pi+:all pi-:all K_S0:all K_S0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 15)
    mmin = 1
    mmax = 15

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B0:16 -> D-:all pi+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 16)
        mmax = 16

    # merge individual B0 lists into one list
    copyListFun(mmin, mmax, 'B0:D-Y+S0')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B0:D-Y+S0')
    # taking only correctly reconstructed B0
    applyCuts('B0:D-Y+S0', 'isSignal>0.5')
    # prevent double counting of different channels with same output particles
    applyCuts('B0:D-Y+S0',
              'abs(daughter(0,genMotherPDG)) < 413 or abs(daughter(0,genMotherPDG)) > 413'
              )


def reconstructB0DMYPS1(q):  # reconstruct B0 -> D- Y+(S=1) decays
    # decay modes in Full Event Interpretation Report (2014)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B0:17 -> D-:all pi+:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 17)
    reconstructDecay('B0:18 -> D-:all pi+:all pi-:all K+:all pi0:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 18)
    reconstructDecay('B0:19 -> D-:all pi+:all pi+:all pi-:all pi0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 19)
    mmin = 17
    mmax = 19

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B0:20 -> D-:all pi+:all pi-:all K+:all ',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 20)
        reconstructDecay('B0:21 -> D-:all K+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 21)
        reconstructDecay('B0:22 -> D-:all K+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 22)
        reconstructDecay('B0:23 -> D-:all pi+:all pi0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 23)
        reconstructDecay('B0:24 -> D-:all pi+:all pi0:all pi0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 24)
        reconstructDecay('B0:25 -> D-:all pi+:all pi+:all pi-:all pi-:all K+:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 25)
        reconstructDecay('B0:26 -> D-:all pi+:all pi+:all pi-:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 26)
        reconstructDecay('B0:27 -> D-:all pi+:all K+:all K-:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 27)
        reconstructDecay('B0:28 -> D-:all pi+:all pi-:all K+:all K+:all K-:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 28)
        reconstructDecay('B0:29 -> D-:all pi+:all K+:all K-:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 29)
        reconstructDecay('B0:30 -> D-:all pi-:all K+:all K+:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 30)
        reconstructDecay('B0:31 -> D-:all pi+:all pi-:all K+:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 31)
        reconstructDecay('B0:32 -> D-:all K+:all K+:all K-:all pi0:all ',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 32)
        reconstructDecay('B0:33 -> D-:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 33)
        reconstructDecay('B0:34 -> D-:all K+:all K+:all K-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 34)
        reconstructDecay('B0:35 -> D-:all K+:all K+:all K-:all pi0:all pi0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 35)
        reconstructDecay('B0:36 -> D-:all K+:all pi0:all K_S0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 36)
        reconstructDecay('B0:37 -> D-:all pi-:all K+:all K+:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 37)
        reconstructDecay('B0:38 -> D-:all K+:all pi0:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 38)
        reconstructDecay('B0:39 -> D-:all K+:all K_S0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 39)
        reconstructDecay('B0:40 -> D-:all K+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 40)
        mmin = 40

    # merge individual B0 lists into one list
    copyListFun(mmin, mmax, 'B0:D-Y+S1')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B0:D-Y+S1')
    # taking only correctly reconstructed B0
    applyCuts('B0:D-Y+S1', 'isSignal>0.5')
    # prevent double counting of different channels with same output particles
    applyCuts('B0:D-Y+S1',
              'abs(daughter(0,genMotherPDG)) < 413 or abs(daughter(0,genMotherPDG)) > 413'
              )


def reconstructB0DSTMYPS0(q):  # reconstruct B0 -> D*- Y+(S=0) decays
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B0:41  -> D*-:all pi+:all pi+:all pi-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 41)
    reconstructDecay('B0:42  -> D*-:all pi+:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 42)
    reconstructDecay('B0:43  -> D*-:all pi+:all pi+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 43)
    reconstructDecay('B0:44  -> D*-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 44)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B0:45  -> D*-:all pi+:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 45)
    reconstructDecay('B0:46  -> D*-:all pi+:all pi-:all pi0:all pi0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 46)
    reconstructDecay('B0:47  -> D*-:all pi+:all pi-:all pi+:all pi-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 47)
    reconstructDecay('B0:48  -> D*-:all pi+:all K+:all K-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 48)
    reconstructDecay('B0:49  -> D*-:all pi+:all K+:all K-:all pi0:all pi0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 49)
    reconstructDecay('B0:50  -> D*-:all K+:all K-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 50)
    reconstructDecay('B0:51  -> D*-:all pi+:all pi-:all K+:all K-:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 51)
    reconstructDecay('B0:52  -> D*-:all pi+:all K_S0:all K_S0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 52)
    reconstructDecay('B0:53  -> D*-:all pi+:all pi0:all pi0:all K_S0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 53)
    reconstructDecay('B0:54  -> D*-:all pi+:all K_S0:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 54)
    reconstructDecay('B0:55  -> D*-:all pi+:all pi-:all K_S0:all K_S0:all pi+:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 55)
    mmin = 41
    mmax = 55

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B0:56  -> D*-:all pi+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 56)
        mmax = 56

    # merge individual B0 lists into one list
    copyListFun(mmin, mmax, 'B0:D*-Y+S0')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B0:D*-Y+S0')
    # taking only correctly reconstructed B0
    applyCuts('B0:D*-Y+S0', 'isSignal>0.5')


def reconstructB0DSTMYPS1(q):  # reconstruct B0 -> D*- Y+(S=1) decays
    # decay modes in Full Event Interpretation Report (2014)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B0:57 -> D*-:all pi+:all pi-:all K+:all pi0:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 57)
    reconstructDecay('B0:58 -> D*-:all pi+:all pi+:all pi-:all pi0:all K_S0:all'
                     , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 58)
    reconstructDecay('B0:59 -> D*-:all pi+:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 59)
    reconstructDecay('B0:60 -> D*-:all pi+:all pi-:all K+:all ',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 60)
    reconstructDecay('B0:61 -> D*-:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 61)
    reconstructDecay('B0:62 -> D*-:all pi+:all pi0:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 62)
    mmin = 57
    mmax = 62

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B0:63 -> D*-:all K+:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 63)
        reconstructDecay('B0:64 -> D*-:all pi+:all pi0:all pi0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 64)
        reconstructDecay('B0:65 -> D*-:all pi+:all K+:all K-:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 65)
        reconstructDecay('B0:66 -> D*-:all pi+:all pi+:all pi-:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 66)
        reconstructDecay('B0:67 -> D*-:all pi+:all pi+:all pi-:all pi-:all K+:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 67)
        reconstructDecay('B0:68 -> D*-:all pi+:all pi-:all K+:all K+:all K-:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 68)
        reconstructDecay('B0:69 -> D*-:all pi+:all K+:all K-:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 69)
        reconstructDecay('B0:70 -> D*-:all pi-:all K+:all K+:all pi0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 70)
        reconstructDecay('B0:71 -> D*-:all K+:all K+:all K-:all pi0:all ',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 71)
        reconstructDecay('B0:72 -> D*-:all pi+:all pi-:all K+:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 72)
        reconstructDecay('B0:73 -> D*-:all K+:all K+:all K-:all pi0:all pi0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 73)
        reconstructDecay('B0:74 -> D*-:all K+:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 74)
        reconstructDecay('B0:75 -> D*-:all K+:all K+:all K-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 75)
        reconstructDecay('B0:76 -> D*-:all pi-:all K+:all K+:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 76)
        reconstructDecay('B0:77 -> D*-:all K+:all pi0:all K_S0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 77)
        reconstructDecay('B0:78 -> D*-:all K+:all pi0:all pi0:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 78)
        reconstructDecay('B0:79 -> D*-:all K+:all K_S0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 79)
        reconstructDecay('B0:80 -> D*-:all K+:all K+:all K-:all K_S0:all K_S0:all'
                         , 'Mbc>5.24 and -0.1 < deltaE < 0.1', 80)
        mmax = 80

    # merge individual B0 lists into one list
    copyListFun(mmin, mmax, 'B0:D*-Y+S1')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B0:D*-Y+S1')
    # taking only correctly reconstructed B0
    applyCuts('B0:D*-Y+S1', 'isSignal>0.5')


def reconstructB0ccbarY0S1(q):  # reconstruct B0 -> ccbar Y0(S=1) decays
    # experimentally clean decay modes
    reconstructDecay('B0:81 -> J/psi:all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 81)
    reconstructDecay('B0:82 -> J/psi:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 82)
    reconstructDecay('B0:83 -> J/psi:all K+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 83)
    reconstructDecay('B0:84 -> J/psi:all K_S0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 84)
    reconstructDecay('B0:85 -> J/psi:all K_S0:all pi-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 85)
    reconstructDecay('B0:86 -> J/psi:all K_S0:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 86)
    reconstructDecay('B0:87 -> J/psi:all K+:all pi0:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 87)

    reconstructDecay('B0:88 -> psi(2S):all K_S0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 88)
    reconstructDecay('B0:89 -> psi(2S):all K+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 89)
    reconstructDecay('B0:90 -> psi(2S):all K_S0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 90)
    reconstructDecay('B0:91 -> psi(2S):all K_S0:all pi-:all pi+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 91)
    reconstructDecay('B0:92 -> psi(2S):all K_S0:all pi0:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 92)
    reconstructDecay('B0:93 -> psi(2S):all K+:all pi-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 93)
    mmin = 81
    mmax = 93

    if q == 1:
        # rest of decay modes (less frequent)
        reconstructDecay('B0:94 -> chi_c0:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 94)
        reconstructDecay('B0:95 -> chi_c0:all K+:all pi-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 95)
        reconstructDecay('B0:96 -> chi_c0:all K_S0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 96)
        reconstructDecay('B0:97 -> chi_c0:all K_S0:all pi-:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 97)
        reconstructDecay('B0:98 -> chi_c0:all K_S0:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 98)
        reconstructDecay('B0:99 -> chi_c0:all K+:all pi-:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 99)

        reconstructDecay('B0:100 -> chi_c1:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 100)
        reconstructDecay('B0:101 -> chi_c1:all K+:all pi-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 101)
        reconstructDecay('B0:102 -> chi_c1:all K_S0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 102)
        reconstructDecay('B0:103 -> chi_c1:all K_S0:all pi-:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 103)
        reconstructDecay('B0:104 -> chi_c1:all K_S0:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 104)
        reconstructDecay('B0:105 -> chi_c1:all K+:all pi-:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 105)

        reconstructDecay('B0:106 -> chi_c2:all K_S0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 106)
        reconstructDecay('B0:107 -> chi_c2:all K+:all pi-:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 107)
        reconstructDecay('B0:108 -> chi_c2:all K_S0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 108)
        reconstructDecay('B0:109 -> chi_c2:all K_S0:all pi-:all pi+:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 109)
        reconstructDecay('B0:110 -> chi_c2:all K_S0:all pi0:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 110)
        reconstructDecay('B0:111 -> chi_c2:all K+:all pi-:all pi0:all',
                         'Mbc>5.24 and -0.1 < deltaE < 0.1', 111)
        mmax = 111

    # merge individual B0 lists into one list
    copyListFun(mmin, mmax, 'B0:ccbarY0S1')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B0:ccbarY0S1')
    # taking only correctly reconstructed B0
    applyCuts('B0:ccbarY0S1', 'isSignal>0.5')


def reconstructB0misc(q):  # miscellaneous modes
    # decay modes in Full Event Interpretation Report (2014)
    reconstructDecay('B0:112 -> D*-:all D*0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 112)
    reconstructDecay('B0:113 -> D*-:all D_s*+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 113)
    reconstructDecay('B0:114 -> anti-D0:all pi+:all pi-:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 114)
    # prevent double counting of different channels with same output particles
    applyCuts('B0:114',
              'abs(daughter(0,genMotherPDG)) < 413 or abs(daughter(0,genMotherPDG)) > 413'
              )
    reconstructDecay('B0:115 -> D-:all D*0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 115)
    reconstructDecay('B0:116 -> D*-:all D_s+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 116)
    reconstructDecay('B0:117 -> D-:all D0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 117)
    reconstructDecay('B0:118 -> D-:all D_s*+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 118)
    reconstructDecay('B0:119 -> D-:all D_s+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 119)

    # decay modes in the top ~98 % most frequent of all included
    reconstructDecay('B0:120 -> anti-D0:all pi+:all pi-:all pi0:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 120)
    # prevent double counting of different channels with same output particles
    applyCuts('B0:120',
              'abs(daughter(0,genMotherPDG)) < 413 or abs(daughter(0,genMotherPDG)) > 413'
              )
    reconstructDecay('B0:121 -> D*-:all D0:all K+:all',
                     'Mbc>5.24 and -0.1 < deltaE < 0.1', 121)
    mmin = 112
    mmax = 121

    # merge individual B0 lists into one list
    copyListFun(mmin, mmax, 'B0:misc')
    # perform MC matching (MC truth asociation)
    matchMCTruth('B0:misc')
    # taking only correctly reconstructed B0
    applyCuts('B0:misc', 'isSignal>0.5')


