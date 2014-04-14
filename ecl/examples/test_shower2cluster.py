#!/usr/bin/env python
# -*- coding: utf-8 -*-

# basf2 test_shower2cluster.py
# input file is /home/belle2/harat/MCsample/mixed_e0001r0001_s00_BGx1.mdst.root
# Output file is shower2cluster.root

from basf2 import *
from modularAnalysis import *

import sys

main = create_path()

# This is the module for ECLShower to ECLCluster
showertocluster = register_module('ECLShowertoCluster')

rootinput = register_module('RootInput')
# now both files below work
inFileROOT = \
    '/home/belle2/harat/MCsample/charged_e0001r0001_s00_BGx1.mdst.root'
# inFileROOT = '/home/belle2/harat/MCsample/mixed_e0001r0001_s00_BGx1.mdst.root'

rootinput.param('inputFileName', inFileROOT)
main.add_module(rootinput)
main.add_module(showertocluster)

particleLoader = register_module('ParticleLoader')
main.add_module(particleLoader)

# Save ECLCluster in rootfile
output = register_module('RootOutput')
outFileROOT = 'shower2cluster.root'
output.param('outputFileName', outFileROOT)
branches = ['ECLShowers', 'ECLClusters', 'ECLClustersToTracks',
            'ECLClustersToMCParticles']
branches += ['MCParticles', 'Tracks']
output.param('branchNames', branches)
main.add_module(output)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics

