#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Shower to Cluster

Input:
    File with mdst format

Output:
    File named 'shower_to_cluster.root'

Usage:
    $ basf2 -i <input_file> -o <output_file>
            -n <number_of_events> test_shower2cluster.py
"""

import basf2 as b2

__author__ = ['Vishal Bhardwaj', 'Abtin Narimani Charan']
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'ECLShowertoCluster' module
eclShowertoCluster = b2.register_module('ECLShowertoCluster')
mainPath.add_module(eclShowertoCluster)

# Register and add 'RootInput' module
rootInput = b2.register_module('RootInput')
mainPath.add_module(rootInput)

# Register and add 'ParticleLoader' module
particleLoader = b2.register_module('ParticleLoader')
mainPath.add_module(particleLoader)

"""Register and add 'RootOutput' module.
This saves ECLCluster in a root file.
"""
rootOutput = b2.register_module('RootOutput')
rootOutput.param('outputFileName', 'shower_to_cluster.root')
rootOutput.param('branchNames', ['ECLShowers',
                                 'ECLClusters',
                                 'ECLClustersToTracks',
                                 'ECLClustersToMCParticles',
                                 'MCParticles',
                                 'Tracks'])
mainPath.add_module(rootOutput)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
