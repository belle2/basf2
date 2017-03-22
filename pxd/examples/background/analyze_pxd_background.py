#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *

set_log_level(LogLevel.INFO)
# Change this to point to actual locations on your system.
input_dir = '/home/benjamin/Desktop/BG/'
output_dir = '/home/benjamin/Desktop/BG/output'

components = []
# components.append(('Coulomb_HER', 100))
# components.append(('Coulomb_LER', 100))
# components.append(('RBB_HER', 100))
# components.append(('RBB_LER', 100))
components.append(('Touschek_HER', 1))
# components.append(('Touschek_LER', 100))
# components.append(('twoPhoton', 200))

files = [input_dir + s + '_' + str(t) + 'us.root' for (s, t) in components]

files = [input_dir + 'output_Touschek_HER_study_1.root']


pxd_branches = ['MCParticles', 'MCParticlesToPXDSimHits',
                'MCParticlesToPXDTrueHits', 'PXDSimHits', 'PXDTrueHits']

input = register_module('RootInput')
input.param('inputFileNames', files)
# input.param('branchNames', pxd_branches)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
# File to save histograms
histo.param('histoFileName', output_dir + 'PXDBackgroundHisto.root')

# Report progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('components', ['PXD'])

# PXD digitizer
pxdDigi = register_module('PXDDigitizer')

# PXD clusterizer
pxdClust = register_module('PXDClusterizer')

# PXD beam background
pxdBkg = register_module('PXDBackground')
# pxdBkg.param('componentNames', [s for (s, t) in components])
# pxdBkg.param('componentTimes', [t for (s, t) in components])
pxdBkg.param('componentName', 'Touschek_HER')
pxdBkg.param('componentTime', 1.0)
pxdBkg.param('outputDirectory', output_dir)

pxdBkg.set_log_level(LogLevel.DEBUG)
pxdBkg.set_debug_level(10)

# output - do we want output?
output = register_module('RootOutput')
output.param('outputFileName', output_dir + 'PXDBackgroundOutput.root')
output.param('branchNames', ['PXDEnergyDepositionEvents', 'PXDNeutronFluxEvents'])

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(histo)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pxdDigi)
main.add_module(pxdClust)
main.add_module(pxdBkg)
main.add_module(output)

# Process events
process(main)

# Print statistics
print(statistics)
