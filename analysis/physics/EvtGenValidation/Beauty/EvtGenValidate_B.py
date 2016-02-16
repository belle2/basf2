#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Reconstruction of several possible decay modes of B mesons
# for the purpose of MC Validation.
#
# Charged B mesons are reconstructed in imp_BC_decayRec.py
# Neutral B mesons are reconstructed in imp_BN_decayRec.py
# Channel names are in imp_chNames.py
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (January 2015)
#               A. Polsak (January 2015)
#
######################################################

# load all necessary scripts
from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleListsFromMC
from modularAnalysis import applyCuts
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList

from imp_BC_decayRec import *
from imp_BN_decayRec import *

import sys
import os

if len(sys.argv) != 5:
    sys.exit('Must provide enough arguments: [input file] [output file] [0/1] [type = B0/BP]')

# input parameters: e.g. basf2 VA101-BBValidate.py input.root output.root 0/1
# this loads selected input file(s) and created an output file with
# reconstructed particles. For only common decays use argument 0,
# otherwise use 1.
inputName = sys.argv[1]
outputName = sys.argv[2]
recLong = sys.argv[3]
type = sys.argv[4]

if type != 'B0' and type != 'BP':
    sys.exit('Invalid type = ' + type + '! Valid types are B0 or BP')

inputFiles = [inputName]

# load files
inputMdstList(inputFiles)

# create FSP maps
pions = ('pi+:all', 'mcPrimary == 1')
kaons = ('K+:all', 'mcPrimary == 1')
# gammas    = ('gamma:all', 'mcPrimary == 1')
# electrons = ('e-:all', 'mcPrimary == 1')
# muons     = ('mu+:all', 'mcPrimary == 1')
pi0 = ('pi0:all', 'mcPrimary == 1')
kshort = ('K_S0:all', 'mcPrimary == 1')

# charm map
d0 = ('D0:all', '')
dp = ('D+:all', '')
dsp = ('D_s+:all', '')
dst0 = ('D*0:all', '')
dstp = ('D*+:all', '')
dsstp = ('D_s*+:all', '')

# ccbar map
jpsi = ('J/psi:all', '')
psi2s = ('psi(2S):all', '')
chic0 = ('chi_c0:all', '')
chic1 = ('chi_c1:all', '')
chic2 = ('chi_c2:all', '')

# b map
bpgen = ('B+:gen', '')
b0gen = ('B0:gen', '')

# fill maps
fillParticleListsFromMC([
    pions,
    kaons,
    #    gammas,
    #    electrons,
    #    muons,
    pi0,
    kshort,
    d0,
    dp,
    dsp,
    dst0,
    dstp,
    dsstp,
    jpsi,
    psi2s,
    chic0,
    chic1,
    chic2,
], True)
fillParticleListsFromMC([bpgen, b0gen])

# reconstruct B mesons: argument 0 for short, argument 1 for reconstruction of all included decays
q = int(recLong)
if q != 0 and q != 1:
    sys.exit('Must provide argument (0/1)')

if type == 'BP':
    reconstructBPD0YPS0(q)
    reconstructBPD0YPS1(q)
    reconstructBPDST0YPS0(q)
    reconstructBPDST0YPS1(q)
    reconstructBPccbarYPS1(q)
    reconstructBPmisc(q)

    # merge individual B+ lists
    copyLists('B+:antiD0Y+', ['B+:antiD0Y+S0', 'B+:antiD0Y+S1'])
    copyLists('B+:antiD*0Y+', ['B+:antiD*0Y+S0', 'B+:antiD*0Y+S1'])
    copyLists('B+:rest', ['B+:ccbarYPS1', 'B+:misc'])

    copyLists('B+:all', ['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

if type == 'B0':
    reconstructB0DMYPS0(q)
    reconstructB0DMYPS1(q)
    reconstructB0DSTMYPS0(q)
    reconstructB0DSTMYPS1(q)
    reconstructB0ccbarY0S1(q)
    reconstructB0misc(q)

    # merge individual B0 lists
    copyLists('B0:D-Y+', ['B0:D-Y+S0', 'B0:D-Y+S1'])
    copyLists('B0:D*-Y+', ['B0:D*-Y+S0', 'B0:D*-Y+S1'])
    copyLists('B0:rest', ['B0:ccbarY0S1', 'B0:misc'])

    copyLists('B0:all', ['B0:D-Y+', 'B0:D*-Y+', 'B0:rest'])

# create and fill flat Ntuple with other information
toolsBP = ['EventMetaData', '^B+']
toolsBP += ['CustomFloats[extraInfo(decayModeID)]', '^B+']
toolsBPgen = ['EventMetaData', '^B+']

toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['CustomFloats[extraInfo(decayModeID)]', '^B0']
toolsB0gen = ['EventMetaData', '^B0']

# nTuple OUTPUT
ntupleFile(outputName)
if type == 'BP':
    ntupleTree('bp', 'B+:all', toolsBP)
    ntupleTree('bpgen', 'B+:gen', toolsBPgen)

if type == 'B0':
    ntupleTree('b0', 'B0:all', toolsB0)
    ntupleTree('b0gen', 'B0:gen', toolsB0gen)

# process the events
process(analysis_main)

# print out the summary
print(statistics)
