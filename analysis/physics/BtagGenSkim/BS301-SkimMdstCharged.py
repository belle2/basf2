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
from imp_BC_decayRec import *
import sys

# input parameters: e.g. basf2 BS301-SkimMdstCharged.py input.root output.root 1
# "0": include only most common decay modes
# "1": include all written decay modes

if len(sys.argv) != 4:
    sys.exit('Must provide enough arguments: [input file name(s)] [output file name] [0/1]'
             )

inputNames = sys.argv[1]
outputName = sys.argv[2]
recLong = sys.argv[3]

inputFiles = [inputNames]

# load files
inputMdstList(inputFiles)

# create lists of FSPs
pions = ('pi+:all', '')
kaons = ('K+:all', '')
gammas = ('gamma:all', '')
electrons = ('e-:all', '')
muons = ('mu+:all', '')
pi0 = ('pi0:all', '')
kshort = ('K_S0:all', '')

fillParticleListsFromMC([
    pions,
    kaons,
    gammas,
    electrons,
    muons,
    pi0,
    kshort,
    ], True)

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
copyLists('B+:antiD*0Y+', ['B+:antiD*0Y+S0', 'B+:antiD*0Y+S1'])
copyLists('B+:rest', ['B+:ccbarYPS1', 'B+:misc'])

copyLists('B+:all', ['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

# add SkimFilter module to set condition variable based on the number of reconstructed B-tag mesons
skim = register_module('SkimFilter')
skim.param('particleLists', ['B+:all'])
analysis_main.add_module(skim)

# save event to a new mdst file if it contained at least one B-tag meson
new_analysis_main = create_path()
add_mdst_output(new_analysis_main, True, outputName)
skim.if_true(new_analysis_main, AfterConditionPath.CONTINUE)

# print out some further info
summaryOfLists(['B+:all'])
summaryOfLists(['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

# process the events
process(analysis_main)

# print out the summary
print statistics
