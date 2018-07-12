#!/usr/bin/env python3
# -*- coding: utf-8 -*
"""
<input>../1163300003.dst.root</input>
<output>../1163300003.ntup.root</output>
<contact> Racha Cheaib rcheaib@olemiss.edu, Mario Merola mario.merola@na.infn.it</contact>
<interval>nightly</interval>
</header>
"""
#######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI and how to validate D(*)+tau- v and D(*)
#  Release required: build-2017-09-08 or later
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

# tau daughter PID
variables.addAlias('d1_d0_electronID', 'daughter(1,daughter(0,electronID))')
variables.addAlias('d1_d0_muonID', 'daughter(1,daughter(0,muonID))')
variables.addAlias('d1_d0_pionID', 'daughter(1,daughter(0,pionID))')
variables.addAlias('d1_d0_kaonID', 'daughter(1,daughter(0,kaonID))')


# D* daughter pion PID
variables.addAlias('d0_d1_electronID', 'daughter(0,daughter(1,electronID))')
variables.addAlias('d0_d1_muonID', 'daughter(0,daughter(1,muonID))')
variables.addAlias('d0_d1_pionID', 'daughter(0,daughter(1,pionID))')
variables.addAlias('d0_d1_kaonID', 'daughter(0,daughter(1,kaonID))')


# D0 daughters : K- pi + PID
variables.addAlias('d0_d0_d0_electronID', 'daughter(0,daughter(0,daughter(0,electronID)))')
variables.addAlias('d0_d0_d0_muonID', 'daughter(0,daughter(0,daughter(0,muonID)))')
variables.addAlias('d0_d0_d0_pionID', 'daughter(0,daughter(0,daughter(0,pionID)))')
variables.addAlias('d0_d0_d0_kaonID', 'daughter(0,daughter(0,daughter(0,kaonID)))')

variables.addAlias('d0_d0_d1_electronID', 'daughter(0,daughter(0,daughter(1,electronID)))')
variables.addAlias('d0_d0_d1_muonID', 'daughter(0,daughter(0,daughter(1,muonID)))')
variables.addAlias('d0_d0_d1_pionID', 'daughter(0,daughter(0,daughter(1,pionID)))')
variables.addAlias('d0_d0_d1_kaonID', 'daughter(0,daughter(0,daughter(1,kaonID)))')

# Momenta
variables.addAlias('d0_p', 'daughter(0,p)')
variables.addAlias('d0_pCMS', 'daughter(0,useCMSFrame(p))')
variables.addAlias('d1_p', 'daughter(1,p)')
variables.addAlias('d1_pCMS', 'daughter(1,useCMSFrame(p))')
variables.addAlias('d0_d0_p', 'daughter(0,daughter(0,p))')
variables.addAlias('d0_d1_p', 'daughter(0,daughter(1,p))')
variables.addAlias('d0_d0_pCMS', 'daughter(0,daughter(0,useCMSFrame(p)))')
variables.addAlias('d0_d1_pCMS', 'daughter(0,daughter(1,useCMSFrame(p)))')
variables.addAlias('d1_d0_p', 'daughter(1,daughter(0,p))')
variables.addAlias('d1_d0_pCMS', 'daughter(1,daughter(0,useCMSFrame(p)))')

variables.addAlias('d0_d0_M', 'daughter(0,daughter(0,InvM))')
variables.addAlias('d0_d1_M', 'daughter(0,daughter(1,InvM))')
variables.addAlias('d1_d0_M', 'daughter(1,daughter(0,InvM))')
variables.addAlias('d0_M', 'daughter(0,InvM)')
variables.addAlias('d1_M', 'daughter(1,InvM)')

variables.addAlias('d0_d0_costheta', 'daughter(0,daughter(0,cosTheta))')
variables.addAlias('d0_costheta', 'daughter(0,cosTheta)')
variables.addAlias('d0_costhetaCMS', 'daughter(0,useCMSFrame(cosTheta))')
variables.addAlias('d0_d0_costhetaCMS', 'daughter(0,daughter(0,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_costhetaCMS', 'daughter(0,daughter(1,useCMSFrame(cosTheta)))')
variables.addAlias('d1_costheta', 'daughter(1,cosTheta)')

variables.addAlias('d0_d0_E', 'daughter(0,daughter(0,E))')
variables.addAlias('d0_d1_E', 'daughter(0,daughter(1,E))')
variables.addAlias('d0_E', 'daughter(0,E)')
variables.addAlias('d1_E', 'daughter(1,E)')
variables.addAlias('d1_d0_E', 'daughter(1,daughter(0,E))')


variables.addAlias('d0_d0_eCMS', 'daughter(0,daughter(0,useCMSFrame(E)))')
variables.addAlias('d0_d1_eCMS', 'daughter(0,daughter(1,useCMSFrame(E)))')
variables.addAlias('d0_eCMS', 'daughter(0,useCMSFrame(E))')
variables.addAlias('d1_eCMS', 'daughter(1,useCMSFrame(E))')
variables.addAlias('d1_d0_eCMS', 'daughter(1,daughter(0,useCMSFrame(E)))')

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
variables.addAlias('d0_d0_mcPDG', 'daughter(0,daughter(0,mcPDG))')
variables.addAlias('d0_d1_mcPDG', 'daughter(0,daughter(1,mcPDG))')


outputRootFile = '../1163300003.ntup.root'


path = create_path()

import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)


fileList = ['../1163300003.dst.root']
inputMdstList('default', fileList)

# execute path and return back to the analysis_main
# the skim condition is TRUE for all events
skimALL = register_module('VariableToReturnValue')
skimALL.param("variable", 'True')
skimALL.if_value('==1', path, AfterConditionPath.CONTINUE)
analysis_main.add_module(skimALL)

# now the FEI reconstruction is done

applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

applyCuts('B0:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.001')

# rank Btag canidates according to their SignalProbability
# 'sigProb' is alias for 'extraInfo(SignalProbability)'
rankByHighest('B0:generic', 'sigProb')
rankByHighest('B0:semileptonic', 'sigProb')
looseMCTruth('B0:generic')
# ranking B mesons
cutAndCopyList('B0:genericRank', 'B0:generic', 'rank==1')
cutAndCopyList('B0:semileptonicRank', 'B0:semileptonic', 'rank==1')


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

copyLists('D0:all', ['D0:kpi'])

# D+:

reconstructDecay('D+:kpipi -> K-:95eff pi+:95eff pi+:95eff', '1.8<InvM<1.9')

copyLists('D+:all', ['D+:kpipi'])

# D*+:

reconstructDecay('D*+:ch1 -> D0:all pi+:lowP', '0<Q<0.04')

copyLists('D*+:all', ['D*+:ch1'])

# tau
reconstructDecay('tau-:ch1 -> e-:all', '')
reconstructDecay('tau-:ch2 -> mu-:all', '')
copyLists('tau-:all', ['tau-:ch1', 'tau-:ch2'])

# B0:sig

reconstructDecay('B0:all -> D*-:all tau+:all', 'Mbc>0')

# Upsilon(4S)

reconstructDecay('Upsilon(4S):BhadBsig0 -> B0:genericRank anti-B0:all', '')
reconstructDecay('Upsilon(4S):BslBsig0 -> B0:semileptonicRank anti-B0:all', '')


# Rest of EVent
buildRestOfEvent('Upsilon(4S):BhadBsig0')
buildRestOfEvent('Upsilon(4S):BslBsig0')

ROETracks = ('ROETracks', '', '')
ROEclusters = ('ROEclusters', '', 'abs(clusterTiming)<clusterErrorTiming and E>0.05')


appendROEMasks('Upsilon(4S):BhadBsig0', [ROEclusters, ROETracks])
appendROEMasks('Upsilon(4S):BslBsig0', [ROEclusters, ROETracks])


variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_mcMissFlagsSel', 'ROE_mcMissFlags(ROEclusters)')
variables.addAlias('ROE_chargeSel', 'ROE_charge(ROEclusters)')
variables.addAlias('nAllROETracks', 'nROETracks(ROETracks)')
variables.addAlias('nROEECLClustersSel', 'nROEECLClusters(ROEclusters)')
variables.addAlias('nROENeutralECLClustersSel', 'nROENeutralECLClusters(ROEclusters)')


applyCuts('Upsilon(4S):BhadBsig0', 'nAllROETracks==0')
applyCuts('Upsilon(4S):BslBsig0', 'nAllROETracks==0')

# Continuum suppression:

buildContinuumSuppression('B0:genericRank', '')
buildContinuumSuppression('B0:semileptonicRank', '')
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
matchMCTruth('B0:genericRank')
matchMCTruth('B0:semileptonicRank')
matchMCTruth('B0:all')


toolsB0 = ['EventMetaData', '^B0:genericRank']
toolsB0 += ['DeltaEMbc', '^B0:genericRank']
toolsB0 += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B0:genericRank']
toolsB0 += ['CustomFloats[d0_dmID:d1_dmID:d0_d0_dmID]', '^B0:genericRank']
toolsB0 += ['CustomFloats[isSignal:isExtendedSignal:looseMCMotherPDG:looseMCWrongDaughterN]', '^B0:genericRank']
toolsB0 += ['CustomFloats[looseMCWrongDaughterBiB:looseMCWrongDaughterPDG]', '^B0:genericRank']
toolsB0 += ['MCTruth', '^B0:genericRank']


toolsB0SL = ['EventMetaData', '^B0:semileptonicRank']
toolsB0SL += ['CustomFloats[cosThetaBetweenParticleAndTrueB]', '^B0:semileptonicRank']
toolsB0SL += ['CustomFloats[sigProb:rank:dmID:uniqueSignal]', '^B0:semileptonicRank']
toolsB0SL += ['CustomFloats[d0_dmID]', '^B0:semileptonicRank']
toolsB0SL += ['CustomFloats[isSignalAcceptMissingNeutrino]', '^B0:semileptonicRank']
toolsB0SL += ['MCTruth', '^B0:semileptonicRank']


tools4SB0Had = ['EventMetaData', '^Upsilon(4S):BhadBsig0']
tools4SB0Had += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S):BhadBsig0']
tools4SB0Had += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S):BhadBsig0']
tools4SB0Had += ['CustomFloats[R2EventLevel:R2:cosTBTO:cosTBz]', 'Upsilon(4S):BhadBsig0 -> ^B0:genericRank anti-B0:all']
tools4SB0Had += ['CustomFloats[sigProb:rank]', 'Upsilon(4S):BhadBsig0 -> ^B0:genericRank anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_E:d0_eCMS:d1_E:d1_eCMS:d1_d0_E:d1_d0_eCMS:d0_d0_E:d0_d0_eCMS:d0_d1_E:d0_d1_eCMS]',
                 'Upsilon(4S):BhadBsig0 ->  B0:genericRank ^anti-B0:all']

tools4SB0Had += ['CustomFloats[missPx(ROEclusters,0):missPy(ROEclusters,0)]', '^Upsilon(4S):BhadBsig0']
tools4SB0Had += ['CustomFloats[missPz(ROEclusters,0):missP(ROEclusters,0)]', '^Upsilon(4S):BhadBsig0']
tools4SB0Had += ['CustomFloats[missPTheta(ROEclusters,0):missE(ROEclusters,0):m2RecoilSignalSide:missM2(ROEclusters,0)]',
                 '^Upsilon(4S):BhadBsig0']
tools4SB0Had += ['CustomFloats[d0_M:d1_M:d0_d0_M:d0_d1_M:d1_d0_M]', 'Upsilon(4S):BhadBsig0 ->  B0:genericRank ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_p:d0_pCMS:d1_p:d1_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_p:d0_d1_pCMS:d1_d0_pCMS:d1_d0_p]',
                 'Upsilon(4S):BhadBsig0 ->  B0:genericRank ^anti-B0:all']
tools4SB0Had += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID]',
                 'Upsilon(4S):BhadBsig0 ->  ^B0:genericRank ^anti-B0:all']
tools4SB0Had += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                 'Upsilon(4S):BhadBsig0 ->  ^B0:genericRank ^anti-B0:all']
tools4SB0Had += ['MCTruth', 'Upsilon(4S):BhadBsig0 ->  ^B0:genericRank ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d1_d0_electronID:d1_d0_muonID:d1_d0_pionID:d1_d0_kaonID]',
                 'Upsilon(4S):BhadBsig0 -> B0:genericRank ^anti-B0:all']
tools4SB0Had += ['CustomFloats[d0_d1_electronID:d0_d1_muonID:d0_d1_pionID:d0_d1_kaonID]',
                 'Upsilon(4S):BhadBsig0 -> B0:genericRank ^anti-B0:all']

tools4SB0Had += ['CustomFloats[d0_d0_d0_kaonID:d0_d0_d0_pionID:d0_d0_d0_muonID:d0_d0_d0_electronID]',
                 'Upsilon(4S):BhadBsig0 -> B0:genericRank ^anti-B0:all']

tools4SB0Had += ['CustomFloats[d0_d0_d1_kaonID:d0_d0_d1_pionID:d0_d0_d1_muonID:d0_d0_d1_electronID]',
                 'Upsilon(4S) -> B0:genericRank ^anti-B0:all']


tools4SB0SL = ['EventMetaData', '^Upsilon(4S):BslBsig0']
tools4SB0SL += ['CustomFloats[ROE_eextraSel]', '^Upsilon(4S):BslBsig0']
tools4SB0SL += ['CustomFloats[d0_Mbc:d0_deltaE:d1_Mbc:d1_deltaE]', '^Upsilon(4S):BslBsig0']
tools4SB0SL += ['CustomFloats[cosThetaBetweenParticleAndTrueB:R2EventLevel:R2:cosTBTO:cosTBz]',
                'Upsilon(4S):BslBsig0 -> ^B0:semileptonicRank anti-B0:all']
tools4SB0SL += ['CustomFloats[sigProb:rank]', 'Upsilon(4S):BslBsig0 -> ^B0:semileptonicRank anti-B0:all']


tools4SB0SL += ['CustomFloats[missPz(ROEclusters,0):missP(ROEclusters,0)]', '^Upsilon(4S):BslBsig0']
tools4SB0SL += ['CustomFloats[missPTheta(ROEclusters,0):missE(ROEclusters,0):m2RecoilSignalSide:missM2(ROEclusters,0)]',
                '^Upsilon(4S):BslBsig0']
tools4SB0SL += ['CustomFloats[d0_E:d0_eCMS:d1_E:d1_eCMS:d1_d0_E:d1_d0_eCMS:d0_d0_E:d0_d0_eCMS:d0_d1_E:d0_d1_eCMS]',
                'Upsilon(4S):BslBsig0 ->  B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_M:d1_M:d0_d0_M:d0_d1_M:d1_d0_M]', 'Upsilon(4S):BslBsig0 -> B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_p:d0_pCMS:d1_p:d1_pCMS:d0_d0_p:d0_d0_pCMS:d0_d1_p:d0_d1_pCMS:d1_d0_pCMS:d1_d0_p]',
                'Upsilon(4S):BslBsig0 -> B0:semileptinicRanki ^anti-B0:all']
tools4SB0SL += ['CustomFloats[dmID:d0_dmID:d1_dmID:d0_d0_dmID]', 'Upsilon(4S):BslBsig0 ->  ^B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[isSignal:isExtendedSignal:isSignalAcceptMissingNeutrino]',
                'Upsilon(4S):BslBsig0 ->  ^B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['MCTruth', 'Upsilon(4S):BslBsig0 ->  ^B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d1_d0_electronID:d1_d0_muonID:d1_d0_pionID:d1_d0_kaonID]',
                'Upsilon(4S):BslBsig0 -> B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_d1_electronID:d0_d1_muonID:d0_d1_pionID:d0_d1_kaonID]',
                'Upsilon(4S):BslBsig0 -> B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_d0_d0_electronID:d0_d0_d0_muonID:d0_d0_d0_pionID:d0_d0_d0_kaonID]',
                'Upsilon(4S):BslBsig0 -> B0:semileptonicRank ^anti-B0:all']
tools4SB0SL += ['CustomFloats[d0_d0_d1_electronID:d0_d0_d1_muonID:d0_d0_d1_pionID:d0_d0_d1_kaonID]',
                'Upsilon(4S):BslBsig0 -> B0:semileptonicRank ^anti-B0:all']


ntupleFile(outputRootFile)
ntupleTree('UpsB0Had', 'Upsilon(4S):BhadBsig0', tools4SB0Had)
ntupleTree('UpsB0sl', 'Upsilon(4S):BslBsig0', tools4SB0SL)
ntupleTree('B0HadTag', 'B0:genericRank', toolsB0)
ntupleTree('B0SLTag', 'B0:semileptonicRank', toolsB0SL)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
