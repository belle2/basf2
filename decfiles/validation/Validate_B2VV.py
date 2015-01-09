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

generateY4S(1000, Belle2.FileSystem.findFile('decfiles/dec/Bu_rho0rho+.dec'))
loadGearbox()

findMCDecay('B+:sig', 'B+ -> [rho0 -> pi+ pi-] [rho+ -> pi0 pi+]')
ntupleFile('Validate_B2VV.root')
toolsTruth = ['EventMetaData', '^B+']
toolsTruth += ['Kinematics', '^B+ -> ^rho0 ^rho+']
toolsTruth += ['VVAngles', 'B+ -> [rho0 -> ^pi+ ^pi-] [rho+ -> ^pi0 ^pi+]']
ntupleTree('TruthTree', 'B+:sig', toolsTruth)

process(analysis_main)
print statistics
