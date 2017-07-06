#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<input>../1730840030.dst.root</input>
<output>../1730840030.ntup.root</output>
<contact>Bryan Fulsom; bryan.fulsom@pnnl.gov</contact>
</header>
"""

import pdg
import sys
import os
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from beamparameters import add_beamparameters

# prepate files and particle lists
inputMdst('default', '../1730840030.dst.root')

# set up for running at Y(3S)
beamparameters = add_beamparameters(analysis_main, "Y3S")

# loadStdCharged()
# fill lists with some minimal 'goodness' requirements
fillParticleList('mu-:all', 'chiProb>0.001 and abs(d0)<2 and abs(z0)<4')
fillParticleList('e-:all', 'chiProb>0.001 and abs(d0)<2 and abs(z0)<4')
fillParticleList('pi-:all', 'chiProb>0.001')
ntupleFile('../1730840030.ntup.root')

# recoil
reconstructDecay('Upsilon(3S):pipirecoil -> pi+:all pi-:all', '')
roeBuilder_recoil = register_module('RestOfEventBuilder')
roeBuilder_recoil.param('particleList', 'Upsilon(3S):pipirecoil')
buildRestOfEvent('Upsilon(3S):pipirecoil')

# inclusive final state
reconstructDecay('Upsilon:mumu -> mu+:all mu-:all', '9.3602 < M < 9.5602')
vertexRave('Upsilon:mumu', 0.0)
matchMCTruth('Upsilon:mumu')
reconstructDecay('Upsilon(2S):mumu -> mu+:all mu-:all', '9.9232 < M < 10.1232')
vertexRave('Upsilon(2S):mumu', 0.0)
matchMCTruth('Upsilon(2S):mumu')
reconstructDecay('Upsilon:ee -> e+:all e-:all', '9.3602 < M < 9.5602')
vertexRave('Upsilon:ee', 0.0)
matchMCTruth('Upsilon:ee')
reconstructDecay('Upsilon(2S):ee -> e+:all e-:all', '9.9232 < M < 10.1232')
vertexRave('Upsilon(2S):ee', 0.0)
matchMCTruth('Upsilon(2S):ee')

# keep each decay mode separate for ease of use
reconstructDecay('Upsilon(3S):2See -> pi+:all pi-:all Upsilon(2S):ee', '10.2552 < M <10.4552')
vertexRave('Upsilon(3S):2See', 0.0)
matchMCTruth('Upsilon(3S):2See')
reconstructDecay('Upsilon(3S):2Smumu -> pi+:all pi-:all Upsilon(2S):mumu', '10.2552 < M <10.4552')
vertexRave('Upsilon(3S):2Smumu', 0.0)
matchMCTruth('Upsilon(3S):2Smumu')
reconstructDecay('Upsilon(3S):1See -> pi+:all pi-:all Upsilon:ee', '10.2552 < M <10.4552')
vertexRave('Upsilon(3S):1See', 0.0)
matchMCTruth('Upsilon(3S):1See')
reconstructDecay('Upsilon(3S):1Smumu -> pi+:all pi-:all Upsilon:mumu', '10.2552 < M <10.4552')
vertexRave('Upsilon(3S):1Smumu', 0.0)
matchMCTruth('Upsilon(3S):1Smumu')

# Fill the ntuples
ntuplepipi = ['InvMass', '^Upsilon(3S):pipirecoil']
ntuplepipi += ['PID', 'Upsilon(3S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['Track', 'Upsilon(3S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['TrackHits', 'Upsilon(3S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['Charge', 'Upsilon(3S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['EventMetaData', '^Upsilon(3S):pipirecoil']
ntuplepipi += ['RecoilKinematics', '^Upsilon(3S):pipirecoil']
ntuplepipi += ['Kinematics', '^Upsilon(3S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['CMSKinematics', '^Upsilon(3S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['RecoStats', '^Upsilon(3S):pipirecoil']
ntuplepipi += ['MCTruth', "Upsilon(3S):pipirecoil -> ^pi+ ^pi-"]
ntuplepipi += ['MCHierarchy', "Upsilon(3S):pipirecoil -> ^pi+ ^pi-"]
ntuplepipi += ['MCKinematics', "Upsilon(3S):pipirecoil -> ^pi+ ^pi-"]

ntuple2see = ['EventMetaData', 'Upsilon(3S):2See']
ntuple2see += ['RecoStats', 'Upsilon(3S):2See']
ntuple2see += ['PID', 'Upsilon(3S):2See -> ^pi+ ^pi- [Upsilon(2S):ee -> ^e+ ^e-]']
ntuple2see += ['Kinematics', '^Upsilon(3S):2See -> ^pi+ ^pi- [^Upsilon(2S):ee -> ^e+ ^e-]']
ntuple2see += ['InvMass', '^Upsilon(3S):2See -> ^pi+ ^pi- [^Upsilon(2S):ee -> ^e+ ^e-]']
ntuple2see += ['Vertex', '^Upsilon(3S):2See -> ^pi+ ^pi- [^Upsilon(2S):ee -> ^e+ ^e-]']
ntuple2see += ['MCTruth', '^Upsilon(3S):2See -> ^pi+ ^pi- [^Upsilon(2S):ee -> ^e+ ^e-]']

ntuple2smumu = ['EventMetaData', 'Upsilon(3S):2Smumu']
ntuple2smumu += ['RecoStats', 'Upsilon(3S):2Smumu']
ntuple2smumu += ['PID', 'Upsilon(3S):2Smumu -> ^pi+ ^pi- [Upsilon(2S):mumu -> ^mu+ ^mu-]']
ntuple2smumu += ['Kinematics', '^Upsilon(3S):2Smumu -> ^pi+ ^pi- [^Upsilon(2S):mumu -> ^mu+ ^mu-]']
ntuple2smumu += ['InvMass', '^Upsilon(3S):2Smumu -> ^pi+ ^pi- [^Upsilon(2S):mumu -> ^mu+ ^mu-]']
ntuple2smumu += ['Vertex', '^Upsilon(3S):2Smumu -> ^pi+ ^pi- [^Upsilon(2S):mumu -> ^mu+ ^mu-]']
ntuple2smumu += ['MCTruth', '^Upsilon(3S):2Smumu -> ^pi+ ^pi- [^Upsilon(2S):mumu -> ^mu+ ^mu-]']

ntuple1see = ['EventMetaData', 'Upsilon(3S):1See']
ntuple1see += ['RecoStats', 'Upsilon(3S):1See']
ntuple1see += ['PID', 'Upsilon(3S):1See -> ^pi+ ^pi- [Upsilon:ee -> ^e+ ^e-]']
ntuple1see += ['Kinematics', '^Upsilon(3S):1See -> ^pi+ ^pi- [^Upsilon:ee -> ^e+ ^e-]']
ntuple1see += ['InvMass', '^Upsilon(3S):1See -> ^pi+ ^pi- [^Upsilon:ee -> ^e+ ^e-]']
ntuple1see += ['Vertex', '^Upsilon(3S):1See -> ^pi+ ^pi- [^Upsilon:ee -> ^e+ ^e-]']
ntuple1see += ['MCTruth', '^Upsilon(3S):1See -> ^pi+ ^pi- [^Upsilon:ee -> ^e+ ^e-]']

ntuple1smumu = ['EventMetaData', 'Upsilon(3S):1Smumu']
ntuple1smumu += ['RecoStats', 'Upsilon(3S):1Smumu']
ntuple1smumu += ['PID', 'Upsilon(3S):1Smumu -> ^pi+ ^pi- [Upsilon:mumu -> ^mu+ ^mu-]']
ntuple1smumu += ['Kinematics', '^Upsilon(3S):1Smumu -> ^pi+ ^pi- [^Upsilon:mumu -> ^mu+ ^mu-]']
ntuple1smumu += ['InvMass', '^Upsilon(3S):1Smumu -> ^pi+ ^pi- [^Upsilon:mumu -> ^mu+ ^mu-]']
ntuple1smumu += ['Vertex', '^Upsilon(3S):1Smumu -> ^pi+ ^pi- [^Upsilon:mumu -> ^mu+ ^mu-]']
ntuple1smumu += ['MCTruth', '^Upsilon(3S):1Smumu -> ^pi+ ^pi- [^Upsilon:mumu -> ^mu+ ^mu-]']

# write out the ntuples
ntupleTree('ntuple_pipi', 'Upsilon(3S):pipirecoil', ntuplepipi)
ntupleTree('ntuple_2see', 'Upsilon(3S):2See', ntuple2see)
ntupleTree('ntuple_2smumu', 'Upsilon(3S):2Smumu', ntuple2smumu)
ntupleTree('ntuple_1see', 'Upsilon(3S):1See', ntuple1see)
ntupleTree('ntuple_1smumu', 'Upsilon(3S):1Smumu', ntuple1smumu)

process(analysis_main)
