#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

set_log_level(LogLevel.INFO)

input_dir = '/data/belle2/BG/Jun2014/bg_full_1.2T/'

components = []
components.append(('RBB_HER', '1p2T', 100))
components.append(('RBB_LER', '1p2T', 100))
components.append(('RBB_HER', '1p5T', 100))
components.append(('RBB_LER', '1p5T', 100))

files = [input_dir + s + '_' + t + '_' + str(u) + 'us.root' for (s, t, u) in
         components]

svd_branches = ['MCParticles', 'MCParticlesToSVDSimHits',
                'MCParticlesToSVDTrueHits', 'SVDSimHits', 'SVDTrueHits']

input = register_module('RootInput')
input.param('inputFileNames', files)
# input.param('branchNames', svd_branches)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'SVDBackgroundHisto2.root')  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('components', ['SVD'])

# SVD digitizer
svdDigi = register_module('SVDDigitizer')
svdDigi.param('ElectronicEffects', False)

# SVD clusterizer
# svdClust = register_module('SVDClusterizer')

# SVD beam background
svdBkg = register_module('SVDBackground')
svdBkg.param('componentNames', [s + '_' + t for (s, t, u) in components])
svdBkg.param('componentTimes', [u for (s, t, u) in components])

# output - do we want output?
output = register_module('RootOutput')
output.param('outputFileName', 'SVDBackgroundOutput2.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(histo)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(svdDigi)
# main.add_module(svdClust)
main.add_module(svdBkg)
main.add_module(output)

# Process events
process(main)

# Print statistics
print statistics
