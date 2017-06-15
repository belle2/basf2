#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1310040140.dst.root</input>
  <output>../1310040140.ntup.root</output>
  <contact>Bilas Pal; palbs@ucmail.uc.edu </contact>
</header>
"""

#######################################################
# Analysis Validation : B2Charmless  WG
# reconstruct the B_s0 -> K_S0 K_S0 decay!
#                          |    |
#                          |    |
#              pi+ pi- <---|    |---> pi+ pi-
#
#
# Bilas Pal, April 12 2017
# palbs@ucmail.uc.edu
#
######################################################


from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from stdV0s import *
from ROOT import Belle2
from modularAnalysis import inputMdstList


# set the BeamParameters for running at Y(5S)
beamparameters = add_beamparameters(analysis_main, "Y5S")
# print_params(beamparameters)

# HLT physics trigger
# add_HLT_Y4S(analysis_main)

# disable to access DB server
# reset_database()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"),
#                   "", True, LogLevel.ERROR)

# load input ROOT file
gb2_setuprel = "release-00-08-00"

inputMdst('default', '../1310040140.dst.root')
# inputMdst('default', sys.argv[1])
# inputMdstList('default',['~/abc/sub00/mdst_000001_prod00001598_task00000001.root',
# '~/abc/sub00/mdst_000002_prod00001598_task00000002.root',
# '~/abc/sub00/mdst_000003_prod00001598_task00000003.root',
# '~/abc/sub00/mdst_000004_prod00001598_task00000004.root',
# '~/abc/sub00/mdst_000005_prod00001598_task00000005.root',
# '~/abc/sub00/mdst_000006_prod00001598_task00000006.root',
# '~/abc/sub00/mdst_000007_prod00001598_task00000007.root',
# '~/abc/sub00/mdst_000008_prod00001598_task00000008.root'])


stdKshorts()
applyCuts('K_S0:all', '0.485611 < M < 0.509611')

# vertex fit to K_S0 daughters
# vertexRave('K_S0:mdst',0.001)
# vertexKFit('K_S0:all',0.0)

# Reconstruct B_s0 from K_S0:mdst & K_S0:mdst
reconstructDecay('B_s0:KSKS -> K_S0:all K_S0:all', 'Mbc > 5.34 and deltaE > -0.2 and deltaE < 0.1')

# vertex fit to B_s0 daughters
# vertexRave('B_s0:KSKS',0.001,'B_s0 -> ^K_S0 ^K_S0')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------
matchMCTruth('B_s0:KSKS')

toolsBs = ['EventMetaData', '^B_s0']
toolsBs += ['InvMass', 'B_s0 -> ^K_S0 ^K_S0']
toolsBs += ['Kinematics', '^B_s0 ->[K_S0 -> ^pi+ ^pi-] [K_S0 -> ^pi+ ^pi-]']
toolsBs += ['MCKinematics', '^B_s0 -> [K_S0 -> ^pi+ ^pi-] [K_S0 -> ^pi+ ^pi-]']
toolsBs += ['DeltaEMbc', '^B_s0']
# toolsBs += ['Vertex', '^B_s0']
# toolsBs += ['MCVertex', '^B_s0']
toolsBs += ['MCTruth', '^B_s0']
toolsBs += ['CustomFloats[isSignal]', '^B_s0 -> ^K_S0 ^K_S0']

ntupleFile('../1310040140.ntup.root')
ntupleTree('Bs', 'B_s0:KSKS', toolsBs)

set_log_level(LogLevel.INFO)

#
# Process and print statistics
#

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
