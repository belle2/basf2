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
add_evtgen_generator(analysis_main, 'signal', Belle2.FileSystem.findFile('decfiles/dec/Bu_Dst0enu.dec'))
loadGearbox()
findMCDecay('B+:sig', 'B+ => [anti-D*0 -> anti-D0 pi0] e+ nu_e')

ntupleFile('../Validate_B2Vlnu.root')
tools = ['EventMetaData', '^B+']
tools += ['Kinematics', '^B+ -> ^anti-D*0 ^e+ ^nu_e']
tools += ['VVAngles', '^B+ -> [anti-D*0 -> ^D0 ^pi0] ^e+ ^nu_e']
tools += ['SL', '^B+ -> [anti-D*0 -> D0 pi0] ^e+ ^nu_e']
ntupleTree('TruthTree', 'B+:sig', tools)

process(analysis_main)
print(statistics)
