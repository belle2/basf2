#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# Contributors: P. Urquijo
#
######################################################

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2

generateY4S(1000, Belle2.FileSystem.findFile('decfiles/dec/Bu_D0taunu.dec'))
loadGearbox()
findMCDecay('B+:sig', 'B+ -> anti-D0  tau+ nu_tau')

ntupleFile('Validate_B2Plnu.root')
toolsTruth = ['EventMetaData', '^B+']
toolsTruth += ['Kinematics', '^B+ -> ^anti-D0 ^tau+ ^nu_tau']
toolsTruth += ['SL', 'B+ -> anti-D0 ^tau+ ^nu_tau']
ntupleTree('TruthTree', 'B+:sig', toolsTruth)

process(analysis_main)
print statistics
