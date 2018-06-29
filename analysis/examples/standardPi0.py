#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdPi0s import *

import sys

# ----------------------------------------------------------------------------------
# This example loads Particle(s) from specified ROOT files and makes standard pi0
# reconstruction
# ----------------------------------------------------------------------------------

if len(sys.argv) != 2:
    sys.exit('Must provide one input parameter:[input_root_file_name]')

inputMdstFileName = sys.argv[1]

# load the input MDST file
inputMdst('default', inputMdstFileName)

# fetch the standard photon (input for standard pi0 reconstruction)
# the reconstructed photon Particles are collected in the gamma:all ParticleList
# stdPhoton()     already in the stdPi0()

# make  standard pi0 reconstruction
# the reconstructed pi0 Particles are collected in the pi0:all, pi0:loose, and pi0:good ParticleList
stdPi0()


toolsPI0 = ['MCTruth', '^pi0']
toolsPI0 += ['Kinematics', '^pi0']
toolsPI0 += ['MassBeforeFit', '^pi0']
toolsPI0 += ['MCKinematics', '^pi0']
toolsPI0 += ['EventMetaData', '^pi0']
toolsPI0 += ['MomentumUncertainty', '^pi0']
toolsPI0 += ['Cluster', 'pi0 -> ^gamma ^gamma']

# write flat ntuples
ntupleFile('StandardPi0_example_output.root')
ntupleTree('pi0Tree', 'pi0:all', toolsPI0)

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
