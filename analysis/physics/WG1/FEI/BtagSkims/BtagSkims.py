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
from modularAnalysis import *

# this function removes a module from a given path
# The RootInput module will be taken from the fei path
# and will be replaced with RootInput module in the
# analysis_main path


def remove_module(path, name):
    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path

# create aliases for the complex functions so that
# their names in the ntuple are human readable
from variables import variables
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('rank', 'extraInfo(sigProb_rank)')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
variables.addAlias('uniqueSignal', 'extraInfo(uniqueSignal)')
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

# load the FEI reconstruction path
# specify the location of the fei_pickle file
fei_pickle = ''
if not os.path.isfile(fei_pickle):
    sys.exit(
        'fei_path.pickle not found at: ' +
        fei_pickle +
        '\n'
        'Please provide the fei_path.pickle file from an existing FEI training by setting the fei_pickle parameter in this script.')

path = get_path_from_file(fei_pickle)
path = remove_module(path, 'RootInput')
path = remove_module(path, 'ProgressBar')

beamparameters = add_beamparameters(path, 'Y4S')

# read the MDST
# this is stupid, but ...
i = 0
theChunk = ['']
for fileChunk in fileChunks:
    if i == chunkN:
        theChunk = list(fileChunk)
    i = i + 1

inputMdstList(theChunk)

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
cutAndCopyList('B0:HadTagGood', 'B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
cutAndCopyList('B+:HadTagGood', 'B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

cutAndCopyList('B0:SLTagGood', 'B0:semileptonic', 'abs(cosThetaBetweenParticleAndNominalB)<10 and sigProb>0.001')
cutAndCopyList('B+:SLTagGood', 'B+:semileptonic', 'abs(cosThetaBetweenParticleAndNominalB)<10 and sigProb>0.001')

# this interface will change in the future
buildRestOfEvent('B0:generic')
buildRestOfEvent('B+:generic')
buildRestOfEvent('B0:semileptonic')
buildRestOfEvent('B+:semileptonic')

# ---------
# Btag + lepton
#
electrons = ('e-:loose', 'DLLKaon>-5 and DLLPion>-5')
muons = ('mu-:loose', 'DLLKaon>-5 and DLLPion>-5')

fillParticleLists([electrons, muons])

# ---------
reconstructDecay('Upsilon(4S):BPHTagGood_elec_RS -> B+:HadTagGood e-:loose', '')
reconstructDecay('Upsilon(4S):BPHTagGood_elec_WS -> B+:HadTagGood e+:loose', '')
reconstructDecay('Upsilon(4S):BPHTagGood_muon_RS -> B+:HadTagGood mu-:loose', '')
reconstructDecay('Upsilon(4S):BPHTagGood_muon_WS -> B+:HadTagGood mu+:loose', '')

reconstructDecay('Upsilon(4S):B0HTagGood_elec_RS -> B0:HadTagGood e-:loose', '')
reconstructDecay('Upsilon(4S):B0HTagGood_elec_WS -> B0:HadTagGood e+:loose', '')
reconstructDecay('Upsilon(4S):B0HTagGood_muon_RS -> B0:HadTagGood mu-:loose', '')
reconstructDecay('Upsilon(4S):B0HTagGood_muon_WS -> B0:HadTagGood mu+:loose', '')

reconstructDecay('Upsilon(4S):BPSLTagGood_elec_RS -> B+:SLTagGood e-:loose', '')
reconstructDecay('Upsilon(4S):BPSLTagGood_elec_WS -> B+:SLTagGood e+:loose', '')
reconstructDecay('Upsilon(4S):BPSLTagGood_muon_RS -> B+:SLTagGood mu-:loose', '')
reconstructDecay('Upsilon(4S):BPSLTagGood_muon_WS -> B+:SLTagGood mu+:loose', '')

reconstructDecay('Upsilon(4S):B0SLTagGood_elec_RS -> B0:SLTagGood e-:loose', '')
reconstructDecay('Upsilon(4S):B0SLTagGood_elec_WS -> B0:SLTagGood e+:loose', '')
reconstructDecay('Upsilon(4S):B0SLTagGood_muon_RS -> B0:SLTagGood mu-:loose', '')
reconstructDecay('Upsilon(4S):B0SLTagGood_muon_WS -> B0:SLTagGood mu+:loose', '')

copyLists('Upsilon(4S):BPHTagGood_ell_RS', ['Upsilon(4S):BPHTagGood_elec_RS', 'Upsilon(4S):BPHTagGood_muon_RS'])
copyLists('Upsilon(4S):BPHTagGood_ell_WS', ['Upsilon(4S):BPHTagGood_elec_WS', 'Upsilon(4S):BPHTagGood_muon_WS'])
copyLists('Upsilon(4S):B0HTagGood_ell_RS', ['Upsilon(4S):B0HTagGood_elec_RS', 'Upsilon(4S):B0HTagGood_muon_RS'])
copyLists('Upsilon(4S):B0HTagGood_ell_WS', ['Upsilon(4S):B0HTagGood_elec_WS', 'Upsilon(4S):B0HTagGood_muon_WS'])
copyLists('Upsilon(4S):BPHTagGood_ell', ['Upsilon(4S):BPHTagGood_ell_WS', 'Upsilon(4S):BPHTagGood_ell_RS'])
copyLists('Upsilon(4S):B0HTagGood_ell', ['Upsilon(4S):B0HTagGood_ell_WS', 'Upsilon(4S):B0HTagGood_ell_RS'])

copyLists('Upsilon(4S):BPSLTagGood_ell_RS', ['Upsilon(4S):BPSLTagGood_elec_RS', 'Upsilon(4S):BPSLTagGood_muon_RS'])
copyLists('Upsilon(4S):BPSLTagGood_ell_WS', ['Upsilon(4S):BPSLTagGood_elec_WS', 'Upsilon(4S):BPSLTagGood_muon_WS'])
copyLists('Upsilon(4S):B0SLTagGood_ell_RS', ['Upsilon(4S):B0SLTagGood_elec_RS', 'Upsilon(4S):B0SLTagGood_muon_RS'])
copyLists('Upsilon(4S):B0SLTagGood_ell_WS', ['Upsilon(4S):B0SLTagGood_elec_WS', 'Upsilon(4S):B0SLTagGood_muon_WS'])
copyLists('Upsilon(4S):BPSLTagGood_ell', ['Upsilon(4S):BPSLTagGood_ell_WS', 'Upsilon(4S):BPSLTagGood_ell_RS'])
copyLists('Upsilon(4S):B0SLTagGood_ell', ['Upsilon(4S):B0SLTagGood_ell_WS', 'Upsilon(4S):B0SLTagGood_ell_RS'])

# ---------
reconstructDecay('Upsilon(4S):BPHTagAll_elec_RS -> B+:generic e-:loose', '')
reconstructDecay('Upsilon(4S):BPHTagAll_elec_WS -> B+:generic e+:loose', '')
reconstructDecay('Upsilon(4S):BPHTagAll_muon_RS -> B+:generic mu-:loose', '')
reconstructDecay('Upsilon(4S):BPHTagAll_muon_WS -> B+:generic mu+:loose', '')

reconstructDecay('Upsilon(4S):B0HTagAll_elec_RS -> B0:generic e-:loose', '')
reconstructDecay('Upsilon(4S):B0HTagAll_elec_WS -> B0:generic e+:loose', '')
reconstructDecay('Upsilon(4S):B0HTagAll_muon_RS -> B0:generic mu-:loose', '')
reconstructDecay('Upsilon(4S):B0HTagAll_muon_WS -> B0:generic mu+:loose', '')

reconstructDecay('Upsilon(4S):BPSLTagAll_elec_RS -> B+:semileptonic e-:loose', '')
reconstructDecay('Upsilon(4S):BPSLTagAll_elec_WS -> B+:semileptonic e+:loose', '')
reconstructDecay('Upsilon(4S):BPSLTagAll_muon_RS -> B+:semileptonic mu-:loose', '')
reconstructDecay('Upsilon(4S):BPSLTagAll_muon_WS -> B+:semileptonic mu+:loose', '')

reconstructDecay('Upsilon(4S):B0SLTagAll_elec_RS -> B0:semileptonic e-:loose', '')
reconstructDecay('Upsilon(4S):B0SLTagAll_elec_WS -> B0:semileptonic e+:loose', '')
reconstructDecay('Upsilon(4S):B0SLTagAll_muon_RS -> B0:semileptonic mu-:loose', '')
reconstructDecay('Upsilon(4S):B0SLTagAll_muon_WS -> B0:semileptonic mu+:loose', '')

copyLists('Upsilon(4S):BPHTagAll_ell_RS', ['Upsilon(4S):BPHTagAll_elec_RS', 'Upsilon(4S):BPHTagAll_muon_RS'])
copyLists('Upsilon(4S):BPHTagAll_ell_WS', ['Upsilon(4S):BPHTagAll_elec_WS', 'Upsilon(4S):BPHTagAll_muon_WS'])
copyLists('Upsilon(4S):B0HTagAll_ell_RS', ['Upsilon(4S):B0HTagAll_elec_RS', 'Upsilon(4S):B0HTagAll_muon_RS'])
copyLists('Upsilon(4S):B0HTagAll_ell_WS', ['Upsilon(4S):B0HTagAll_elec_WS', 'Upsilon(4S):B0HTagAll_muon_WS'])
copyLists('Upsilon(4S):BPHTagAll_ell', ['Upsilon(4S):BPHTagAll_ell_WS', 'Upsilon(4S):BPHTagAll_ell_RS'])
copyLists('Upsilon(4S):B0HTagAll_ell', ['Upsilon(4S):B0HTagAll_ell_WS', 'Upsilon(4S):B0HTagAll_ell_RS'])

copyLists('Upsilon(4S):BPSLTagAll_ell_RS', ['Upsilon(4S):BPSLTagAll_elec_RS', 'Upsilon(4S):BPSLTagAll_muon_RS'])
copyLists('Upsilon(4S):BPSLTagAll_ell_WS', ['Upsilon(4S):BPSLTagAll_elec_WS', 'Upsilon(4S):BPSLTagAll_muon_WS'])
copyLists('Upsilon(4S):B0SLTagAll_ell_RS', ['Upsilon(4S):B0SLTagAll_elec_RS', 'Upsilon(4S):B0SLTagAll_muon_RS'])
copyLists('Upsilon(4S):B0SLTagAll_ell_WS', ['Upsilon(4S):B0SLTagAll_elec_WS', 'Upsilon(4S):B0SLTagAll_muon_WS'])
copyLists('Upsilon(4S):BPSLTagAll_ell', ['Upsilon(4S):BPSLTagAll_ell_WS', 'Upsilon(4S):BPSLTagAll_ell_RS'])
copyLists('Upsilon(4S):B0SLTagAll_ell', ['Upsilon(4S):B0SLTagAll_ell_WS', 'Upsilon(4S):B0SLTagAll_ell_RS'])

# create skim summaries
# -------------
# Btag all skim
summaryOfLists(['B0:generic', 'B+:generic'])
summaryOfLists(['B0:semileptonic', 'B+:semileptonic'])
summaryOfLists(['B0:generic', 'B+:generic', 'B0:semileptonic', 'B+:semileptonic'])
summaryOfLists(['B0:generic', 'B0:semileptonic'])
summaryOfLists(['B+:generic', 'B+:semileptonic'])

# -------------
# Btag good skim
summaryOfLists(['B0:HadTagGood', 'B+:HadTagGood'])
summaryOfLists(['B0:SLTagGood', 'B+:SLTagGood'])
summaryOfLists(['B0:HadTagGood', 'B+:HadTagGood', 'B0:SLTagGood', 'B+:SLTagGood'])
summaryOfLists(['B0:HadTagGood', 'B0:SLTagGood'])
summaryOfLists(['B+:HadTagGood', 'B+:SLTagGood'])

# -------------
# Btag good + ell (RS) skim
summaryOfLists(['B0:HadTagGood_ell_RS', 'B+:HadTagGood_ell_RS'])
summaryOfLists(['B0:SLTagGood_ell_RS', 'B+:SLTagGood_ell_RS'])
summaryOfLists(['B0:HadTagGood_ell_RS', 'B+:HadTagGood_ell_RS', 'B0:SLTagGood_ell_RS', 'B+:SLTagGood_ell_RS'])
summaryOfLists(['B0:HadTagGood_ell_RS', 'B0:SLTagGood_ell_RS'])
summaryOfLists(['B+:HadTagGood_ell_RS', 'B+:SLTagGood_ell_RS'])

# -------------
# Btag good + ell (RS+WS) skim
summaryOfLists(['B0:HadTagGood_ell', 'B+:HadTagGood_ell'])
summaryOfLists(['B0:SLTagGood_ell', 'B+:SLTagGood_ell'])
summaryOfLists(['B0:HadTagGood_ell', 'B+:HadTagGood_ell', 'B0:SLTagGood_ell', 'B+:SLTagGood_ell'])
summaryOfLists(['B0:HadTagGood_ell', 'B0:SLTagGood_ell'])
summaryOfLists(['B+:HadTagGood_ell', 'B+:SLTagGood_ell'])

# -------------
# Btag all + ell (RS) skim
summaryOfLists(['B0:HadTagAll_ell_RS', 'B+:HadTagAll_ell_RS'])
summaryOfLists(['B0:SLTagAll_ell_RS', 'B+:SLTagAll_ell_RS'])
summaryOfLists(['B0:HadTagAll_ell_RS', 'B+:HadTagAll_ell_RS', 'B0:SLTagAll_ell_RS', 'B+:SLTagAll_ell_RS'])
summaryOfLists(['B0:HadTagAll_ell_RS', 'B0:SLTagAll_ell_RS'])
summaryOfLists(['B+:HadTagAll_ell_RS', 'B+:SLTagAll_ell_RS'])

# -------------
# Btag all + ell (RS+WS) skim
summaryOfLists(['B0:HadTagAll_ell', 'B+:HadTagAll_ell'])
summaryOfLists(['B0:SLTagAll_ell', 'B+:SLTagAll_ell'])
summaryOfLists(['B0:HadTagAll_ell', 'B+:HadTagAll_ell', 'B0:SLTagAll_ell', 'B+:SLTagAll_ell'])
summaryOfLists(['B0:HadTagAll_ell', 'B0:SLTagAll_ell'])
summaryOfLists(['B+:HadTagAll_ell', 'B+:SLTagAll_ell'])


# Process the events
process(analysis_main)

# print out the summary
print(statistics)

#  LocalWords:  summaryOfLists
