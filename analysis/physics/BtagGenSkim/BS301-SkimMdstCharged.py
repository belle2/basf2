#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# Reconstruction of several possible decay modes of charged B mesons.
# Charged B mesons are reconstructed in imp_BC_decayRec.py
# D mesons are reconstructed in imp_D_decayRec.py
# ccbar events are reconstructed in imp_cc_decayRec.py
# decay mode names are written in imp_chNames.py
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (December 2014)
#               A. Polsak (December 2014)
######################################################

# load all necessary scripts
from basf2 import *
from reconstruction import *

from modularAnalysis import inputMdstList
from modularAnalysis import newFillParticleListFromMC
from modularAnalysis import applyCuts
from modularAnalysis import summaryOfLists
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList
from modularAnalysis import summaryOfLists

from imp_D_decayRec import *
from imp_cc_decayRec import *
from imp_BC_decayRec import *
import sys

# input parameters: e.g. basf2 BS301-SkimMdstCharged.py 1
# "0": include only most common decay modes
# "1": include all written decay modes

if len(sys.argv) != 2:
    sys.exit('Must provide enough arguments')

recLong = sys.argv[1]

inputFiles = ['BS101-BBChargedGen.root']

# load files
inputMdstList(inputFiles)

# create lists of FSPs
newFillParticleListFromMC('pi+:all', '')
newFillParticleListFromMC('K+:all', '')
newFillParticleListFromMC('gamma:all', '')
newFillParticleListFromMC('e-:all', '')
newFillParticleListFromMC('mu+:all', '')

# reconstruct, check and select correct pi0
reconstructDecay('pi0:all -> gamma:all gamma:all', '0.12 < M < 0.15', 1)
matchMCTruth('pi0:all')
applyCuts('pi0:all', 'isSignal > 0.5')

# reconstruct, check and select correct K_S0
reconstructDecay('K_S0:pi0 -> pi0:all pi0:all', '0.45 < M < 0.55', 1)
reconstructDecay('K_S0:pi -> pi+:all pi-:all', '0.45 < M < 0.55', 2)
copyLists('K_S0:all', ['K_S0:pi0', 'K_S0:pi'])
matchMCTruth('K_S0:all')
applyCuts('K_S0:all', 'isSignal > 0.5')

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

# reconstruct B+B- mesons
q = int(recLong)
if q != 0 and q != 1:
    sys.exit('Must provide argument 0 or 1')

reconstructBPD0YPS0(q)
reconstructBPD0YPS1(q)
reconstructBPDST0YPS0(q)
reconstructBPDST0YPS1(q)
reconstructBPccbarYPS1(q)
reconstructBPmisc(q)

# merge individual B+ lists
# the same is already done for D mesons and ccbar, but in their own files, B mesons are here separately for ease of access
copyLists('B+:antiD0Y+', ['B+:antiD0Y+S0', 'B+:antiD0Y+S1'])
applyCuts('B+:antiD0Y+',
          'abs(daughter(0,genMotherPDG)) < 423 or abs(daughter(0,genMotherPDG)) > 423'
          )
             # this prevents double counting of different channels with same particle output

copyLists('B+:antiD*0Y+', ['B+:antiD*0Y+S0', 'B+:antiD*0Y+S1'])
copyLists('B+:rest', ['B+:ccbarYPS1', 'B+:misc'])

copyLists('B+:all', ['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

# add SkimFilter module to set condition variable based on the number of reconstructed B-tag mesons
skim = register_module('SkimFilter')
skim.param('particleLists', ['B+:all'])
analysis_main.add_module(skim)

# save event to a new mdst file if it contained at least one B-tag meson
new_analysis_main = create_path()
add_mdst_output(new_analysis_main, True, 'BS301-SkimMdstCharged.root')
skim.if_true(new_analysis_main, AfterConditionPath.CONTINUE)

# print out some further info
summaryOfLists(['B+:all'])
summaryOfLists(['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

# process the events
process(analysis_main)

# print out the summary
print statistics
