#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<input>../1263700001.dst.root</input>
<output>../1263700001.ntup.root</output>
<contact> Racha Cheaib rcheaib@olemiss.edu, Mario Merola mario.merola@na.infn.it</contact>
<interval>nightly</interval>
</header>
"""
#######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI and how to validate D(*)lv
#
#  Release required: release-01-00-00
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
gb2_setuprel = "build-2017-09-08"

# reset_database()
use_local_database('/cvmfs/belle.cern.ch/conditions/GT_gen_prod_003.01_Master-20170721-132500-FEI-skim-a.txt', readonly=True)


from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

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
variables.addAlias('d1_pCMS', 'daughter(1,useCMSFrame(p))')
variables.addAlias('d0_d0_p', 'daughter(0,daughter(0,p))')
variables.addAlias('d0_d0_pCMS', 'daughter(0,daughter(0,useCMSFrame(p)))')
variables.addAlias('d0_d1_pCMS', 'daughter(0,daughter(1,useCMSFrame(p)))')
variables.addAlias('d0_d1_p', 'daughter(0,daughter(1,p))')


variables.addAlias('d0_d0_M', 'daughter(0,daughter(0,InvM))')
variables.addAlias('d0_d1_M', 'daughter(0,daughter(1,InvM))')
variables.addAlias('d0_d0_d1_M', 'daughter(0,daughter(0,daughter(1,InvM)))')
variables.addAlias('d0_M', 'daughter(0,InvM)')
variables.addAlias('d0_d0_d0_M', 'daughter(0,daughter(0,daughter(0,InvM)))')
variables.addAlias('d1_M', 'daughter(1,InvM)')

variables.addAlias('d0_d0_costheta', 'daughter(0,daughter(0,cosTheta))')
variables.addAlias('d0_costheta', 'daughter(0,cosTheta)')
variables.addAlias('d0_costhetaCMS', 'daughter(0,useCMSFrame(cosTheta))')
variables.addAlias('d0_d0_costhetaCMS', 'daughter(0,daughter(0,useCMSFrame(cosTheta)))')

variables.addAlias('d0_d0_E', 'daughter(0,daughter(0,E))')
variables.addAlias('d0_d1_E', 'daughter(0,daughter(1,E))')
variables.addAlias('d0_E', 'daughter(0,E)')
variables.addAlias('d1_E', 'daughter(1,E)')

variables.addAlias('d0_deltaE', 'daughter(0,deltaE)')
variables.addAlias('d0_Mbc', 'daughter(0,Mbc)')
variables.addAlias('d1_deltaE', 'daughter(1,deltaE)')
variables.addAlias('d1_Mbc', 'daughter(1,Mbc)')


variables.addAlias('d0_mcPDG', 'daughter(0,mcPDG)')
variables.addAlias('d1_mcPDG', 'daughter(1,mcPDG)')
variables.addAlias('d0_d0_mcPDG', 'daughter(1,daughter(0,mcPDG))')
variables.addAlias('d0_d1_mcPDG', 'daughter(1,daughter(0,mcPDG))')

# PID variables
# B daughter lepton PID
variables.addAlias('d1_electronID', 'daughter(1,electronID)')
variables.addAlias('d1_muonID', 'daughter(1,muonID)')
variables.addAlias('d1_pionID', 'daughter(1,pionID)')
variables.addAlias('d1_kaonID', 'daughter(1,kaonID)')


# D0 daughters: K- pi+ PID
variables.addAlias('d0_d0_d0_electronID', 'daughter(0,daughter(0,daughter(0,electronID)))')
variables.addAlias('d0_d0_d0_muonID', 'daughter(0,daughter(0,daughter(0,muonID)))')
variables.addAlias('d0_d0_d0_pionID', 'daughter(0,daughter(0,daughter(0,pionID)))')
variables.addAlias('d0_d0_d0_kaonID', 'daughter(0,daughter(0,daughter(0,kaonID)))')


variables.addAlias('d0_d0_d1_electronID', 'daughter(0,daughter(0,daughter(1,electronID)))')
variables.addAlias('d0_d0_d1_muonID', 'daughter(0,daughter(0,daughter(1,muonID)))')
variables.addAlias('d0_d0_d1_pionID', 'daughter(0,daughter(0,daughter(1,pionID)))')
variables.addAlias('d0_d0_d1_kaonID', 'daughter(0,daughter(0,daughter(1,kaonID)))')

path = create_path()

import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)


fileList = ['../1263700001.dst.root']

inputMdstList('default', fileList)
outputRootFile = '../1263700001.ntup.root'
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
reconstructDecay('B-:ch1 -> D*0:all e+:highP', 'Mbc>0')
reconstructDecay('B-:ch2 -> D*0:all mu+:highP', 'Mbc>0')
copyLists('B-:all', ['B-:ch1', 'B-:ch2'])


# Upsilon(4S)

reconstructDecay('Upsilon(4S):BhadBsigp -> B+:genericRank B-:all', '')
reconstructDecay('Upsilon(4S):BslBsigp -> B+:semileptonicRank B-:all', '')

buildRestOfEvent('Upsilon(4S):BhadBsigp')
buildRestOfEvent('Upsilon(4S):BslBsigp')

ROETracks = ('ROETracks', '', '')
ROEclusters = ('ROEclusters', '', 'abs(clusterTiming)<clusterErrorTiming and E>0.05')


appendROEMasks('Upsilon(4S):BhadBsigp', [ROEclusters, ROETracks])
appendROEMasks('Upsilon(4S):BslBsigp', [ROEclusters, ROETracks])


variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_mcMissFlagsSel', 'ROE_mcMissFlags(ROEclusters)')
variables.addAlias('ROE_chargeSel', 'ROE_charge(ROEclusters)')
variables.addAlias('ROE_ESel', 'ROE_E(ROEclusters)')
variables.addAlias('ROE_PSel', 'ROE_P(ROEclusters)')
variables.addAlias('nAllROETracks', 'nROETracks(ROETracks)')
variables.addAlias('nROEECLClustersSel', 'nROEECLClusters(ROEclusters)')
variables.addAlias('nROENeutralECLClustersSel', 'nROENeutralECLClusters(ROEclusters)')


applyCuts('Upsilon(4S):BhadBsigp', 'nAllROETracks==0')
applyCuts('Upsilon(4S):BslBsigp', 'nAllROETracks==0')

buildContinuumSuppression('B+:genericRank', '')
buildContinuumSuppression('B+:semileptonicRank', '')

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


toolsBP = ['EventMetaData', '^B+:genericRank']
toolsBP += ['DeltaEMbc', '^B+:genericRank']
toolsBP += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+:genericRank']
toolsBP += ['CustomFloats[d0_dmID]', '^B+:genericRank']
toolsBP += ['CustomFloats[isSignal:isExtendedSignal:looseMCMotherPDG:looseMCWrongDaughterN]', '^B+:genericRank']
toolsBP += ['CustomFloats[looseMCWrongDaughterBiB:looseMCWrongDaughterPDG]', '^B+:genericRank']
toolsBP += ['MCTruth', '^B+:genericRank']

toolsBPSL = ['EventMetaData', '^B+:semileptonicRank']
toolsBPSL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B+:semileptonicRank']
toolsBPSL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B+:semileptonicRank']
toolsBPSL += ['CustomFloats[d0_dmID]', '^B+:semileptonicRank']
toolsBPSL += ['MCTruth', '^B+:semileptonicRank']


tools4SBplusHad = ['EventMetaData', '^Upsilon(4S):BhadBsigp']
tools4SBplusHad += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S):BhadBsigp']
tools4SBplusHad += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S):BhadBsigp']
tools4SBplusHad += ['CustomFloats[R2EventLevel:R2:cosTBTO:cosTBz]', 'Upsilon(4S):BhadBsigp -> ^B+:genericRank B-:all']
tools4SBplusHad += ['CustomFloats[sigProb:rank]', 'Upsilon(4S):BhadBsigp -> ^B+:genericRank B-:all']
tools4SBplusHad += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                    'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += [
    'CustomFloats[missPz(ROEclusters,0):missPy(ROEclusters,0):missPx(ROEclusters,0):missP(R0Eclusters,0)]',
    '^Upsilon(4S):BhadBsigp ->B+:genericRank B-:all']
tools4SBplusHad += [
    'CustomFloats[missPTheta(ROEclusters,0):missE(ROEclusters,0):missingMass:missM2(ROEclusters,0)]',
    '^Upsilon(4S):BhadBsigp']
tools4SBplusHad += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                    'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_E:d1_E:d0_d0_E:d0_d1_E]', 'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_M:d1_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_p:d0_pCMS:d1_p:d1_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_p:d0_d1_pCMS]',
                    'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_mcPDG:d1_mcPDG:d0_d0_mcPDG:d0_d1_mcPDG]',
                    'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[dmID:d0_dmID:d0_d0_dmID]',
                    'Upsilon(4S):BhadBsigp ->  B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[isSignal:isExtendedSignal]',
                    'Upsilon(4S):BhadBsigp ->  ^B+:genericRank ^B-:all']
tools4SBplusHad += ['MCTruth', 'Upsilon(4S):BhadBsigp ->  ^B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d1_electronID:d1_muonID:d1_pionID:d1_kaonID]', 'Upsilon(4S):BhadBsigp -> B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_d0_d0_electronID:d0_d0_d0_muonID:d0_d0_d0_pionID:d0_d0_d0_kaonID]',
                    'Upsilon(4S):BhadBsigp -> B+:genericRank ^B-:all']
tools4SBplusHad += ['CustomFloats[d0_d0_d1_electronID:d0_d0_d1_muonID:d0_d0_d1_pionID:d0_d0_d1_kaonID]',
                    'Upsilon(4S):BhadBsigp -> B+:genericRank ^B-:all']

tools4SBplusSL = ['EventMetaData', '^Upsilon(4S):BslBsigp']
tools4SBplusSL += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S):BslBsigp']
tools4SBplusSL += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S):BslBsigp']
tools4SBplusSL += ['CustomFloats[cosThetaBetweenParticleAndTrueB:R2EventLevel:R2:cosTBTO:cosTBz]',
                   'Upsilon(4S):BslBsigp -> ^B+:semileptonicRank B-:all']
tools4SBplusSL += ['CustomFloats[sigProb:rank]', 'Upsilon(4S):BslBsigp -> ^B+:semileptonicRank B-:all']

tools4SBplusSL += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                   'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[missPz(ROEclusters,0):missPy(ROEclusters,0):missPx(ROEclusters,0):missP(R0Eclusters,0)]',
                   '^Upsilon(4S):BslBsigp ->B+:semileptonicRank B-:all']

tools4SBplusSL += [
    'CustomFloats[missPTheta(ROEclusters,0):missE(ROEclusters,0):missingMass:missM2(ROEclusters,0)]',
    '^Upsilon(4S):BslBsigp']
tools4SBplusSL += ['CustomFloats[d0_costheta:d0_d0_costheta:d0_costhetaCMS:d0_d0_costhetaCMS]',
                   'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']

tools4SBplusSL += ['CustomFloats[d0_E:d1_E:d0_d0_E:d0_d1_E]', 'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_M:d1_M:d0_d0_M:d0_d1_M]', 'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_p:d0_pCMS:d1_p:d1_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_p:d0_d1_pCMS]',
                   'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_mcPDG:d1_mcPDG:d0_d0_mcPDG:d0_d1_mcPDG]',
                   'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[dmID:d0_dmID:d0_d0_dmID]', 'Upsilon(4S):BslBsigp ->  B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d1_electronID:d1_muonID:d1_pionID:d1_kaonID]', 'Upsilon(4S):BslBsigp -> B+:genericRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_d0_d0_electronID:d0_d0_d0_muonID:d0_d0_d0_pionID:d0_d0_d0_kaonID]',
                   'Upsilon(4S):BslBsigp -> B+:genericRank ^B-:all']
tools4SBplusSL += ['CustomFloats[d0_d0_d1_electronID:d0_d0_d1_muonID:d0_d0_d1_pionID:d0_d0_d1_kaonID]',
                   'Upsilon(4S):BslBsigp -> B+:genericRank ^B-:all']
tools4SBplusSL += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                   'Upsilon(4S):BslBsigp ->  ^B+:semileptonicRank ^B-:all']
tools4SBplusSL += ['MCTruth', 'Upsilon(4S):BslBsigp ->  ^B+:semileptonicRank ^B-:all']


# Make lists and outptu to mdst and udst files.
# write out the flat ntuple
ntupleFile(outputRootFile)
ntupleTree('UpsBplusSL', 'Upsilon(4S):BslBsigp', tools4SBplusSL)
ntupleTree('UpsBplusHad', 'Upsilon(4S):BhadBsigp', tools4SBplusHad)
ntupleTree('BPHadTag', 'B+:generic', toolsBP)
ntupleTree('BPSLTag', 'B+:semileptonic', toolsBPSL)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
