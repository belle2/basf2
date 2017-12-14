#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
<input>../1150720000.dst.root</input>
<output>../1150720000.ntup.root</output>
<contact>Matic Lubej; matic.lubej@ijs.si</contact>
</header>
"""

#######################################################
#
# Validation script for B0 --> pi l nu
#
#
# Contributors: M. Lubej (April 2017)
#
######################################################

import sys
import glob
import os

from basf2 import *
from modularAnalysis import *

from variables import variables

logging.log_level = LogLevel.ERROR

outputRootFile = "../1150720000.ntup.root"

mcFiles = ["../1150720000.dst.root"]

# input
inputMdstList('default', mcFiles)

# fsp selection
electrons = ('e-:rec', 'useCMSFrame(p) > 1.0 and electronID > 0.8')
muons = ('mu-:rec', 'useCMSFrame(p) > 1.0 and muonID > 0.8')
pions = ('pi-:rec', 'pionID > 0.8')

fillParticleLists([electrons, muons, pions])

# B meson reconstruction
reconstructDecay('B0:e_rec -> pi-:rec e+:rec', '', 1)
reconstructDecay('B0:mu_rec -> pi-:rec mu+:rec', '', 2)

# merge + MC matching + vertex
copyLists('B0:sl_rec', ['B0:e_rec', 'B0:mu_rec'])
matchMCTruth('B0:sl_rec')
vertexRave('B0:sl_rec', 0.001, '', 'ipprofile')

# select signal + build ROE
buildRestOfEvent('B0:sl_rec')

# define ROE mask cuts
tracksCut = 'abs(d0) < 10.0 and abs(z0) < 20.0'
clustersCut1 = '[clusterE1E9 > 0.4 or E > 0.075] and [[E > 0.062 and abs(clusterTiming)<18 and clusterReg==1] or '
clustersCut2 = '[E > 0.060 and abs(clusterTiming)<20 and clusterReg==2] or [E > 0.056 and abs(clusterTiming)<44 and clusterReg==3]]'
clustersCut = clustersCut1 + clustersCut2

# define and append ROE masks
m0 = ('m0', '', '', [0, 0, 1, 0, 0, 0])
m1 = ('m1', tracksCut, clustersCut, [0, 0, 1, 0, 0, 0])
appendROEMasks('B0:sl_rec', [m0, m1])

# helper function for ROE variables


def setVars(mask_i):
    variables.addAlias('ROECharge' + str(mask_i), 'ROE_charge(m' + str(mask_i) + ')')
    variables.addAlias('ROEDE' + str(mask_i), 'ROE_deltae(m' + str(mask_i) + ')')
    variables.addAlias('sigDE' + str(mask_i), 'correctedB_deltae(m' + str(mask_i) + ',0)')
    variables.addAlias('sigMbc' + str(mask_i), 'correctedB_mbc(m' + str(mask_i) + ',0)')
    variables.addAlias('sigDELab' + str(mask_i), 'correctedB_deltae(m' + str(mask_i) + ',1)')
    variables.addAlias('sigMbcLab' + str(mask_i), 'correctedB_mbc(m' + str(mask_i) + ',1)')
    variables.addAlias('missM2' + str(mask_i), 'missM2(m' + str(mask_i) + ',0)')
    variables.addAlias('missM2Lab' + str(mask_i), 'missM2(m' + str(mask_i) + ',6)')
    variables.addAlias('m2OverEmiss' + str(mask_i), 'missM2OverMissE(m' + str(mask_i) + ')')
    variables.addAlias('roeFlags' + str(mask_i), 'ROE_mcMissFlags(m' + str(mask_i) + ')')

# which variables to save
varArray = ['ROECharge', 'roeFlags', 'sigDE', 'sigMbc', 'm2OverEmiss']

# helper function for ROE variables


def appendVariables(maskIterator_i, array):
    setVars(maskIterator_i)
    command = 'recInfo += [\'CustomFloats['
    for var in array:
        command += var + str(maskIterator_i) + ':'
    command = command[:-1]
    command += ']\', \'^B0\']'
    exec(command)

# define ntuple tool
recInfo = ['EventMetaData', '^B0']
recInfo += ['CustomFloats[decayTypeRecoil]', '^B0']
recInfo += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B0']
recInfo += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B0']

# append ROE variables to ntuple tool
appendVariables(0, varArray)
appendVariables(1, varArray)

# save to file
ntupleFile(outputRootFile)
ntupleTree('rec', 'B0:sl_rec', recInfo)

# show progress
progress = register_module('Progress')
analysis_main.add_module(progress)

# process the events
process(analysis_main)

# print out the summary
print(statistics)
