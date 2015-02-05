#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

set_log_level(LogLevel.INFO)
# Change this to point to actual locations on your system.
input_dir = '/data/belle2/BG/Jun2014/bg_SVD/'
output_dir = '/data/belle2/BG/Jun2014/bg_SVD/output/'

components = []
components.append(('Coulomb_HER', 100))
components.append(('Coulomb_LER', 100))
components.append(('RBB_HER', 100))
components.append(('RBB_LER', 100))
components.append(('Touschek_HER', 100))
components.append(('Touschek_LER', 100))
components.append(('twoPhoton', 200))

files = [input_dir + s + '_' + str(t) + 'us.root' for (s, t) in components]

svd_branches = ['MCParticles', 'MCParticlesToSVDSimHits',
                'MCParticlesToSVDTrueHits', 'SVDSimHits', 'SVDTrueHits']

input = register_module('RootInput')
input.param('inputFileNames', files)
# input.param('branchNames', svd_branches)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
# File to save histograms
histo.param('histoFileName', output_dir + 'SVDBackgroundHisto.root')

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
svdBkg.param('componentNames', [s for (s, t) in components])
svdBkg.param('componentTimes', [t for (s, t) in components])
svdBkg.param('outputDirectory', output_dir)

svdBkg.set_log_level(LogLevel.DEBUG)
svdBkg.set_debug_level(10)

# output - do we want output?
output = register_module('RootOutput')
output.param('outputFileName', output_dir + 'SVDBackgroundOutput.root')
output.param('branchNames', ['SVDEnergyDepositionEvents', 'SVDNeutronFluxEvents'])

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
