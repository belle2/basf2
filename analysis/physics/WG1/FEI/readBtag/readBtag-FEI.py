#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI.
#
# This script is used as well for the production of
# of official monitoring and validation single tag samples.
#
# Contributors: A. Zupanc (November 2015)
#
######################################################
import sys
import glob
import os.path

from basf2 import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import rankByHighest
from modularAnalysis import copyLists
from modularAnalysis import buildRestOfEvent
from modularAnalysis import applyCuts
from modularAnalysis import looseMCTruth

# create aliases for the complex functions so that
# their names in the ntuple are human readable
from variables import variables
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('rank', 'extraInfo(sigProb_rank)')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
variables.addAlias('uniqueSignal', 'extraInfo(uniqueSignal)')
variables.addAlias('looseMCMotherPDG', 'extraInfo(looseMCMotherPDG)')
variables.addAlias('looseMCWrongDaughterN', 'extraInfo(looseMCWrongDaughterN)')
variables.addAlias('looseMCWrongDaughterPDG', 'extraInfo(looseMCWrongDaughterPDG)')
variables.addAlias('looseMCWrongDaughterBiB', 'extraInfo(looseMCWrongDaughterBiB)')
variables.addAlias('d0_dmID', 'daughter(0,extraInfo(decayModeID))')
variables.addAlias('d1_dmID', 'daughter(1,extraInfo(decayModeID))')
variables.addAlias('d0_d0_dmID', 'daughter(0,daughter(0,extraInfo(decayModeID)))')
variables.addAlias('d1_d0_dmID', 'daughter(1,daughter(0,extraInfo(decayModeID)))')
variables.addAlias('d1d2_M', 'daughterInvariantMass(1,2)')
variables.addAlias('d1d3_M', 'daughterInvariantMass(1,3)')
variables.addAlias('d1d4_M', 'daughterInvariantMass(1,4)')
variables.addAlias('d2d3_M', 'daughterInvariantMass(2,3)')
variables.addAlias('d2d4_M', 'daughterInvariantMass(2,4)')
variables.addAlias('d3d4_M', 'daughterInvariantMass(3,4)')
variables.addAlias('d1d2d3_M', 'daughterInvariantMass(1,2,3)')
variables.addAlias('d2d3d4_M', 'daughterInvariantMass(2,3,4)')
variables.addAlias('d1d2d3d4_M', 'daughterInvariantMass(1,2,3,4)')

# this script is intended to be executed in runOverMC.py
if len(sys.argv) != 5:
    sys.exit('Not enough arguments: [output file name] [MC files chunk number] [number of files per chunk] [location of MC files]'
             )

outputRootFile = sys.argv[1]
chunkN = int(sys.argv[2])
filesPerJob = int(sys.argv[3])
mcLocation = sys.argv[4]

from makeChunks import chunks
mcFiles = glob.glob(mcLocation + '/*.root')
fileChunks = chunks(mcFiles, filesPerJob)

# TODO: specify the location of database (faster)
db_location = '/home/belle/zupanc/belle2/physics/FEI/Belle2_Generic_2016_1/fei_database'
use_central_database('production', LogLevel.WARNING, db_location)

# TODO: load the FEI reconstruction path
# specify the location of the fei_pickle file
fei_pickle =
'/home/belle/zupanc/belle2/physics/FEI/Belle2_Generic_2016_1_looseMCMatching/paths/basf2_final_path_without_selection.pickle'

if not os.path.isfile(fei_pickle):
    sys.exit(
        'fei_path.pickle not found at: ' +
        fei_pickle +
        '\n'
        'Please provide the fei_path.pickle file from an existing FEI training by setting the fei_pickle parameter in this script.')

path = get_path_from_file(fei_pickle)

# read the MDST
# this is stupid, but ...
i = 0
theChunk = ['']
for fileChunk in fileChunks:
    if i == chunkN:
        theChunk = list(fileChunk)
    i = i + 1

inputMdstList('default', theChunk)

# execute path and return back to the analysis_main
# the skim condition is TRUE for all events
skimALL = register_module('VariableToReturnValue')
skimALL.param("variable", 'True')
skimALL.if_value('==1', path, AfterConditionPath.CONTINUE)
analysis_main.add_module(skimALL)

# now the FEI reconstruction is done
# and we're back in analysis_main pathB

# apply some very loose cuts to reduce the number
# of Btag candidates
applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

applyCuts('B0:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.001')
applyCuts('B+:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.001')

# rank Btag canidates according to their SignalProbability
# 'sigProb' is alias for 'extraInfo(SignalProbability)'
rankByHighest('B0:generic', 'sigProb')
rankByHighest('B+:generic', 'sigProb')

rankByHighest('B0:semileptonic', 'sigProb')
rankByHighest('B+:semileptonic', 'sigProb')

looseMCTruth('B0:generic')
looseMCTruth('B+:generic')

# create and fill flat Ntuple
toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['DeltaEMbc', '^B0']
toolsB0 += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B0']
toolsB0 += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID:d1_d0_dmID]', '^B0']
toolsB0 += ['CustomFloats[d1d2_M:d1d3_M:d1d4_M:d2d3_M:d2d4_M:d3d4_M]', '^B0']
toolsB0 += ['CustomFloats[d1d2d3_M:d2d3d4_M:d1d2d3d4_M]', '^B0']
toolsB0 += ['CustomFloats[isSignal:isExtendedSignal:looseMCMotherPDG:looseMCWrongDaughterN]', '^B0']
toolsB0 += ['CustomFloats[looseMCWrongDaughterBiB:looseMCWrongDaughterPDG]', '^B0']
toolsB0 += ['MCTruth', '^B0']

toolsBP = ['EventMetaData', '^B+']
toolsBP += ['DeltaEMbc', '^B+']
toolsBP += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+']
toolsBP += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID:d1_d0_dmID]', '^B+']
toolsBP += ['CustomFloats[d1d2_M:d1d3_M:d1d4_M:d2d3_M:d2d4_M:d3d4_M]', '^B+']
toolsBP += ['CustomFloats[d1d2d3_M:d2d3d4_M:d1d2d3d4_M]', '^B+']
toolsBP += ['CustomFloats[isSignal:isExtendedSignal:looseMCMotherPDG:looseMCWrongDaughterN]', '^B+']
toolsBP += ['CustomFloats[looseMCWrongDaughterBiB:looseMCWrongDaughterPDG]', '^B+']
toolsBP += ['MCTruth', '^B+']

toolsB0SL = ['EventMetaData', '^B0']
toolsB0SL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B0']
toolsB0SL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B0']
toolsB0SL += ['CustomFloats[d0_dmID]', '^B0']
toolsB0SL += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B0']
toolsB0SL += ['MCTruth', '^B0']

toolsBPSL = ['EventMetaData', '^B+']
toolsBPSL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B+']
toolsBPSL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+']
toolsBPSL += ['CustomFloats[d0_dmID]', '^B+']
toolsBPSL += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B+']
toolsBPSL += ['MCTruth', '^B+']

# write out the flat ntuple
ntupleFile(outputRootFile)
ntupleTree('B0HadTag', 'B0:generic', toolsB0)
ntupleTree('BPHadTag', 'B+:generic', toolsBP)
ntupleTree('B0SLTag', 'B0:semileptonic', toolsB0SL)
ntupleTree('BPSLTag', 'B+:semileptonic', toolsBPSL)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
