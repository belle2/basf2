#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__authors__ = [
    "Chiara La Licata"
]

from basf2 import *
from modularAnalysis import *
import sys
import glob
import os.path
import basf2

'''
**Skim Author**:
Chiara La Licata

**Skim list definitions for neutral B decays:**
B0 -> D- pi+ with D- -> k+ pi- pi-
B0 -> D*- pi+ with D* -> anti-D0 pi- and anti-D0->k+ pi-; k+ pi- pi0; k+ pi- pi- pi+

**List of cuts applied**:
B0 cuts:
Mbc > 5.2
abs(deltaE) < 0.3

D- cut:
1.8 < M < 1.9

D0 cut:
1.7 < M < 2.0

D* cut:
0.0 < massDifference(0) < 0.16

'''

# B0 to D- pi+


def loadB0toDpi(path):
    '''
    B0 -> D- pi+ reconstructed
    with D- -> k+ pi- pi-

    B0 cuts and D- cut applied

    '''

    print('loadB0toDpi')

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    BsigChannel = 'D-:Kpipi pi+:all'

    BsigList = []

    reconstructDecay('B0:Dpi -> ' + BsigChannel, Bcuts, 0, path=path)

    BsigList.append('B0:Dpi')

    copyLists('B0:SigDpi', BsigList, path=path)

    Lists = BsigList
    return Lists


# B0 to D*- pi+  with all D decay modes

def loadB0toDstarh(path):
    '''
    B0 -> D*- pi+ reconstructed with with D*- -> anti-D0
    and anti-D0->k+ pi-; k+ pi- pi0; k+ pi- pi- pi+

    B0 cuts, D* and D0 cuts applied

    '''

    print('loadB0toDstarh')

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    BsigList = []

    reconstructDecay('B0:Dstarpi -> D*-:DstarMinus pi+:all', Bcuts, 0, path=path)
    BsigList.append('B0:Dstarpi')

    copyLists('B0:SigDstarh', BsigList, path=path)

    Lists = BsigList
    return Lists


# B0 to D*- pi+  with all D to k pi

def loadB0toDstarh_kpi(path):
    '''
    B0 -> D*- pi+ reconstructed with with D*- -> anti-D0
    anti-D0->k+ pi- only

    B0 cuts, D* and D0 cuts applied

    '''

    print('loadB0toDstarh_kpi')

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    BsigList_kpi = []

    reconstructDecay('B0:Dstarpi_kpi -> D*-:DstarMinus_kpi pi+:all', Bcuts, 0, path=path)
    BsigList_kpi.append('B0:Dstarpi_kpi')

    copyLists('B0:SigDstarh_kpi', BsigList_kpi, path=path)

    Lists = BsigList_kpi
    return Lists


# B0 to D*- pi+  with all D to k pi pi

def loadB0toDstarh_kpipi(path):
    '''
    B0 -> D*- pi+ reconstructed with with D*- -> anti-D0
    anti-D0->k+ pi- pi0 only

    B0 cuts, D* and D0 cuts applied

    '''

    print('loadB0toDstarh_kpipi')

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    BsigList_kpipi = []

    reconstructDecay('B0:Dstarpi_kpipi -> D*-:DstarMinus_kpipi pi+:all', Bcuts, 0, path=path)
    BsigList_kpipi.append('B0:Dstarpi_kpipi')

    copyLists('B0:SigDstarh_kpipi', BsigList_kpipi, path=path)

    Lists = BsigList_kpipi
    return Lists


# B0 to D*- pi+  with all D to k pi pi pi

def loadB0toDstarh_kpipipi(path):
    '''
    B0 -> D*- pi+ reconstructed with with D*- -> anti-D0
    anti-D0->k+ pi- pi- pi+ only

    B0 cuts, D* and D0 cuts applied

    '''

    print('loadB0toDstarh_kpipipi')

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    BsigList_kpipipi = []

    reconstructDecay('B0:Dstarpi_kpipipi -> D*-:DstarMinus_kpipipi pi+:all', Bcuts, 0, path=path)
    BsigList_kpipipi.append('B0:Dstarpi_kpipipi')

    copyLists('B0:SigDstarh_kpipipi', BsigList_kpipipi, path=path)

    Lists = BsigList_kpipipi
    return Lists


# D- to k+ pi- pi-


def loadD(path):
    '''
    D- -> k+ pi- pi- reconstructed

    D- cut applied
    '''

    print('loadD')

    Dcuts = '1.8 < M < 1.9'

    DChannel = 'K+:all pi-:all pi-:all'

    DList = []

    reconstructDecay('D-:Kpipi -> ' + DChannel, Dcuts, 0, path=path)
    DList.append('D-:Kpipi')

    return DList

# D0 to all interesting D decay modes


def loadD0(path):
    '''
    D0 -> k- pi+; k- pi+ pi0; k- pi+ pi+ pi- reconstructed
    D0 cut applied

    '''
    print('loadD0')

    D0cuts = '1.7 < M < 2.0'

    D0Channels = [
        'K-:all pi+:all',
        'K-:all pi+:all pi0:skim',
        'K-:all pi+:all pi-:all pi+:all'
    ]

    D0List = []

    for chID, channel in enumerate(D0Channels):
        reconstructDecay('D0:D0' + str(chID+1) + ' -> ' + channel, D0cuts, chID, path=path)
        D0List.append('D0:D0' + str(chID+1))
        copyLists('D0:hh', D0List, path=path)

    return D0List

# D0 to k Pi Pi Pi


def loadD0_kpipipi(path):
    '''
    D0 -> k- pi+ pi+ pi- reconstructed
    D0 cut applied

    '''

    print('loadD0_kpipipi')

    D0cuts = '1.7 < M < 2.0'

    D0Channels = [
        'K-:all pi+:all pi-:all pi+:all'
    ]

    D0List_kpipipi = []

    for chID, channel in enumerate(D0Channels):
        reconstructDecay('D0:D0_kpipipi' + str(chID+1) + ' -> ' + channel, D0cuts, chID, path=path)
        D0List_kpipipi.append('D0:D0_kpipipi' + str(chID+1))
        copyLists('D0:D0_k-pi+pi+pi-', D0List_kpipipi, path=path)

    return D0List_kpipipi


# D0 to k pi pi

def loadD0_kpipi(path):
    '''
    D0 -> k- pi+ pi0 reconstructed
    D0 cut applied

    '''

    print('loadD0_kpipi')

    D0cuts = '1.7 < M < 2.0'

    D0Channels = [
        'K-:all pi+:all pi0:skim'
    ]

    D0List_kpipi = []

    for chID, channel in enumerate(D0Channels):
        reconstructDecay('D0:D0_kpipi' + str(chID+1) + ' -> ' + channel, D0cuts, chID, path=path)
        D0List_kpipi.append('D0:D0_kpipi' + str(chID+1))
        copyLists('D0:D0_k-pi+pi0', D0List_kpipi, path=path)

    return D0List_kpipi


# D0 to k pi

def loadD0_kpi(path):
    '''
    D0 -> k- pi+ reconstructed
    D0 cut applied

    '''

    print('loadD0_kpi')

    D0cuts = '1.7 < M < 2.0'

    D0Channels = [
        'K-:all pi+:all'
    ]

    D0List_kpi = []

    for chID, channel in enumerate(D0Channels):
        reconstructDecay('D0:D0_kpi' + str(chID+1) + ' -> ' + channel, D0cuts, chID, path=path)
        D0List_kpi.append('D0:D0_kpi' + str(chID+1))
        copyLists('D0:D0_k-pi+', D0List_kpi, path=path)

    return D0List_kpi


# D* all D decay modes

def loadDstar(path):
    '''
    D* -> anti-D0 pi- with anti-D0 -> k+ pi-; k+ pi- pi0; k+ pi- pi- pi+ reconstructed
    D* cut applied

    '''

    print('loadDstar')

    DstarCut = '0.0 < massDifference(0) < 0.16'

    DstarMinusChannel = 'anti-D0:hh pi-:all'

    DstarList = []

    reconstructDecay('D*-:DstarMinus -> ' + DstarMinusChannel, DstarCut, 0, path=path)

    DstarList.append('D*-:DstarMinus')

    List = DstarList

    return List


# D* kPi D decay mode

def loadDstar_kpi(path):
    '''
    D* -> anti-D0 pi- with anti-D0 -> k+ pi- reconstructed
    D* cut applied

    '''
    print('loadDstar_kpi')

    DstarCut = '0.0 < massDifference(0) < 0.16'

    DstarMinusChannel = 'anti-D0:D0_k-pi+ pi-:all'

    DstarList_kpi = []

    reconstructDecay('D*-:DstarMinus_kpi -> ' + DstarMinusChannel, DstarCut, 0, path=path)

    DstarList_kpi.append('D*-:DstarMinus_kpi')

    List = DstarList_kpi

    return List


# D* kPiPi D decay mode

def loadDstar_kpipi(path):
    '''
    D* -> anti-D0 pi- with anti-D0 -> k+ pi- pi0 reconstructed
    D* cut applied

    '''
    print('loadDstar_kpipi')

    DstarCut = '0.0 < massDifference(0) < 0.16'

    DstarMinusChannel = 'anti-D0:D0_k-pi+pi0 pi-:all'

    DstarList_kpipi = []

    reconstructDecay('D*-:DstarMinus_kpipi -> ' + DstarMinusChannel, DstarCut, 0, path=path)

    DstarList_kpipi.append('D*-:DstarMinus_kpipi')

    List = DstarList_kpipi

    return List


# D* kPiPiPi D decay mode

def loadDstar_kpipipi(path):
    '''
    D* -> anti-D0 pi- with anti-D0 -> k+ pi- pi- pi+ reconstructed
    D* cut applied

    '''
    print('loadDstar_kpipipi')

    DstarCut = '0.0 < massDifference(0) < 0.16'

    DstarMinusChannel = 'anti-D0:D0_k-pi+pi+pi- pi-:all'

    DstarList_kpipipi = []

    reconstructDecay('D*-:DstarMinus_kpipipi -> ' + DstarMinusChannel, DstarCut, 0, path=path)

    DstarList_kpipipi.append('D*-:DstarMinus_kpipipi')

    List = DstarList_kpipipi

    return List
