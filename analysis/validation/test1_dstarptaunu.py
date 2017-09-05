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


# PDG ID
variables.addAlias('d0_PDG', 'daughter(0,PDG)')
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


outputRootFile = 'dstarptaunuOutput.root'


# TODO: specify the location of database (faster)
db_location = '/home/belle/rachac/fei/fei_database'
use_central_database('test_rachac', LogLevel.WARNING, 'fei_database')
# use_local_database('dbcache.txt',db_location,False,LogLevel.WARNING)

# TODO: load the FEI reconstruction path
# specify the location of the fei_pickle file
fei_pickle = '/home/belle/rachac/fei/paths/basf2_final_path_without_selection.pickle'
if not os.path.isfile(fei_pickle):
    sys.exit(
        'fei_path.pickle not found at: ' +
        fei_pickle +
        '\n'
        'Please provide the fei_path.pickle file from an existing FEI training by setting the fei_pickle parameter in this script.')

path = get_path_from_file(fei_pickle)
fileList = ['/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000872/s00/e0000/4S/r00000/signal/sub00/' +
            'mdst_000001_prod00000872_task00000001.root',
            '/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000872/s00/e0000/4S/r00000/signal/sub00/' +
            'mdst_000002_prod00000872_task00000002.root']


inputMdstList('default', fileList)

# execute path and return back to the analysis_main
# the skim condition is TRUE for all events
skimALL = register_module('VariableToReturnValue')
skimALL.param("variable", 'True')
skimALL.if_value('==1', path, AfterConditionPath.CONTINUE)
analysis_main.add_module(skimALL)

# now the FEI reconstruction is done

# apply some very loose cuts to reduce the number
# of Btag candidates
applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

applyCuts('B0:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.001')

# rank Btag canidates according to their SignalProbability
# 'sigProb' is alias for 'extraInfo(SignalProbability)'
rankByHighest('B0:generic', 'sigProb')
rankByHighest('B0:semileptonic', 'sigProb')
looseMCTruth('B0:generic')


# Calling standard particle lists

stdK('95eff')
stdPi('95eff')
stdMu('all')
stdE('all')
stdPi0s('looseFit')
stdKshorts()
stdPhotons('all')

cutAndCopyList('e+:highP', 'e+:all', 'useCMSFrame(p)>0.3')
cutAndCopyList('mu+:highP', 'mu+:all', 'useCMSFrame(p)>0.3')
cutAndCopyList('pi+:lowP', 'pi+:95eff', 'useCMSFrame(p)<0.4')
cutAndCopyList('pi0:lowP', 'pi0:looseFit', 'useCMSFrame(p)<0.4')
cutAndCopyList('gamma:lowE', 'gamma:all', 'useCMSFrame(E)<0.4')
# D0 :

reconstructDecay('D0:kpi -> K-:95eff pi+:95eff', '1.8<InvM<1.9')
reconstructDecay('D0:kpipi0 -> K-:95eff pi+:95eff pi0:looseFit', '1.8<InvM<1.9')
reconstructDecay('D0:k3pi -> K-:95eff pi+:95eff pi-:95eff pi+:95eff', '1.8<InvM<1.9')

copyLists('D0:all', ['D0:kpi', 'D0:kpipi0', 'D0:k3pi'])

# D+:

reconstructDecay('D+:kpipi -> K-:95eff pi+:95eff pi+:95eff', '1.8<InvM<1.9')
reconstructDecay('D+:kspi -> K_S0:all pi+:95eff', '1.8<InvM<1.9')
reconstructDecay('D+:kspipi0 -> K_S0:all pi+:95eff pi0:looseFit', '1.8<InvM<1.9')

copyLists('D+:all', ['D+:kpipi', 'D+:kspi', 'D+:kspipi0'])


# D*+:

reconstructDecay('D*+:ch1 -> D0:all pi+:lowP', '0<Q<0.04')
reconstructDecay('D*+:ch2 -> D+:all pi0:lowP', '0<Q<0.04')

copyLists('D*+:all', ['D*+:ch1', 'D*+:ch2'])


# tau
reconstructDecay('tau-:ch1 -> e-:all', '')
reconstructDecay('tau-:ch2 -> mu-:all', '')
copyLists('tau-:all', ['tau-:ch1', 'tau-:ch2'])

# B0:sig

reconstructDecay('B0:ch1 -> D*+:all tau-:all', 'Mbc>0')
reconstructDecay('B0:ch2 -> D*+:all tau-:all', 'Mbc>0')
copyLists('B0:all', ['B0:ch1', 'B0:ch2'])

# Upsilon(4S)

reconstructDecay('Upsilon(4S):BhadBsig0 -> B0:generic B0:all', '')
reconstructDecay('Upsilon(4S):BslBsig0 -> B0:semileptonic B0:all', '')

buildRestOfEvent('Upsilon(4S):BhadBsig0')
buildRestOfEvent('Upsilon(4S):BslBsig0')

ROETracks = ('ROETracks', '', '')
ROEclusters = ('ROEclusters', '', '')


appendROEMasks('Upsilon(4S):BhadBsig0', [ROEclusters, ROETracks])
appendROEMasks('Upsilon(4S):BslBsig0', [ROEclusters, ROETracks])


variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_mcMissFlagsSel', 'ROE_mcMissFlags(ROEclusters)')
variables.addAlias('ROE_chargeSel', 'ROE_charge(ROEclusters)')
variables.addAlias('ROE_ESel', 'ROE_E(ROEclusters)')
variables.addAlias('ROE_PSel', 'ROE_P(ROEclusters)')
variables.addAlias('nAllROETracks', 'nROETracks(ROETracks)')
variables.addAlias('nROEECLClustersSel', 'nROEECLClusters(ROEclusters)')
variables.addAlias('nROENeutralECLClustersSel', 'nROENeutralECLClusters(ROEclusters)')


applyCuts('Upsilon(4S):BhadBsig0', 'nAllROETracks==0')
applyCuts('Upsilon(4S):BslBsig0', 'nAllROETracks==0')


# perform MC matching

matchMCTruth('e+:highP')
matchMCTruth('mu+:highP')
matchMCTruth('K+:95eff')
matchMCTruth('pi+:95eff')
matchMCTruth('pi0:looseFit')
matchMCTruth('pi0:lowP')
matchMCTruth('D0:all')
matchMCTruth('D+:all')
matchMCTruth('D*+:all')
matchMCTruth('B0:generic')
matchMCTruth('B0:semileptonic')
matchMCTruth('B0:all')


toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['DeltaEMbc', '^B0']
toolsB0 += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B0']
toolsB0 += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID:d1_d0_dmID]', '^B0']
toolsB0 += ['CustomFloats[d1d2_M:d1d3_M:d1d4_M:d2d3_M:d2d4_M:d3d4_M]', '^B0']
toolsB0 += ['CustomFloats[d1d2d3_M:d2d3d4_M:d1d2d3d4_M]', '^B0']
toolsB0 += ['CustomFloats[isSignal:isExtendedSignal:looseMCMotherPDG:looseMCWrongDaughterN]', '^B0']
toolsB0 += ['CustomFloats[looseMCWrongDaughterBiB:looseMCWrongDaughterPDG]', '^B0']
toolsB0 += ['MCTruth', '^B0']


toolsB0SL = ['EventMetaData', '^B0']
toolsB0SL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B0']
toolsB0SL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B0']
toolsB0SL += ['CustomFloats[d0_dmID]', '^B0']
toolsB0SL += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B0']
toolsB0SL += ['MCTruth', '^B0']


tools4SB0Had = ['EventMetaData', '^Upsilon(4S)']
tools4SB0Had += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S)']
tools4SB0Had += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S)']
tools4SB0Had += ['CustomFloats[R2:cosTBTO:cosTBz]', 'Upsilon(4S) -> ^B0:generic anti-B0:all']
tools4SB0Had += ['CustomFloats[sigProb:rank]', 'Upsilon(4S) -> ^B0:generic anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS:d0_d1_costhetaCMS:
                               d0_d1_d0_costhetaCMS:d0_d1_d1_costhetaCMS]',
                 'Upsilon(4S) ->  B0:generic ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_d0_E:d0_d1_E]', 'Upsilon(4S) ->  B0:generic ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S) ->  ^B0:generic ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_p:d0_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_pCMS:d0_d1_d0_pCMS:d0_d1_d1_pCMS]',
                 'Upsilon(4S) ->  B0:generic ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_mcPDG:d1_mcPDG:d1_d0_mcPDG:d1_d0_d0_mcPDG:d1_d0_d1_mcPDG]',
                 '^Upsilon(4S) ->  B0:generic anti-B0:all']
tools4SB0Had += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID:d0_d1_dmID:d1_d0_dmID]', 'Upsilon(4S) ->  ^B0:generic ^anti-B0:all']
tools4SB0Had += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                 'Upsilon(4S) ->  ^B0:generic ^anti-B0:all']
tools4SB0Had += ['MCTruth', 'Upsilon(4S) ->  ^B0:generic ^anti-B0:all']


tools4SB0SL = ['EventMetaData', '^Upsilon(4S)']
tools4SB0SL += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S)']
tools4SB0SL += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S)']
tools4SB0SL += ['CustomFloats[R2:cosTBTO:cosTBz]', 'Upsilon(4S) -> ^B0:semileptonic anti-B0:all']
tools4SB0SL += ['CustomFloats[sigProb:rank]', 'Upsilon(4S) -> ^B0:semileptonic anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS:d0_d1_costhetaCMS:
                              d0_d1_d0_costhetaCMS:d0_d1_d1_costhetaCMS]',    'Upsilon(4S) -> B0:semileptonic ^ anti - B0:all']
tools4SB0SL += ['CustomFloats[d0_d0_E:d0_d1_E]', 'Upsilon(4S) ->  B0:semileptonic ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S) ->  ^B0:semileptonic ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_p:d0_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_pCMS:d0_d1_d0_pCMS:d0_d1_d1_pCMS]',
                'Upsilon(4S) ->  B+:semileptonic ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_mcPDG:d1_mcPDG:d1_d0_mcPDG:d1_d0_d0_mcPDG:d1_d0_d1_mcPDG]',
                '^Upsilon(4S) ->  B0:semileptonic anti-B0:all']
tools4SB0SL += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID:d0_d1_dmID:d1_d0_dmID]',
                'Upsilon(4S) ->  ^B0:semileptonic ^anti-B0:all']
tools4SB0SL += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                'Upsilon(4S) ->  ^B0:semileptonic ^anti-B0:all']
tools4SB0SL += ['MCTruth', 'Upsilon(4S) ->  ^B0:semileptonic ^anti-B0:all']


# write out the flat ntuple
ntupleFile(outputRootFile)
ntupleTree('Ups:B0Had', 'Upsilon(4S):BhadBsig0', tools4SB0Had)
ntupleTree('B0HadTag', 'B0:generic', toolsB0)
ntupleTree('B0SLTag', 'B0:semileptonic', toolsB0SL)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
