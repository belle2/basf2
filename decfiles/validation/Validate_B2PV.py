#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# Contributors: P. Urquijo
#
######################################################

from basf2 import *
from reconstruction import add_mdst_output
from modularAnalysis import *
from ROOT import Belle2

generateY4S(1000, Belle2.FileSystem.findFile('decfiles/dec/Bd_pipipi0=CPV.dec'
            ))
loadGearbox()
findMCDecay('B0:sig', 'B0 -> pi0 pi+ pi-')

ntupleFile('Validate_B2PV.root')
toolsTruth = ['EventMetaData', '^B0']
toolsTruth += ['Kinematics', '^B0 -> ^pi0 ^pi+ ^pi-']
toolsTruth += ['Dalitz', 'B0 -> ^pi0 ^pi+ ^pi-']
ntupleTree('TruthTree', 'B0:sig', toolsTruth)

process(analysis_main)
print statistics
