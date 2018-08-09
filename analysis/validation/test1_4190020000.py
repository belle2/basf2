#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<input>../4190020000.dst.root</input>
<output>../4190020000.ntup.root</output>
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
inputMdst('default', '../4190020000.dst.root')

# set up for running at Y(6S)
beamparameters = add_beamparameters(analysis_main, "Y6S")
# pdg.add_particle("Upsilon(6S)", 9010553, 11.0190e+00, 0.0790e+00, 0, 1)

# loadStdCharged()
fillParticleList('pi+:std', '')
ntupleFile('../4190020000.ntup.root')

# pion recoil
reconstructDecay('Upsilon(6S):pipirecoil -> pi+:std pi-:std', '')
reconstructDecay('Upsilon(6S):pirecoil -> pi+:std', '')

roeBuilder_recoil = register_module('RestOfEventBuilder')
roeBuilder_recoil.param('particleList', 'Upsilon(6S):pipirecoil')
buildRestOfEvent('Upsilon(6S):pipirecoil')

roeBuilder_pirecoil = register_module('RestOfEventBuilder')
roeBuilder_pirecoil.param('particleList', 'Upsilon(6S):pirecoil')
buildRestOfEvent('Upsilon(6S):pirecoil')

ntuplepipi = ['InvMass', '^Upsilon(6S):pipirecoil']
ntuplepipi += ['PID', 'Upsilon(6S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['Track', 'Upsilon(6S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['TrackHits', 'Upsilon(6S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['Charge', 'Upsilon(6S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['EventMetaData', '^Upsilon(6S):pipirecoil']
ntuplepipi += ['RecoilKinematics', '^Upsilon(6S):pipirecoil']
ntuplepipi += ['Kinematics', '^Upsilon(6S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['CMSKinematics', '^Upsilon(6S):pipirecoil -> ^pi+ ^pi-']
ntuplepipi += ['RecoStats', '^Upsilon(6S):pipirecoil']
ntuplepipi += ['MCTruth', "Upsilon(6S):pipirecoil -> ^pi+ ^pi-"]
ntuplepipi += ['MCHierarchy', "Upsilon(6S):pipirecoil -> ^pi+ ^pi-"]
ntuplepipi += ['MCKinematics', "Upsilon(6S):pipirecoil -> ^pi+ ^pi-"]
ntuplepipi += ['CustomFloats[daughter(0,mRecoil):daughter(1,mRecoil)]', '^Upsilon(6S):pipirecoil']

ntuplepi = ['PID', 'Upsilon(6S):pirecoil -> ^pi+']
ntuplepi += ['Track', 'Upsilon(6S):pirecoil -> ^pi+']
ntuplepi += ['TrackHits', 'Upsilon(6S):pirecoil -> ^pi+']
ntuplepi += ['Charge', 'Upsilon(6S):pirecoil -> ^pi+']
ntuplepi += ['EventMetaData', '^Upsilon(6S):pirecoil']
ntuplepi += ['RecoilKinematics', '^Upsilon(6S):pirecoil']
ntuplepi += ['Kinematics', '^Upsilon(6S):pirecoil -> ^pi+']
ntuplepi += ['CMSKinematics', '^Upsilon(6S):pirecoil -> ^pi+']
ntuplepi += ['RecoStats', '^Upsilon(6S):pirecoil']
ntuplepi += ['MCTruth', "Upsilon(6S):pirecoil -> ^pi+"]
ntuplepi += ['MCHierarchy', "Upsilon(6S):pirecoil -> ^pi+"]
ntuplepi += ['MCKinematics', "Upsilon(6S):pirecoil -> ^pi+"]

ntupleTree('pipi_ntuple', 'Upsilon(6S):pipirecoil', ntuplepipi)
ntupleTree('pi_ntuple', 'Upsilon(6S):pirecoil', ntuplepi)

process(analysis_main)
