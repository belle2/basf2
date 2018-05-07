#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Contributors: P. Urquijo
#
######################################################

from basf2 import *
from generators import add_evtgen_generator
from reconstruction import add_mdst_output
from modularAnalysis import *
from ROOT import Belle2

setupEventInfo(10000)
add_evtgen_generator(analysis_main, 'signal', Belle2.FileSystem.findFile('decfiles/dec/Bu_Xulnu=cocktail.dec'))
loadGearbox()

ntupleFile('../Validate_B2Xulnu.root')

findMCDecay('B+:pi', 'B+ => pi0 e+ nu_e')
toolsPi = ['EventMetaData', '^B+']
toolsPi += ['Kinematics', '^B+ -> ^pi0 ^e+ ^nu_e']
toolsPi += ['SL', '^B+ -> pi0 ^e+ ^nu_e']
ntupleTree('PiTree', 'B+:pi', toolsPi)

findMCDecay('B+:rho', 'B+ => [ rho0 -> pi+ pi- ] e+ nu_e')
toolsRho = ['EventMetaData', '^B+']
toolsRho += ['Kinematics', '^B+ -> ^rho0 ^e+ ^nu_e']
toolsRho += ['VVAngles', '^B+ -> [ rho0 -> ^pi+ ^pi- ] ^e+ ^nu_e']
toolsRho += ['SL', '^B+ -> [ rho0 -> pi+ pi- ] ^e+ ^nu_e']
ntupleTree('RhoTree', 'B+:rho', toolsRho)

findMCDecay('B+:Xu', 'B+ => Xu0 e+ nu_e')
toolsXu = ['EventMetaData', '^B+']
toolsXu += ['Kinematics', '^B+ -> ^Xu0 ^e+ ^nu_e']
toolsXu += ['SL', '^B+ -> Xu0 ^e+ ^nu_e']
ntupleTree('XuTree', 'B+:Xu', toolsXu)

findMCDecay('B+:pipi', 'B+ --> pi+ pi- e+ nu_e')
tools2Pi = ['EventMetaData', '^B+']
tools2Pi += ['Kinematics', '^B+ -> ^pi+ ^pi- ^e+ ^nu_e']
tools2Pi += ['VVAngles', '^B+ -> ^pi+ ^pi- ^e+ ^nu_e']
ntupleTree('PiPiTree', 'B+:pipi', tools2Pi)

findMCDecay('B+:pitau', 'B+ => pi0 tau+ nu_tau')
toolsPiTau = ['EventMetaData', '^B+']
toolsPiTau += ['Kinematics', '^B+ -> ^pi0 ^tau+ ^nu_tau']
toolsPiTau += ['SL', '^B+ -> pi0 ^tau+ ^nu_tau']
ntupleTree('PiTauTree', 'B+:pitau', toolsPiTau)

summaryOfLists(['B+:pi', 'B+:rho', 'B+:pipi', 'B+:pitau', 'B+:Xu'])

findMCDecay('B+', 'B+')
findMCDecay('B-', 'B-')
findMCDecay('B0', 'B0')
findMCDecay('anti-B0', 'anti-B0')

summaryOfLists(['B+', 'B0', 'anti-B0', 'B-'])

process(analysis_main)
print(statistics)
