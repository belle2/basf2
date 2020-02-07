#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# basf2 test_shower2cluster.py
# input file is /home/belle2/harat/MCsample/mixed_e0001r0001_s00_BGx1.mdst.root
# Output file is shower2cluster.root

import basf2 as b2

main = b2.create_path()

# This is the module for ECLShower to ECLCluster
showertocluster = b2.register_module('ECLShowertoCluster')

# now both files below work
inFileROOT = '/home/belle2/harat/MCsample/charged_e0001r0001_s00_BGx1.mdst.root'
# inFileROOT = '/home/belle2/harat/MCsample/mixed_e0001r0001_s00_BGx1.mdst.root'

rootinput = b2.register_module('RootInput')
rootinput.param('inputFileName', inFileROOT)
main.add_module(rootinput)
main.add_module(showertocluster)

particleLoader = b2.register_module('ParticleLoader')
main.add_module(particleLoader)

# Save ECLCluster in rootfile
output = b2.register_module('RootOutput')
outFileROOT = 'shower2cluster.root'
output.param('outputFileName', outFileROOT)

branches = ['ECLShowers',
            'ECLClusters',
            'ECLClustersToTracks',
            'ECLClustersToMCParticles']

branches += ['MCParticles',
             'Tracks']

output.param('branchNames', branches)
main.add_module(output)

# Start processing of modules
b2.process(main)

# Print call statistics
print(b2.statistics)
