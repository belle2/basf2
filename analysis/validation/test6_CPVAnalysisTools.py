#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <input>B2JpsiKs_mu_BGx1_MC9.root</input>
  <output>CPVToolsOutput.root</output>
  <contact>Fernando Abudinen; abudinen@mpp.mpg.de</contact>
  <description>This file employs all the time dependent CPV analysis tools. It reconstructs B0sig->J/PsiKs on the signal side
  and applies the flavor tagger on the ROE. The vertex of B0sig is reconstructed and the vertex of B0tag is reconstructed
  with the TagV module. </description>
</header>
"""

import ROOT
import sysconfig
ROOT.gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
from basf2 import *
from flavorTagger import *
from variables import variables
import sys

inputMdst('default', '../B2JpsiKs_mu_BGx1_MC9.root')

# Reconstruction of signal side and MC match
fillParticleList('pi+:all', '')
fillParticleList('mu+:all', '')

reconstructDecay('K_S0:pipi -> pi+:all pi-:all', 'dM<0.25')
reconstructDecay('J/psi:mumu -> mu+:all mu-:all', 'dM<0.11')
reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', 'Mbc > 5.2 and abs(deltaE)<0.2')

matchMCTruth('B0:jpsiks')

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jpsiks')

# Get Special GT for the flavor tagger weight files
use_central_database("GT_gen_prod_003.11_release-00-09-01-FEI-a")

# Flavor Tagger, Vertex of Signal Side and TagV
flavorTagger(
    particleLists=['B0:jpsiks'],
    weightFiles='B2JpsiKs_muBGx1')

vertexRave('B0:jpsiks', 0.0, 'B0:jpsiks -> [J/psi:mumu -> ^mu+ ^mu-] K_S0', '')
TagV('B0:jpsiks', MCassociation='breco')

toolsDST = ['EventMetaData', '^B0']
toolsDST += ['FlavorTagging[TMVA-FBDT, FANN-MLP, qpCategories]', '^B0']
toolsDST += ['RecoStats', '^B0']
toolsDST += ['MCVertex', '^B0']
toolsDST += ['Vertex', '^B0']
toolsDST += ['MCTagVertex', '^B0']
toolsDST += ['TagVertex', '^B0']
toolsDST += ['MCDeltaT', '^B0']
toolsDST += ['DeltaT', '^B0']
toolsDST += ['DeltaEMbc', '^B0']
toolsDST += ['InvMass', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
toolsDST += ['CustomFloats[isSignal]', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
toolsDST += ['PDGCode', '^B0']
toolsDST += ['CMSKinematics', '^B0']
toolsDST += ['MCHierarchy', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['PID', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['TrackHits', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['Track', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['MCTruth', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
toolsDST += ['ROEMultiplicities', '^B0']

ntupleFile('../CPVToolsOutput.root')
ntupleTree('B0tree', 'B0:jpsiks', toolsDST)

summaryOfLists(['B0:jpsiks'])

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
