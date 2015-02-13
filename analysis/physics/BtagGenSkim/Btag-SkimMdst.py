#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# Reconstruction of several possible decay modes of B mesons.
# Charged B mesons are reconstructed in imp_BC_decayRec.py
# Neutral B mesons are reconstructed in imp_BN_decayRec.py
# D mesons are reconstructed in imp_D_decayRec.py
# ccbar events are reconstructed in imp_cc_decayRec.py
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (January 2014)
#               A. Polsak (January 2014)
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
from imp_BN_decayRec import *
import sys

# input parameters: e.g. basf2 Btag-SkimMdst.py input.root output.root 1
# "0": include only most common decay modes
# "1": include all written decay modes

if len(sys.argv) != 4:
    sys.exit('Must provide enough arguments: [input file name] [output file name] [0/1]'
             )

inputName = sys.argv[1]
outputName = sys.argv[2]
recLong = sys.argv[3]

# load files
inputMdstList(inputName)

# create lists of FSPs
pions = ('pi+:all', '')
kaons = ('K+:all', '')
gammas = ('gamma:all', '')
electrons = ('e-:all', '')
muons = ('mu+:all', '')

fillParticleListsFromMC([pions, kaons, gammas, electrons, muons], True)

# reconstruct pi0 and K_S0
reconstructDecay('pi0:all -> gamma:all gamma:all', '0.10 < M < 0.15')
reconstructDecay('K_S0:all -> pi+:all pi-:all', '0.45 < M < 0.55')

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

# reconstruct B mesons
q = int(recLong)
if q != 0 and q != 1:
    sys.exit('Must provide argument 0 or 1')

reconstructBPD0YPS0(q)
reconstructBPD0YPS1(q)
reconstructBPDST0YPS0(q)
reconstructBPDST0YPS1(q)
reconstructBPccbarYPS1(q)
reconstructBPmisc(q)

reconstructB0DMYPS0(q)
reconstructB0DMYPS1(q)
reconstructB0DSTMYPS0(q)
reconstructB0DSTMYPS1(q)
reconstructB0ccbarY0S1(q)
reconstructB0misc(q)

# merge individual B+ lists
copyLists('B+:antiD0Y+', ['B+:antiD0Y+S0', 'B+:antiD0Y+S1'])
copyLists('B+:antiD*0Y+', ['B+:antiD*0Y+S0', 'B+:antiD*0Y+S1'])
copyLists('B+:rest', ['B+:ccbarYPS1', 'B+:misc'])

copyLists('B+:all', ['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

# merge individual B0 lists
copyLists('B0:D-Y+', ['B0:D-Y+S0', 'B0:D-Y+S1'])
copyLists('B0:D*-Y+', ['B0:D*-Y+S0', 'B0:D*-Y+S1'])
copyLists('B0:rest', ['B0:ccbarY0S1', 'B0:misc'])

copyLists('B0:all', ['B0:D-Y+', 'B0:D*-Y+', 'B0:rest'])

# add SkimFilter module to set condition variable based on the number of reconstructed B-tag mesons
skim = register_module('SkimFilter')
skim.param('particleLists', ['B+:all', 'B0:all'])
analysis_main.add_module(skim)

# save event to a new mdst file if it contained at least one B-tag meson
empty_path = create_path()
skim.if_false(empty_path)
add_mdst_output(analysis_main, True, outputName)

# print out some further info
summaryOfLists(['B+:all'])
summaryOfLists(['B0:all'])

# process the events
process(analysis_main)

# print out the summary
print statistics
