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

generateY4S(10000, Belle2.FileSystem.findFile('decfiles/dec/Bd_rho0gamma.dec'))
loadGearbox()

ntupleFile('Validate_B2Vgamma.root')

findMCDecay('B0:rho', 'B0 -> [ rho0 -> pi+ pi- ] gamma')
toolsRho = ['EventMetaData', '^B0']
toolsRho += ['Kinematics', '^B0 -> ^rho0 ^gamma']
toolsRho += ['Helicity', 'B0 -> [ rho0 -> ^pi+ ^pi- ] gamma']
toolsRho += ['Helicity', 'B0 ->  ^rho0  ^gamma']
ntupleTree('RhoTree', 'B0:rho', toolsRho)

findMCDecay('B+', 'B+')
findMCDecay('B-', 'B-')
findMCDecay('B0', 'B0')
findMCDecay('B_s0', 'B_s0')
findMCDecay('anti-B0', 'anti-B0')
findMCDecay('anti-B_s0', 'anti-B_s0')

summaryOfLists(['B+', 'B0', 'anti-B0', 'B-'])

process(analysis_main)
print statistics
