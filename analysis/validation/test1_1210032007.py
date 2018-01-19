#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1210032007.dst.root</input>
  <output>../1210032007.ntup.root</output>
  <contact>Komarov Ilya; ilya.komarov@desy.de </contact>
    <description>Reconstruction of longitudinaly-polarised B->K*(->K+pi0)rho(->pi+pi-) events. Aplied cuts:
    <ul>
        <li>&#x394;E &#x2208; [-0.3; -0.3]</li>
        <li>Mbc  &#x2208; [5.179; 5.379]</li>
        <li>M(&#x3C1;(770))  &#x2208; [0.52; 1.05]</li>
        <li>M(K*(892))  &#x2208; [0.792; 0.992]</li>
        <li>&#x3C0;0 from stdPi0s('loose')</li>
        <li>No PID requiremens on charged tracks (stdPi|K('all'))</li>
        <li>Reconstructed candidates are truth-matched</li>
    </ul>
  </description>
</header>

"""

import os
import sys

from basf2 import *
from modularAnalysis import inputMdst, inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import fillParticleList, fillParticleListFromMC
from modularAnalysis import fillConvertedPhotonsList
from modularAnalysis import loadGearbox
from modularAnalysis import vertexKFit, massKFit, massVertexKFit
from modularAnalysis import vertexRave, massVertexRave
from modularAnalysis import rankByHighest
from modularAnalysis import copyParticles
from modularAnalysis import printVariableValues, variablesToExtraInfo, applyCuts
import stdPi0s
import stdCharged

filelist = []

rootOutputFile = "../1210032007.ntup.root"
inputMdstList('default', "../1210032007.dst.root")

stdPi0s.stdPi0s('loose')
stdCharged.stdPi('all')
stdCharged.stdK('all')

pdgDaugCut = 'countDaughters(abs(mcPDG) == 113) == 1 and countDaughters(abs(mcPDG) == 323) == 1'
rhoCutCase1 = '[daughter(0,abs(mcPDG)) == 113 and daughter(0,countDaughters(abs(mcPDG) == 211)) == 2]'
rhoCutCase2 = '[daughter(1,abs(mcPDG)) == 113 and daughter(1,countDaughters(abs(mcPDG) == 211)) == 2]'
KstCutCase1 = '[daughter(0,abs(mcPDG)) == 323 and daughter(0,countDaughters(abs(mcPDG) == 111)) == 1\
 and daughter(0,countDaughters(abs(mcPDG) == 321)) == 1]'
KstCutCase2 = '[daughter(1,abs(mcPDG)) == 323 and daughter(1,countDaughters(abs(mcPDG) == 111)) == 1\
 and daughter(1,countDaughters(abs(mcPDG) == 321)) == 1]'
MCCut = pdgDaugCut + ' and [' + KstCutCase1 + ' or ' + \
    KstCutCase2 + '] and [' + rhoCutCase1 + ' or ' + rhoCutCase2 + ']'
#
fillParticleListFromMC('B+:gen', MCCut, True)

from variables import variables
variables.addAlias('MCTrue', 'passesCut(' + MCCut + ')')
variables.addAlias('vtx_rank', 'extraInfo(chiProb_rank)')
variables.addAlias('MCT_Mbc', 'matchedMC(Mbc)')
variables.addAlias('MK', 'daughter(0,M)')
variables.addAlias('MR', 'daughter(1,M)')
variables.addAlias('MCT_MK', 'daughter(0,matchedMC(M))')
variables.addAlias('MCT_MR', 'daughter(1,matchedMC(M))')
variables.addAlias('MCT_deltaE', 'matchedMC(deltaE)')

# Helicity variables
variables.addAlias('R_p', 'daughter(1,p)')
variables.addAlias('R_px', 'daughter(1,px)')
variables.addAlias('R_py', 'daughter(1,py)')
variables.addAlias('R_pz', 'daughter(1,pz)')
variables.addAlias('hel_Rh1_px', 'daughter(1,useRestFrame(daughter(1,px)))')
variables.addAlias('hel_Rh1_py', 'daughter(1,useRestFrame(daughter(1,py)))')
variables.addAlias('hel_Rh1_pz', 'daughter(1,useRestFrame(daughter(1,pz)))')
variables.addAlias('hel_Rh1_p', 'daughter(1,useRestFrame(daughter(1,p)))')
variables.addAlias('MCT_R_px', 'daughter(1,matchedMC(px))')
variables.addAlias('MCT_R_py', 'daughter(1,matchedMC(py))')
variables.addAlias('MCT_R_pz', 'daughter(1,matchedMC(pz))')
variables.addAlias('MCT_R_p', 'daughter(1,matchedMC(p))')
variables.addAlias('K_p', 'daughter(0,p)')
variables.addAlias('K_px', 'daughter(0,px)')
variables.addAlias('K_py', 'daughter(0,py)')
variables.addAlias('K_pz', 'daughter(0,pz)')
variables.addAlias('hel_Kh1_px', 'daughter(0,useRestFrame(daughter(1,px)))')
variables.addAlias('hel_Kh1_py', 'daughter(0,useRestFrame(daughter(1,py)))')
variables.addAlias('hel_Kh1_pz', 'daughter(0,useRestFrame(daughter(1,pz)))')
variables.addAlias('hel_Kh1_p', 'daughter(0,useRestFrame(daughter(1,p)))')
variables.addAlias('MCT_K_px', 'daughter(0,matchedMC(px))')
variables.addAlias('MCT_K_py', 'daughter(0,matchedMC(py))')
variables.addAlias('MCT_K_pz', 'daughter(0,matchedMC(pz))')
variables.addAlias('MCT_K_p', 'daughter(0,matchedMC(p))')
variables.addAlias('hel_MCT_Kh1_px', 'daughter(0,useRestFrame(daughter(1,matchedMC(px))))')
variables.addAlias('hel_MCT_Kh1_py', 'daughter(0,useRestFrame(daughter(1,matchedMC(py))))')
variables.addAlias('hel_MCT_Kh1_pz', 'daughter(0,useRestFrame(daughter(1,matchedMC(pz))))')
variables.addAlias('hel_MCT_Kh1_p', 'daughter(0,useRestFrame(daughter(1,matchedMC(p))))')
variables.addAlias('hel_MCT_Rh1_px', 'daughter(1,useRestFrame(daughter(1,matchedMC(px))))')
variables.addAlias('hel_MCT_Rh1_py', 'daughter(1,useRestFrame(daughter(1,matchedMC(py))))')
variables.addAlias('hel_MCT_Rh1_pz', 'daughter(1,useRestFrame(daughter(1,matchedMC(pz))))')
variables.addAlias('hel_MCT_Rh1_p', 'daughter(1,useRestFrame(daughter(1,matchedMC(p))))')
variables.addAlias('minus_K_P1P2', 'formula(hel_Kh1_px*K_px + hel_Kh1_py*K_py + hel_Kh1_pz*K_pz)')
variables.addAlias('K_P1P2', 'formula(0 - minus_K_P1P2)')
variables.addAlias('K_PP', 'formula(K_p*hel_Kh1_p)')
variables.addAlias('helK', 'formula( K_P1P2/K_PP )')
variables.addAlias('minus_R_P1P2', 'formula(hel_Rh1_px*R_px + hel_Rh1_py*R_py + hel_Rh1_pz*R_pz)')
variables.addAlias('R_P1P2', 'formula(0 - minus_R_P1P2)')
variables.addAlias('R_PP', 'formula(R_p*hel_Rh1_p)')
variables.addAlias('helR', 'formula( R_P1P2/R_PP )')
variables.addAlias('minus_MCT_K_P1P2', 'formula(hel_MCT_Kh1_px*MCT_K_px + hel_MCT_Kh1_py*MCT_K_py + hel_MCT_Kh1_pz*MCT_K_pz)')
variables.addAlias('MCT_K_P1P2', 'formula(0 - minus_MCT_K_P1P2)')
variables.addAlias('MCT_K_PP', 'formula(MCT_K_p*hel_MCT_Kh1_p)')
variables.addAlias('MCT_helK', 'formula( MCT_K_P1P2/MCT_K_PP )')
variables.addAlias('minus_MCT_R_P1P2', 'formula(hel_MCT_Rh1_px*MCT_R_px + hel_MCT_Rh1_py*MCT_R_py + hel_MCT_Rh1_pz*MCT_R_pz)')
variables.addAlias('MCT_R_P1P2', 'formula(0 - minus_MCT_R_P1P2)')
variables.addAlias('MCT_R_PP', 'formula(MCT_R_p*hel_MCT_Rh1_p)')
variables.addAlias('MCT_helR', 'formula( MCT_R_P1P2/MCT_R_PP )')


copyParticles('pi0:my', 'pi0:loose', False)
massVertexKFit('pi0:my', 0.0, '')
matchMCTruth('pi0:my')
reconstructDecay('rho0:pipi -> pi+:all pi-:all', '0.52 < M < 1.05')
reconstructDecay('K*+:Kpi0 -> K+:all pi0:my', '0.792 < M < 0.992')
reconstructDecay('B+:ours -> K*+:Kpi0 rho0:pipi', 'deltaE<0.3 and deltaE > -0.3 and Mbc < 5.379 and Mbc > 5.179')
vertexRave('B+:ours', 0.0, 'B+ -> [K*+ -> ^K+ ^pi0] [rho0 -> ^pi+ ^pi-] ')

matchMCTruth('B+:ours')
rankByHighest("B+:ours", "chiProb")

matchMCTruth('B+:gen')

RecBTools = ['CustomFloats[Mbc:helR:helK:MK:MR:deltaE:vtx_rank]', '^B+']

MCBTools = ['MCTruth', '^B+ ']
MCBTools += ['CustomFloats[MCTrue:isSignal]', '^B+']
MCBTools += ['CustomFloats[MCT_Mbc:MCT_helR:MCT_helK:MCT_MK:MCT_MR:MCT_MR:MCT_deltaE]', '^B+']


Tools = RecBTools + MCBTools

ntupleFile(rootOutputFile)
ntupleTree('B2KstRho', 'B+:ours', Tools)
ntupleTree('MCB2KstRho', 'B+:gen', RecBTools)

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)
