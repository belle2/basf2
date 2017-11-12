#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI and how to validate D(*)lv and D(*)
#
#
# This script is used as well for the production of
# of official monitoring and validation single tag samples.
#
# Contributors: Racha Cheaib April 2017
#
#  FEI training copied from Anze Zupanc directory:
# /home/belle/zupanc/belle2/physics/FEI/Belle2_Generic_2016_1_looseMCMatching
#
#
#  Release required: build-2017-03-06
#
######################################################
import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from stdCharged import *
from stdPi0s import *
from stdV0s import *
gb2_setuprel = "build-2017-03-06"


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


variables.addAlias('d0_d1_dmID', 'daughter(0,daughter(1,extraInfo(decayModeID)))')


# Momenta
variables.addAlias('d0_p', 'daughter(0,p)')
variables.addAlias('d0_pCMS', 'daughter(0,useCMSFrame(p))')
variables.addAlias('d1_p', 'daughter(1,p)')
variables.addAlias('d2_p', 'daughter(2,p)')
variables.addAlias('d3_p', 'daughter(3,p)')
variables.addAlias('d0_d0_p', 'daughter(0,daughter(0,p))')
variables.addAlias('d0_d0_pCMS', 'daughter(0,daughter(0,useCMSFrame(p)))')
variables.addAlias('d0_d1_pCMS', 'daughter(0,daughter(1,useCMSFrame(p)))')
variables.addAlias('d0_d1_p', 'daughter(0,daughter(1,p))')
variables.addAlias('d0_d2_p', 'daughter(0,daughter(2,p))')
variables.addAlias('d0_d3_p', 'daughter(0,daughter(3,p))')
variables.addAlias('d0_d1_d0_p', 'daughter(0,daughter(1,daughter(0,p)))')
variables.addAlias('d0_d1_d1_p', 'daughter(0,daughter(1,daughter(1,p)))')
variables.addAlias('d0_d1_d0_pCMS', 'daughter(0,daughter(1,daughter(0,useCMSFrame(p))))')
variables.addAlias('d0_d1_d1_pCMS', 'daughter(0,daughter(1,daughter(1,useCMSFrame(p))))')
variables.addAlias('d0_d0_d2_p', 'daughter(0,daughter(0,daughter(2,p)))')
variables.addAlias('d0_d0_d3_p', 'daughter(0,daughter(0,daughter(3,p)))')

variables.addAlias('d0_d0_M', 'daughter(0,daughter(0,InvM))')
variables.addAlias('d0_d1_M', 'daughter(0,daughter(1,InvM))')
variables.addAlias('d0_d0_d1_M', 'daughter(0,daughter(0,daughter(1,InvM)))')
variables.addAlias('d0_M', 'daughter(0,InvM)')
variables.addAlias('d1_M', 'daughter(1,InvM)')

variables.addAlias('d0_d0_costheta', 'daughter(0,daughter(0,cosTheta))')
variables.addAlias('d0_costheta', 'daughter(0,cosTheta)')
variables.addAlias('d0_costhetaCMS', 'daughter(0,useCMSFrame(cosTheta))')
variables.addAlias('d0_d0_costhetaCMS', 'daughter(0,daughter(0,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_costhetaCMS', 'daughter(0,daughter(1,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_d0_costhetaCMS', 'daughter(0,daughter(1,daughter(0,useCMSFrame(cosTheta))))')
variables.addAlias('d0_d1_d1_costhetaCMS', 'daughter(0,daughter(1,daughter(1,useCMSFrame(cosTheta))))')
variables.addAlias('d1_costheta', 'daughter(1,cosTheta)')

variables.addAlias('d0_d0_E', 'daughter(0,daughter(0,E))')
variables.addAlias('d0_d1_E', 'daughter(0,daughter(1,E))')
variables.addAlias('d0_E', 'daughter(0,E)')
variables.addAlias('d1_E', 'daughter(1,E)')

variables.addAlias('d0_deltaE', 'daughter(0,deltaE)')
variables.addAlias('d0_Mbc', 'daughter(0,Mbc)')
variables.addAlias('d1_deltaE', 'daughter(1,deltaE)')
variables.addAlias('d1_Mbc', 'daughter(1,Mbc)')


# PDG ID ariables.addAlias('d0_PDG',               'daughter(0,PDG)')
variables.addAlias('d1_PDG', 'daughter(1,PDG)')
variables.addAlias('d2_PDG', 'daughter(2,PDG)')
variables.addAlias('d3_PDG', 'daughter(3,PDG)')
variables.addAlias('d0_d0_PDG', 'daughter(0,daughter(0,PDG))')
variables.addAlias('d0_d1_PDG', 'daughter(0,daughter(1,PDG))')
variables.addAlias('d0_d2_PDG', 'daughter(0,daughter(2,PDG))')
variables.addAlias('d0_d3_PDG', 'daughter(0,daughter(3,PDG))')
variables.addAlias('d0_d1_d0_PDG', 'daughter(0,daughter(1,daughter(0,PDG)))')
variables.addAlias('d0_d1_d1_PDG', 'daughter(0,daughter(1,daughter(1,PDG)))')
variables.addAlias('d0_d0_d2_PDG', 'daughter(0,daughter(0,daughter(2,PDG)))')
variables.addAlias('d0_d0_d3_PDG', 'daughter(0,daughter(0,daughter(3,PDG)))')


variables.addAlias('d0_mcPDG', 'daughter(0,mcPDG)')
variables.addAlias('d1_mcPDG', 'daughter(1,mcPDG)')
variables.addAlias('d1_d0_mcPDG', 'daughter(1,daughter(0,mcPDG))')
variables.addAlias('d1_d0_d0_mcPDG', 'daughter(1,daughter(0,daughter(0,mcPDG)))')
variables.addAlias('d1_d0_d1_mcPDG', 'daughter(1,daughter(0,daughter(1,mcPDG)))')
variables.addAlias('d1_eid', 'daughter(1,electronID)')
variables.addAlias('d1_muid', 'daughter(1,muonID)')

outputRootFile = 'dstarlnuOutput.root'

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()


# TODO: specify the location of database (faster)
use_central_database('production', LogLevel.WARNING, 'fei_database')

path = create_path()

import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)

# fileList = ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
#            'mdst_001724_prod00000962_task00001729.root'
#            ]

fileList = ["../1263700001.dst.root"]

inputMdstList('default', fileList)

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
applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
applyCuts('B+:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.001')

# rank Btag canidates according to their SignalProbability
# 'sigProb' is alias for 'extraInfo(SignalProbability)'
rankByHighest('B+:generic', 'sigProb')

rankByHighest('B+:semileptonic', 'sigProb')

looseMCTruth('B+:generic')
# ranking B mesons
cutAndCopyList('B+:genericRank', 'B+:generic', 'rank==1')
cutAndCopyList('B+:semileptonicRank', 'B+:semileptonic', 'rank==1')


# Calling standard particle lists

stdK('95eff')
stdPi('95eff')
stdMu('all')
stdE('all')
stdPi0s('looseFit')
stdKshorts()
stdPhotons('loose')

cutAndCopyList('e+:highP', 'e+:all', 'useCMSFrame(p)>0.3')
cutAndCopyList('mu+:highP', 'mu+:all', 'useCMSFrame(p)>0.3')
cutAndCopyList('pi+:lowP', 'pi+:95eff', 'useCMSFrame(p)<0.4')
cutAndCopyList('pi0:lowP', 'pi0:looseFit', 'useCMSFrame(p)<0.4')
cutAndCopyList('gamma:lowE', 'gamma:loose', 'useCMSFrame(E)<0.4')
# D0 :

reconstructDecay('D0:kpi -> K-:95eff pi+:95eff', '1.8<InvM<1.9')

copyLists('D0:all', ['D0:kpi'])  # , 'D0:kpipi0', 'D0:kspipi'])


# D*0

reconstructDecay('D*0:ch1 -> D0:all pi0:lowP', '0<Q<0.04')
reconstructDecay('D*0:ch2 -> D0:all gamma:lowE', '0<Q<0.04')
copyLists('D*0:all', ['D*0:ch1', 'D*0:ch2'])

# B+:sig
reconstructDecay('B+:ch1 -> D*0:all e+:highP', 'Mbc>0')
reconstructDecay('B+:ch2 -> D*0:all mu+:highP', 'Mbc>0')
copyLists('B+:all', ['B+:ch1', 'B+:ch2'])


# Upsilon(4S)

reconstructDecay('Upsilon(4S):BhadBsig+ -> B+:genericRank B-:all', '')
reconstructDecay('Upsilon(4S):BslBsig+ -> B+:semileptonicRank B-:all', '')

buildRestOfEvent('Upsilon(4S):BhadBsig+')
buildRestOfEvent('Upsilon(4S):BslBsig+')

ROETracks = ('ROETracks', '', '')
ROEclusters = ('ROEclusters', '', '')


appendROEMasks('Upsilon(4S):BhadBsig+', [ROEclusters, ROETracks])
appendROEMasks('Upsilon(4S):BslBsig+', [ROEclusters, ROETracks])


variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_mcMissFlagsSel', 'ROE_mcMissFlags(ROEclusters)')
variables.addAlias('ROE_chargeSel', 'ROE_charge(ROEclusters)')
variables.addAlias('ROE_ESel', 'ROE_E(ROEclusters)')
variables.addAlias('ROE_PSel', 'ROE_P(ROEclusters)')
variables.addAlias('nAllROETracks', 'nROETracks(ROETracks)')
variables.addAlias('nROEECLClustersSel', 'nROEECLClusters(ROEclusters)')
variables.addAlias('nROENeutralECLClustersSel', 'nROENeutralECLClusters(ROEclusters)')


applyCuts('Upsilon(4S):BhadBsig+', 'nAllROETracks==0')
applyCuts('Upsilon(4S):BslBsig+', 'nAllROETracks==0')


# perform MC matching

matchMCTruth('e+:highP')
matchMCTruth('mu+:highP')
matchMCTruth('K+:95eff')
matchMCTruth('pi+:95eff')
matchMCTruth('pi0:looseFit')
matchMCTruth('pi0:lowP')
matchMCTruth('D0:all')
matchMCTruth('D*0:all')
matchMCTruth('B+:genericRank')
matchMCTruth('B+:semileptonicRank')
matchMCTruth('B+:all')


toolsBP = ['EventMetaData', '^B+']
toolsBP += ['DeltaEMbc', '^B+']
toolsBP += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+']
toolsBP += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID:d1_d0_dmID]', '^B+']
toolsBP += ['CustomFloats[d1d2_M:d1d3_M:d1d4_M:d2d3_M:d2d4_M:d3d4_M]', '^B+']
toolsBP += ['CustomFloats[d1d2d3_M:d2d3d4_M:d1d2d3d4_M]', '^B+']
toolsBP += ['CustomFloats[isSignal:isExtendedSignal:looseMCMotherPDG:looseMCWrongDaughterN]', '^B+']
toolsBP += ['CustomFloats[looseMCWrongDaughterBiB:looseMCWrongDaughterPDG]', '^B+']
toolsBP += ['MCTruth', '^B+']


toolsBPSL = ['EventMetaData', '^B+']
toolsBPSL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B+']
toolsBPSL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+']
toolsBPSL += ['CustomFloats[d0_dmID]', '^B+']
toolsBPSL += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B+']
toolsBPSL += ['MCTruth', '^B+']


tools4SBplusHad = ['EventMetaData', '^Upsilon(4S)']
tools4SBplusHad += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S)']
tools4SBplusHad += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S)']
tools4SBplusHad += ['CustomFloats[R2:cosTBTO:cosTBz]', 'Upsilon(4S) -> ^B+:genericRank B-:all']
tools4SBplusHad += ['CustomFloats[sigProb:rank]', 'Upsilon(4S) -> ^B+:genericRank B-:all']
tools4SBplusHad += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                    'Upsilon(4S) ->  B+:genericRank ^B-:all']

tools4SBplusHad += ['CustomFloats[d0_costheta:d0_d1_costhetaCMS:d0_d1_d0_costhetaCMS:d0_d1_d1_costhetaCMS]',
                    'Upsilon(4S) ->  B+:genericRank ^B-:all']

tools4SBplusHad += ['CustomFloats[d0_d0_E:d0_d1_E]', 'Upsilon(4S) ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S) ->  ^B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_p:d0_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_pCMS:d0_d1_d0_pCMS:d0_d1_d1_pCMS]',
                    'Upsilon(4S) ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_mcPDG:d1_mcPDG:d1_d0_mcPDG:d1_d0_d0_mcPDG:d1_d0_d1_mcPDG]',
                    '^Upsilon(4S) ->  B+:genericRank B-:all']
tools4SBplusHad += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID:d0_d1_dmID:d1_d0_dmID]',
                    'Upsilon(4S) ->  ^B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                    'Upsilon(4S) ->  ^B+:genericRank ^B-:all']
tools4SBplusHad += ['MCTruth', 'Upsilon(4S) ->  ^B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d1_eid:d1_muid]', 'Upsilon(4S) -> B+:genericRank ^B-:all']

tools4SBplusSL = ['EventMetaData', '^Upsilon(4S)']
tools4SBplusSL += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S)']
tools4SBplusSL += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S)']
tools4SBplusSL += ['CustomFloats[R2:cosTBTO:cosTBz]', 'Upsilon(4S) -> ^B+:semileptonicRank B-:all']
tools4SBplusSL += ['CustomFloats[sigProb:rank]', 'Upsilon(4S) -> ^B+:semileptonicRank B-:all']
tools4SBplusSL += ['CustomFloats[d0_d0_E:d0_d1_E]', 'Upsilon(4S) ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_p:d0_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_pCMS:d0_d1_d0_pCMS:d0_d1_d1_pCMS]',
                   'Upsilon(4S) ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_mcPDG:d1_mcPDG:d1_d0_mcPDG:d1_d0_d0_mcPDG:d1_d0_d1_mcPDG]',
                   '^Upsilon(4S) ->  B+:semileptonicRank B-:all']
tools4SBplusSL += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID:d0_d1_dmID:d1_d0_dmID]',
                   'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                   'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['MCTruth', 'Upsilon(4S) ->  ^B+:semileptonicRank ^B-:all']


# Make lists and outptu to mdst and udst files.
# write out the flat ntuple
ntupleFile(outputRootFile)
ntupleTree('Ups:BplusSL', 'Upsilon(4S):BslBsig+', tools4SBplusSL)
ntupleTree('Ups:BplusHad', 'Upsilon(4S):BhadBsig+', tools4SBplusHad)
ntupleTree('BPHadTag', 'B+:generic', toolsBP)
ntupleTree('BPSLTag', 'B+:semileptonic', toolsBPSL)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
