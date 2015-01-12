#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# Reconstruction of several possible decay modes of neutral B mesons.
# Neutral B mesons are reconstructed in imp_BN_decayRec.py
# D mesons are reconstructed in imp_D_decayRec.py
# ccbar events are reconstructed in imp_cc_decayRec.py
# decay mode names are written in chNames.py
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (December 2014)
#               A. Polsak (December 2014)
######################################################

# load all necessary scripts
from basf2 import *
from reconstruction import *

from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleListsFromMC
from modularAnalysis import applyCuts
from modularAnalysis import summaryOfLists
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList

from imp_D_decayRec import *
from imp_cc_decayRec import *
from imp_BN_decayRec import *
import sys

# input parameters: e.g. basf2 BS302-SkimMdstNeutral.py 0
# "0": include only most common decay modes
# "1": include all written decay modes

if len(sys.argv) != 2:
    sys.exit('Must provide argument enough')

recLong = sys.argv[1]

inputFiles = ['BS102-BBNeutralGen.root']

# load files
inputMdstList(inputFiles)

# create lists of FSPs
pions = ('pi+:all', '')
kaons = ('K+:all', '')
gammas = ('gamma:all', '')
electrons = ('e-:all', '')
muons = ('mu+:all', '')
pi0 = ('pi0:all', '')
kshort = ('K_S0', '')

fillParticleListsFromMC([
    pions,
    kaons,
    gammas,
    electrons,
    muons,
    pi0,
    kshort,
    ])

# next lines do the same as the lines for pi0 and K_S0
# reconstruct charm
reconstructD0()
reconstructDP()
reconstructDSP()
reconstructDST0()
reconstructDSTP()
reconstructDSSTP()

# reconstruct ccbar
reconstructJpsi()
reconstructPsi2S()
reconstructChi_cy()

# reconstruct anti-B0B0 mesons
q = int(recLong)
if q != 0 and q != 1:
    sys.exit('Must provice argument 0/1')

reconstructB0DMYPS0(q)
reconstructB0DMYPS1(q)
reconstructB0DSTMYPS0(q)
reconstructB0DSTMYPS1(q)
reconstructB0ccbarY0S1(q)
reconstructB0misc(q)

# merge individual B0 lists
# the same is already done for D mesons and ccbar, but in their own files, B mesons are here separately for ease of access
copyLists('B0:D-Y+', ['B0:D-Y+S0', 'B0:D-Y+S1'])
copyLists('B0:D*-Y+', ['B0:D*-Y+S0', 'B0:D*-Y+S1'])
copyLists('B0:rest', ['B0:ccbarY0S1', 'B0:misc'])

copyLists('B0:all', ['B0:D-Y+', 'B0:D*-Y+', 'B0:rest'])

# add SkimFilter module to set condition variable based on the number of reconstructed B-tag mesons
skim = register_module('SkimFilter')
skim.param('particleLists', ['B0:all'])
analysis_main.add_module(skim)

# save event to a new mdst file if it contained at least one B-tag meson
new_analysis_main = create_path()
add_mdst_output(new_analysis_main, True, 'BS302-SkimMdstNeutral.root')
skim.if_true(new_analysis_main, AfterConditionPath.CONTINUE)

# print out some further info
summaryOfLists(['B0:all'])
summaryOfLists(['B0:D-Y+', 'B0:D*-Y+', 'B0:rest'])

# process the events
process(analysis_main)

# print out the summary
print statistics
