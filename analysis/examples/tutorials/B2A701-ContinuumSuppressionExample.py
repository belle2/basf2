#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# This tutorial demonstrates how to perform a
# skim of continuum MC for B0 candidates in Bd->KsPi0 decays.
#
# One can then run the following 2 continuum suppression tutorials
# (B2A702 and B2A703) over these skims, to apply TMVA training
# and expert.
#
# Usage:
#  -Command-line (for a single input file specified below):
#    ./B2A701-ContinuumSuppressionExample.py mc35_ccbar_BGx0_s00.root mc35_ccbar_BGx0_s00.mdst.root
#  -Queue (for the entire /mc35_ccbar_BGx0_s00/* directory):
#    bsub -q l -e error_mc35_ccbar_BGx0_s00.log -o output_mc35_ccbar_BGx0_s00.log
#     'basf2 ./B2A701-ContinuumSuppressionExample.py mc35_ccbar_BGx0_s00.root mc35_ccbar_BGx0_s00.mdst.root
#      -i "/hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/BGx0/mc35_ccbar_BGx0_s00/*"'
#
# This will produce a flat ntuple along with a skimmed mdst.root file
#
# Contributors: P. Goldenzweig (July 2014)
#
################################################################################

import os.path
import sys
from basf2 import *
from modularAnalysis import *
from stdFSParticles import stdPi0
from stdV0s import stdKshorts


outTuple = str(sys.argv[1])
outSkim = str(sys.argv[2])

main = create_path()

inputMdstList('MC5', ['/hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/BGx0/mc35_ccbar_BGx0_s00/ccbar_e0001r0973_s00_BGx0.mdst.root'])

# Do the analysis
fillParticleList('gamma:all', '')
fillParticleList('pi+:good', '')
fillParticleList('pi-:good', '')

reconstructDecay('K_S0 -> pi+:good pi-:good', '', 1)
reconstructDecay('pi0  -> gamma:all gamma:all', '', 1)
reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2')

matchMCTruth('B0')
buildRestOfEvent('B0')
buildContinuumSuppression('B0')
TagV('B0', 'breco')


toolsB = ['EventMetaData', '^B0']
toolsB += ['RecoStats', '^B0']
toolsB += ['DeltaEMbc', '^B0']
toolsB += ['ContinuumSuppression', '^B0']
toolsB += ['TagVertex', '^B0']
toolsB += ['MCTagVertex', '^B0']
toolsB += ['DeltaT', '^B0']
toolsB += ['MCDeltaT', '^B0']
toolsB += ['InvMass[BeforeFit]', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ]   ^pi0                             ']
toolsB += ['PID', ' B0 -> [  K_S0 -> ^pi+:good ^pi-:good ]    pi0                             ']
toolsB += ['Kinematics', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]']
toolsB += ['MCTruth', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]']
toolsB += ['MCHierarchy', '^B0 -> [ ^K_S0 -> ^pi+:good ^pi-:good ] [ ^pi0 -> ^gamma:all ^gamma:all ]']


# Write flat ntuples
ntupleFile(outTuple)
ntupleTree('ntupB0', 'B0', toolsB)

# Write skim mdst files
skimMdst = register_module('SkimFilter')
skimMdst.param('particleLists', ['B0'])
analysis_main.add_module(skimMdst)

save_path = create_path()
skimMdst.if_true(save_path)
outputMdst(outSkim, path=save_path)

process(analysis_main)
print(statistics)
