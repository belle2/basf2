#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# Reconstruction of several possible decay modes of D mesons from BB events.
# D mesons are reconstructed in D_decayRec.py
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej  (January 2015)
#               A. Polsak (January 2015)
#
######################################################

# load all necessary scripts
from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import applyCuts
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList
from modularAnalysis import fillParticleListsFromMC

from imp_D_decayRec import *

import sys
import os

if len(sys.argv) != 3:
    sys.exit('Must provide enough arguments: [input file] [output file]')

# input parameters: e.g. basf2 EvtGenValidation-D.py input.root output.root
# this loads selected input file(s) and created an output file with reconstructed particles.
inputName = sys.argv[1]
outputName = sys.argv[2]

inputFiles = [inputName]

# load files
inputMdstList(inputFiles)

# load all final state MCParticles
pions = ('pi+:all', '')
kaons = ('K+:all', '')
gammas = ('gamma:all', '')
electrons = ('e-:all', '')
muons = ('mu+:all', '')
pi0 = ('pi0:all', '')
kshort = ('K_S0:all', '')

d0 = ('D0:gen', '')
dp = ('D+:gen', '')
dsp = ('D_s+:gen', '')
dst0 = ('D*0:gen', '')
dstp = ('D*+:gen', '')
dsstp = ('D_s*+:gen', '')

fillParticleListsFromMC([
    pions,
    kaons,
    gammas,
    electrons,
    muons,
    pi0,
    kshort,
    d0,
    dp,
    dsp,
    dst0,
    dstp,
    dsstp,
    ], True)

# reconstruct charm
reconstructD0()
reconstructDP()
reconstructDSP()
reconstructDST0()
reconstructDSTP()
reconstructDSSTP()

# create and fill flat Ntuple with other information
toolsD0 = ['EventMetaData', '^D0']
toolsD0 += ['CustomFloats[extraInfo(decayModeID)]', '^D0']
toolsDP = ['EventMetaData', '^D+']
toolsDP += ['CustomFloats[extraInfo(decayModeID)]', '^D+']
toolsDSP = ['EventMetaData', '^D_s+']
toolsDSP += ['CustomFloats[extraInfo(decayModeID)]', '^D_s+']

toolsDST0 = ['EventMetaData', '^D*0']
toolsDST0 += ['CustomFloats[extraInfo(decayModeID)]', '^D*0']
toolsDSTP = ['EventMetaData', '^D*+']
toolsDSTP += ['CustomFloats[extraInfo(decayModeID)]', '^D*+']
toolsDSSTP = ['EventMetaData', '^D_s*+']
toolsDSSTP += ['CustomFloats[extraInfo(decayModeID)]', '^D_s*+']

toolsD0gen = ['EventMetaData', '^D0']
toolsDPgen = ['EventMetaData', '^D+']
toolsDSPgen = ['EventMetaData', '^D_s+']
toolsDST0gen = ['EventMetaData', '^D*0']
toolsDSTPgen = ['EventMetaData', '^D*+']
toolsDSSTPgen = ['EventMetaData', '^D_s*+']

# nTuple OUTPUT
ntupleFile(outputName)
ntupleTree('d0', 'D0:all', toolsD0)
ntupleTree('dp', 'D+:all', toolsDP)
ntupleTree('dsp', 'D_s+:all', toolsDSP)
ntupleTree('dst0', 'D*0:all', toolsDST0)
ntupleTree('dstp', 'D*+:all', toolsDSTP)
ntupleTree('dsstp', 'D_s*+:all', toolsDSSTP)

ntupleTree('d0gen', 'D0:gen', toolsD0gen)
ntupleTree('dpgen', 'D+:gen', toolsDPgen)
ntupleTree('dspgen', 'D_s+:gen', toolsDSPgen)
ntupleTree('dst0gen', 'D*0:gen', toolsDST0gen)
ntupleTree('dstpgen', 'D*+:gen', toolsDSTPgen)
ntupleTree('dsstpgen', 'D_s*+:gen', toolsDSSTPgen)

# Process the events
process(analysis_main)

# print out the summary
print statistics
