#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

b2.set_log_level(b2.LogLevel.INFO)
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

input = b2.register_module('RootInput')
input.param('inputFileNames', files)
# input.param('branchNames', svd_branches)

# Histogram manager immediately after master module
histo = b2.register_module('HistoManager')
# File to save histograms
histo.param('histoFileName', output_dir + 'SVDBackgroundHisto.root')

# Report progress of processing
progress = b2.register_module('Progress')

# Load parameters
gearbox = b2.register_module('Gearbox')
# Create geometry
geometry = b2.register_module('Geometry')
geometry.param('components', ['SVD'])

# SVD digitizer
svdDigi = b2.register_module('SVDDigitizer')
svdDigi.param('ElectronicEffects', False)

# SVD clusterizer
# svdClust = register_module('SVDClusterizer')

# SVD beam background
svdBkg = b2.register_module('SVDBackground')
svdBkg.param('componentNames', [s for (s, t) in components])
svdBkg.param('componentTimes', [t for (s, t) in components])
svdBkg.param('outputDirectory', output_dir)

svdBkg.set_log_level(b2.LogLevel.DEBUG)
svdBkg.set_debug_level(10)

# output - do we want output?
output = b2.register_module('RootOutput')
output.param('outputFileName', output_dir + 'SVDBackgroundOutput.root')
output.param('branchNames', ['SVDEnergyDepositionEvents', 'SVDNeutronFluxEvents'])

# Create paths
main = b2.create_path()

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
b2.process(main)

# Print statistics
print(b2.statistics)
