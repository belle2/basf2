#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

set_log_level(LogLevel.INFO)

input_dir = '/data/belle2/BG/Feb2014/output2/'

components = [\
    ('Coulomb_HER', 10), ('Coulomb_LER', 10), ('RBB_HER', 10),\
    ('RBB_LER', 10), ('Touschek_HER', 10), ('Touschek_LER', 10)]
files = [input_dir + s + '_' + str(t) + 'us.root' for (s, t) in components]

svd_branches = ['MCParticles',
    'MCParticlesToSVDSimHits',
    'MCParticlesToSVDTrueHits',
    'SVDSimHits', 'SVDTrueHits']

input = register_module('RootInput')
input.param('inputFileNames', files)
input.param('branchNames', svd_branches)

# Report progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('components', ['SVD'])

# SVD digitizer
svdDigi = register_module('SVDDigitizer')

# SVD clusterizer
#svdClust = register_module('SVDClusterizer')

# SVD beam background
svdBkg = register_module('SVDBackground')
svdBkg.param('componentNames', [s for (s, t) in components])
svdBkg.param('componentTimes', [t for (s, t) in components])

#output - do we want output?
output = register_module('RootOutput')
output.param('outputFileName', '/data/belle2/BG/Feb2014/output1/QED_50us.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(svdDigi)
#main.add_module(svdClust)
main.add_module(svdBkg)
# main.add_module(output)

# Process events
process(main)

# Print statistics
print statistics
