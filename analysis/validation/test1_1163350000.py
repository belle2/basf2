#!/usr/bin/env python3
# -*- coding: utf-8 -*

"""
<header>
<input>../1163350000.dst.root</input>
<output>../1163350000.ntup.root</output>
<contact>Racha Cheaib racha.cheaib@desy.de</contact>
<interval>nightly</interval>
</header>
"""
#######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI and how to validate D(*)+tau- v and D(*) modes. The Btag is forced to a hadronic mode.
#  Release required: release-01-00-01 or later
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
gb2_setuprel = "release-01-00-01"


# from fei import backward_compatibility_layer
# backward_compatibility_layer.pid_renaming_oktober_2017()

# use_central_database('production', LogLevel.WARNING, 'fei_database')


# their names in the ntuple are human readable
from variables import variables


variables.addAlias('ROE_eextraSel', 'ROE_eextra(ROEclusters)')
variables.addAlias('ROE_neextraSel', 'ROE_neextra(ROEclusters)')
variables.addAlias('ROE_mcMissFlagsSel', 'ROE_mcMissFlags(ROEclusters)')
variables.addAlias('ROE_chargeSel', 'ROE_charge(ROEclusters)')
variables.addAlias('ROE_ESel', 'ROE_E(ROEclusters)')
variables.addAlias('ROE_PSel', 'ROE_P(ROEclusters)')
variables.addAlias('nAllROETracks', 'nROETracks(ROETracks)')
variables.addAlias('nROEECLClustersSel', 'nROEECLClusters(ROEclusters)')
variables.addAlias('abs_dM', 'abs(dM)')
variables.addAlias('d0_abs_dM', 'daughter(0,abs(dM))')
variables.addAlias('dMrank', 'extraInfo(abs_dM_rank)')
variables.addAlias('d0_dMrank', 'daughter(0,extraInfo(abs_dM_rank))')
variables.addAlias('nROENeutralECLClustersSel', 'nROENeutralECLClusters(ROEclusters)')
variables.addAlias('d1_goodGamma', 'daughter(1,goodGamma)')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
variables.addAlias('massDiff', 'massDifference(0)')
variables.addAlias('massDiffErr', 'massDifferenceError(0)')
variables.addAlias('massDiffrank', 'extraInfo(massDiff_rank)')
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
variables.addAlias('d0_p', 'daughter(0,p)')  # B Momenta
variables.addAlias('d0_pCMS', 'daughter(0,useCMSFrame(p))')  # B momenta
variables.addAlias('d1_p', 'daughter(1,p)')  # Btag Momenta
variables.addAlias('d1_pCMS', 'daughter(1,useCMSFrame(p))')  # Btag

variables.addAlias('d2_p', 'daughter(2,p)')  # B Momenta
variables.addAlias('d2_pCMS', 'daughter(2,useCMSFrame(p))')  # B momenta
variables.addAlias('d3_p', 'daughter(3,p)')  # Btag Momenta
variables.addAlias('d3_pCMS', 'daughter(3,useCMSFrame(p))')  # Btag


variables.addAlias('d0_d0_p', 'daughter(0,daughter(0,p))')  # Dstar
variables.addAlias('d0_d0_pCMS', 'daughter(0,daughter(0,useCMSFrame(p)))')  # Dstar


variables.addAlias('d0_d1_pCMS', 'daughter(0,daughter(1,useCMSFrame(p)))')  # tau
variables.addAlias('d0_d1_p', 'daughter(0,daughter(1,p))')  # tau
variables.addAlias('d0_d1_d0_p', 'daughter(0,daughter(1,daughter(0,p)))')  # muon or electron
variables.addAlias('d0_d1_d0_pCMS', 'daughter(0,daughter(1,daughter(0,useCMSFrame(p))))')  # muon or electron
variables.addAlias('d0_d0_d0_p', 'daughter(0,daughter(0,daughter(0,p)))')  # D+ or D0
variables.addAlias('d0_d0_d0_pCMS', 'daughter(0,daughter(0,daughter(0,useCMSFrame(p))))')
variables.addAlias('d0_d0_d1_pCMS', 'daughter(0,daughter(0,daughter(1,useCMSFrame(p))))')  # pion from D*
variables.addAlias('d0_d0_d1_p', 'daughter(0,daughter(0,daughter(1,p)))')  # pion from D*
variables.addAlias('d0_d0_d0_d0_pCMS', 'daughter(0,daughter(0,daughter(0,daughter(0,useCMSFrame(p)))))')
variables.addAlias('d0_d0_d0_d1_pCMS', 'daughter(0,daughter(0,daughter(daughter(1,useCMSFrame(p)))))')
variables.addAlias('d0_d0_d0_d2_pCMS', 'daughter(0,daughter(0,daughter(daughter(2,useCMSFrame(p)))))')
variables.addAlias('d0_d0_d0_d3_pCMS', 'daughter(0,daughter(0,daughter(daughter(3,useCMSFrame(p)))))')
variables.addAlias('d0_d0_d0_d0_p', 'daughter(0,daughter(0,daughter(0,daughter(0,p))))')
variables.addAlias('d0_d0_d0_d1_p', 'daughter(0,daughter(0,daughter(daughter(1,p))))')
variables.addAlias('d0_d0_d0_d2_p', 'daughter(0,daughter(0,daughter(daughter(2,p))))')
variables.addAlias('d0_d0_d0_d3_p', 'daughter(0,daughter(0,daughter(daughter(3,p))))')
# Mass
variables.addAlias('d0_M', 'daughter(0,InvM)')  # Bsig
variables.addAlias('d1_M', 'daughter(1,InvM)')  # Btag

variables.addAlias('d2_M', 'daughter(2,InvM)')  # Bsig
variables.addAlias('d3_M', 'daughter(3,InvM)')  # Btag

variables.addAlias('d0_mcpdg', 'daughter(0,mcPDG)')  # Bsig
variables.addAlias('d1_mcpdg', 'daughter(1,mcPDG)')  # Btag

variables.addAlias('d2_mcpdg', 'daughter(2,mcPDG)')  # Bsig
variables.addAlias('d3_mcpdg', 'daughter(3,mcPDG)')  # Btag
variables.addAlias('d0_d0_M', 'daughter(0,daughter(0,InvM))')  # Dstar
variables.addAlias('d0_d1_M', 'daughter(0,daughter(1,InvM))')  # tau
variables.addAlias('d0_d1_d0_M', 'daughter(0,daughter(1,daughter(0,InvM)))')  # muon or electron
variables.addAlias('d0_d0_d0_M', 'daughter(0,daughter(0,daughter(0,InvM)))')  # D+ or D0
variables.addAlias('d0_d0_d1_M', 'daughter(0,daughter(0,daughter(1,InvM)))')  # pion from D*
variables.addAlias('d0_d0_d0_d0_M', 'daughter(0,daughter(0,daughter(0,daughter(0,InvM))))')
variables.addAlias('d0_d0_d0_d1_M', 'daughter(0,daughter(0,daughter(daughter(1,InvM))))')
variables.addAlias('d0_d0_d0_d2_M', 'daughter(0,daughter(0,daughter(daughter(2,InvM))))')
variables.addAlias('d0_d0_d0_d3_M', 'daughter(0,daughter(0,daughter(daughter(3,InvM))))')
variables.addAlias('d_ID', 'daughter(0,daughter(0,daughter(0,extraInfo(decayModeID))))')
variables.addAlias('dstarID', 'daughter(0,daughter(0,extraInfo(decayModeID)))')
variables.addAlias('d0_d0_costheta', 'daughter(0,daughter(0,cosTheta))')
variables.addAlias('d0_costheta', 'daughter(0,cosTheta)')
variables.addAlias('d0_costhetaCMS', 'daughter(0,useCMSFrame(cosTheta))')
variables.addAlias('d0_d0_costhetaCMS', 'daughter(0,daughter(0,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_costhetaCMS', 'daughter(0,daughter(1,useCMSFrame(cosTheta)))')
variables.addAlias('d0_d1_d0_costhetaCMS', 'daughter(0,daughter(1,daughter(0,useCMSFrame(cosTheta))))')
variables.addAlias('d0_d1_d1_costhetaCMS', 'daughter(0,daughter(1,daughter(1,useCMSFrame(cosTheta))))')
variables.addAlias('d0_E', 'daughter(0,E)')
variables.addAlias('d1_E', 'daughter(1,E)')
variables.addAlias('d0_eCMS', 'daughter(0,useCMSFrame(E))')
variables.addAlias('d1_eCMS', 'daughter(1,useCMSFrame(E))')

variables.addAlias('d2_eCMS', 'daughter(2,useCMSFrame(E))')
variables.addAlias('d3_eCMS', 'daughter(3,useCMSFrame(E))')


variables.addAlias('d0_E1E9', 'daughter(0,clusterE1E9)')
variables.addAlias('d1_E1E9', 'daughter(1,clusterE1E9)')

variables.addAlias('d0_clusErrTiming', 'daughter(0,clusterErrorTiming)')
variables.addAlias('d1_clusErrTiming', 'daughter(1,clusterErrorTiming)')
variables.addAlias('d0_phi', 'daughter(0,clusterPhi)')
variables.addAlias('d1_phi', 'daughter(1,clusterPhi)')

variables.addAlias('d0_clusTrkMatch', 'daughter(0,clusterTrackMatch)')
variables.addAlias('d1_clusTrkMatch', 'daughter(1,clusterTrackMatch)')
variables.addAlias('d0_clusReg', 'daughter(0,clusterReg)')
variables.addAlias('d1_clusReg', 'daughter(1,clusterReg)')
variables.addAlias('d0_clusTiming', 'daughter(0,clusterTiming)')
variables.addAlias('d1_clusTiming', 'daughter(1,clusterTiming)')
variables.addAlias('d0_ZerMVA', 'daughter(0,clusterZernikeMVA)')
variables.addAlias('d1_ZerMVA', 'daughter(1,clusterZenikeMVA)')
variables.addAlias('d0_goodBelleGamma', 'daughter(0,goodBelleGamma)')
variables.addAlias('d1_goodBelleGamma', 'daughter(1,goodBelleGamma)')


outputRootFile = '../1163350000.ntup.root'


fileList = ['../1163350000.dst.root']

inputMdstList('default', fileList)  # '/ghi/fs01/belle2/bdata/group/physics/semitauonic/
# release-00-09-01/DB00000276/MC9/1193300007/BGx0/sub00/*.root')


stdPi('85eff')
stdK('85eff')
stdE('90eff')
stdMu('90eff')
# Calling standard particle lists
fillParticleList('pi+:95eff', 'pt>0.05')
fillParticleList('K+:85eff', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1 and 0.3<useCMSFrame(p)<2.8')
fillParticleList('e+:90eff', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1 and 0.3 <useCMSFrame(p)<1.8')
fillParticleList('mu+:90eff', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1 and 0.3<useCMSFrame(p)<1.8')

loadStdCharged()
stdPi0s('eff40')
cutAndCopyList('pi0:sig', 'pi0:eff40', '0.1<InvM<0.16')
cutAndCopyList('pi+:sig', 'pi+:95eff', 'dr < 0.5 and -2 < dz < 2 and pt > 0.1')
cutAndCopyList('pi0:lowp', 'pi0:eff40', '0.06<useCMSFrame(p)<0.25')
cutAndCopyList('pi+:lowp', 'pi+:85eff', '0.06<useCMSFrame(p)<0.25')

# tag side reconstruction
reconstructDecay('D-:tag ->K-:85eff pi+:sig pi-:sig', '1.8<M<1.9')
reconstructDecay('B0:tag ->D-:tag pi+:sig', 'Mbc>5.27 and abs(deltaE)<0.2')

# pi0
# KSs
loadStdKS()
vertexKFit('K_S0:all', 0.001)
cutAndCopyList('K_S0:sig', 'K_S0:all', 'distance>0.5 and significanceOfDistance>2')
# Photons
stdPhotons('tight')
stdPhotons('pi0highE')
applyCuts('gamma:pi0highE', 'E>0.150 and clusterE9E25>0.8 and abs(clusterTiming)<clusterErrorTiming')
applyCuts('gamma:tight', 'E>0.04 and abs(clusterTiming)<clusterErrorTiming')

# D+ :


reconstructDecay('D+:sig1 -> K-:85eff pi+:sig pi+:sig', '1.7<InvM<2.1', 1)
reconstructDecay('D+:sig2 -> K-:85eff pi+:sig pi+:sig pi0:sig', '1.7<InvM<2.1', 2)
reconstructDecay('D+:sig3 -> K_S0:sig pi+:sig', '1.7<InvM<2.1', 3)
reconstructDecay('D+:sig4 -> K_S0:sig pi+:sig pi-:85eff pi+:sig', '1.7<InvM<2.1', 4)
reconstructDecay('D+:sig5 -> K_S0:sig pi+:sig pi0:sig', '1.7<InvM<2.1', 5)
reconstructDecay('D+:sig6 -> K_S0:sig K+:85eff', '1.7<InvM<2.1', 6)

copyLists('D+:sigD', ['D+:sig1', 'D+:sig2', 'D+:sig3', 'D+:sig4', 'D+:sig5', 'D+:sig6'])
rankByLowest('D+:sigD', 'abs_dM')


reconstructDecay('D0:sig1 -> K-:85eff pi+:sig', '1.7<InvM<2.1', 7)
reconstructDecay('D0:sig2 -> K_S0:sig pi0:sig', '1.7<InvM<2.1', 8)
reconstructDecay('D0:sig3 -> K-:85eff pi+:sig pi0:sig', '1.7<InvM<2.1', 9)
reconstructDecay('D0:sig4 -> K-:85eff pi+:sig pi-:sig pi+:sig', '1.7<InvM<2.1', 10)
reconstructDecay('D0:sig5 -> K_S0:sig pi+:sig pi-:sig', '1.7<InvM<2.1', 11)
reconstructDecay('D0:sig6 -> K_S0:sig pi+:sig pi-:sig pi0:sig', '1.7<InvM<2.1', 12)
copyLists('D0:sigD', ['D0:sig1', 'D0:sig2', 'D0:sig3', 'D0:sig4', 'D0:sig5', 'D0:sig6'])

rankByLowest('D0:sigD', 'abs_dM')
reconstructDecay('D*+:sigDstar1 -> D0:sigD pi+:lowp', '0.139<massDifference(0)<0.16', 1)
reconstructDecay('D*+:sigDstar2 ->D+:sigD pi0:lowp', '0.139<massDifference(0)<0.16', 2)

copyLists('D*+:sigDstar', ['D*+:sigDstar1', 'D*+:sigDstar2'])


rankByLowest('D*-:sigDstar', 'massDifference(0)')

rankByLowest('D*-:sigDstar', 'abs_dM')

# tau
reconstructDecay('tau-:ch1 -> e-:90eff', '')
reconstructDecay('tau-:ch2 -> mu-:90eff', '')
copyLists('tau-:mytau', ['tau-:ch1', 'tau-:ch2'])

# B0:sig

reconstructDecay('B0:sig -> D*-:sigDstar tau+:mytau', 'Mbc>0')
# Upsilon(4S)

reconstructDecay('Upsilon(4S) -> B0:sig  anti-B0:tag', '')


# Rest of EVent
buildRestOfEvent('Upsilon(4S)')

ROETracks = ('ROETracks', '', '')
ROEclusters = ('ROEclusters', '', 'abs(clusterTiming)<clusterErrorTiming and E>0.05')


appendROEMasks('Upsilon(4S)', [ROEclusters, ROETracks])


Bsig_B0Dstar_tool = ['MCTruth', '^B0:sig -> ^D*+:sigDstar tau-:mytau ']
Bsig_B0Dstar_tool += ['MCHierarchy', '^B0:sig -> ^D*+:sigDstar ^tau-:mytau ']
Bsig_B0Dstar_tool += ['MCKinematics', '^B0:sig -> ^D*+:sigDstar ^tau-:mytau ']
Bsig_B0Dstar_tool += ['DeltaEMbc', '^B0:sig -> D*+:sigDstar tau-:mytau']
Bsig_B0Dstar_tool += ['Kinematics', '^B0:sig -> ^D*+:sigDstar ^tau-:mytau ']
Bsig_B0Dstar_tool += ['InvMass', '^B0:sig -> ^D*+:sigDstar ^tau-:mytau ']
Bsig_B0Dstar_tool += ['EventMetaData', '^B0:sig']
Bsig_B0Dstar_tool += ['CustomFloats[decayAngle(0):decayAngle(1):isSignal]', '^B0:sig ->^D*+:sigDstar tau-:mytau']
Bsig_B0Dstar_tool += [
    'CustomFloats[cosThetaBetweenParticleAndTrueB:pRecoil:daughterAngleInBetween(0,1)]',
    '^B0:sig ->D*+:sigDstar tau-:mytau']

Y4S_B0Dstar_tool = ['MCTruth', '^Upsilon(4S) -> [^anti-B0:sig -> ^D*+:sigDstar ^tau-:mytau] ^B0:tag']
Y4S_B0Dstar_tool += ['ExtraEnergy', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['MCKinematics', '^Upsilon(4S) -> [^anti-B0:sig -> ^D*+:sigDstar tau-:mytau] ^B0:tag']
Y4S_B0Dstar_tool += ['Kinematics', '^Upsilon(4S) -> [^anti-B0:sig -> ^D*+:sigDstar  tau-:mytau ] ^B0:tag']
Y4S_B0Dstar_tool += ['InvMass', 'Upsilon(4S) -> [anti-B0:sig -> ^D*+:sigDstar  tau-:mytau] B0:tag']
Y4S_B0Dstar_tool += ['ExtraEnergy', 'Upsilon(4S)']
Y4S_B0Dstar_tool += ['RecoilKinematics', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['ROEMultiplicities', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['EventMetaData', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['CustomFloats[decayAngle(0):decayAngle(1):isSignal]',
                     'Upsilon(4S) -> [^anti-B0:sig ->^D*+:sigDstar ^tau-:mytau] ^B0:tag']
Y4S_B0Dstar_tool += ['CustomFloats[m2Recoil:decayAngle(0,1)]', 'Upsilon(4S) -> ^anti-B0:sig B0:tag']
Y4S_B0Dstar_tool += [
    'CustomFloats[cosThetaBetweenParticleAndTrueB:pRecoil:daughterAngleInBetween(0,1)]',
    'Upsilon(4S) -> ^anti-B0:sig B0:tag']
Y4S_B0Dstar_tool += ['CustomFloats[daughter(0,dr):daughter(0,dz):daughter(0,pt):daughter(0,p)]',
                     'Upsilon(4S) -> [anti-B0:sig -> D*+:sigDstar ^tau-:mytau ] B0:tag']
Y4S_B0Dstar_tool += ['CustomFloats[abs(clusterTiming):clusterErrorTiming]', '^Upsilon(4S)']
Y4S_B0Dstar_tool += [
    'CustomFloats[p:isSignal:useCMSFrame(p):useCMSFrame(daughter(0,p)):useCMSFrame(daughter(1,p)):daughter(1,p)]',
    'Upsilon(4S) -> [anti-B0:sig -> ^D*+:sigDstar  tau-:mytau] B0:tag']
Y4S_B0Dstar_tool += ['CustomFloats[isSignal:Mbc:deltaE]',
                     'Upsilon(4S) -> [^anti-B0:sig -> D*+:sigDstar tau-:mytau] ^B0:tag']
Y4S_B0Dstar_tool += ['CustomFloats[d0_M:d1_M:d0_d0_M:d0_d1_M:d0_d1_d0_M:d0_d0_d0_M]', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['CustomFloats[d0_d0_d1_M]', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['CustomFloats[d_ID:dstarID]', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['CustomFloats[d0_p:d1_p:d0_d0_p:d0_d1_M:d0_d1_d0_p:d0_d0_d0_p:d0_d0_d1_p]', '^Upsilon(4S)']
Y4S_B0Dstar_tool += ['CustomFloats[d0_pCMS:d1_pCMS:d0_d0_pCMS:d0_d1_M:d0_d1_d0_pCMS:d0_d0_d0_pCMS:d0_d0_d1_pCMS]', '^Upsilon(4S)']

Y4S_B0Dstar_tool += ['CustomFloats[abs_dM:dMrank:dM:massDifference(0)]',
                     'Upsilon(4S) -> [ anti-B0:sig -> ^D*+:sigDstar tau-:mytau] B0:tag ']
Y4S_B0Dstar_tool += [
    'CustomFloats[dr:dz:pt:p:useCMSFrame(p):d0_p:d1_p:d0_pCMS:d1_pCMS:E:useCMSFrame(E):d0_eCMS:d1_eCMS:dmID]',
    'Upsilon(4S) -> [ anti-B0:sig -> ^D*+:sigDstar tau-:mytau] B0:tag ']


Y4S_B0Dstar_tool += ['CustomFloats[dmID:useCMSFrame(p):d0_M:d0_pCMS:useCMSFrame(p):E:InvM]',
                     'Upsilon(4S) -> [ anti-B0:sig -> ^D*+:sigDstar ^tau-:mytau] B0:tag ']
Y4S_B0Dstar_tool += ['CustomFloats[R2EventLevel:cosTBTO:pRecoil]', '^Upsilon(4S) -> anti-B0:sig B0:tag']
# D*+
Dstar_sig_tool = ['MCTruth', '^D*+:myD*']
Dstar_sig_tool += ['MCHierarchy', '^D*+:myD*']
Dstar_sig_tool += ['InvMass', '^D*+:myD*']
Dstar_sig_tool += ['MCKinematics', '^D*+:myD*']
Dstar_sig_tool += ['Kinematics', '^D*+:myD*']
Dstar_sig_tool += ['Track', 'D*+:myD*']
Dstar_sig_tool += ['EventMetaData', '^D*+:myD*']
Dstar_sig_tool += ['CustomFloats[abs_dM:dM:massDifference(0)]', '^D*+:myD*']
Dstar_sig_tool += ['CustomFloats[dr:dz:pt:p:useCMSFrame(p):d0_p:d1_p:d0_pCMS]', '^D*+:myD*']
Dstar_sig_tool += ['CustomFloats[d1_pCMS:E:useCMSFrame(E):d0_eCMS:d1_eCMS:dmID]', '^D*+:myD* ']
# D*0
Dstar0_sig_tool = ['MCTruth', '^D*0:myD*']
Dstar0_sig_tool += ['MCHierarchy', '^D*0:myD*']
Dstar0_sig_tool += ['InvMass', '^D*0:myD*']
Dstar0_sig_tool += ['MCKinematics', '^D*0:myD*']
Dstar0_sig_tool += ['Kinematics', '^D*0:myD*']
Dstar0_sig_tool += ['Track', 'D*0:myD*']
Dstar0_sig_tool += ['EventMetaData', '^D*0:myD*']
Dstar0_sig_tool += ['CustomFloats[abs_dM:dM:massDifference(0)]', '^D*0:myD*']
Dstar0_sig_tool += ['CustomFloats[dr:dz:pt:p:useCMSFrame(p):d0_p:d1_p:d0_pCMS]', '^D*0:myD*']
Dstar0_sig_tool += ['CustomFloats[d1_pCMS:E:useCMSFrame(E):d0_eCMS:d1_eCMS:dmID]', '^D*0:myD* ']


# D+
Dp_sig_tool = ['MCTruth', '^D+:sigD']
Dp_sig_tool += ['MCHierarchy', '^D+:sigD']
Dp_sig_tool += ['InvMass', '^D+:sigD']
Dp_sig_tool += ['Track', '^D+:sigD']
Dp_sig_tool += ['MCKinematics', '^D+:sigD']
Dp_sig_tool += ['Kinematics', '^D+:sigD ']
Dp_sig_tool += ['EventMetaData', '^D+:sigD']
Dp_sig_tool += ['CustomFloats[abs_dM:dMrank:dM:massDifference(0)]', '^D+:sigD']
Dp_sig_tool += ['CustomFloats[dr:dz:pt:p:useCMSFrame(p):d0_p:d1_p:d0_pCMS]', '^D+:sigD']
Dp_sig_tool += ['CustomFloats[d1_pCMS:E:useCMSFrame(E):d0_eCMS:d1_eCMS:dmID]', '^D+:sigD ']


# D0
D0_sig_tool = ['MCTruth', '^D0:sigD']
D0_sig_tool += ['MCHierarchy', '^D0:sigD']
D0_sig_tool += ['InvMass', '^D0:sigD']
D0_sig_tool += ['Track', '^D0:sigD']
D0_sig_tool += ['MCKinematics', '^D0:sigD']
D0_sig_tool += ['Kinematics', '^D0:sigD ']
D0_sig_tool += ['EventMetaData', '^D0:sigD']
D0_sig_tool += ['CustomFloats[abs_dM:dmID:dr:dz:pt:p:useCMSFrame(p)]', '^D0:sigD']
D0_sig_tool += ['CustomFloats[d0_M:d1_M:d2_M:d0_mcpdg:d1_mcpdg:d2_mcpdg]', '^D0:sigD']
D0_sig_tool += ['CustomFloats[d0_pCMS:d1_pCMS:d2_pCMS:d0_eCMS:d1_eCMS:d2_eCMS]', '^D0:sigD']

B0_generic_tool = ['MCTruth', '^B0:tag']
B0_generic_tool += ['DeltaEMbc', '^B0:tag']
B0_generic_tool += ['MCHierarchy', '^B0:tag']
B0_generic_tool += ['MCKinematics', '^B0:tag']
B0_generic_tool += ['MCVertex', '^B0:tag']
B0_generic_tool += ['Kinematics', '^B0:tag']
B0_generic_tool += ['EventMetaData', '^B0:tag']
B0_generic_tool += ['CustomFloats[useCMSFrame(p):decayAngle(0):isSignal]', '^B0:tag']

ntupleFile('../1163350000.ntup.root')
ntupleTree('Dpall', 'D+:sigD', Dp_sig_tool)
ntupleTree('DSTsig', 'D*+:sigDstar', Dstar_sig_tool)
ntupleTree('D0all', 'D0:sigD', D0_sig_tool)
ntupleTree('Y4S', 'Upsilon(4S)', Y4S_B0Dstar_tool)


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
