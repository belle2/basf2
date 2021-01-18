#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------
# Simulation using MCParticles made of reconstructed cosmic tracks from GCR data
# Input file can be prepared with top/analysis/makeMCParticlesFromGCRData.py
# ---------------------------------------------------------------------------------

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_cosmics_reconstruction

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Define a global tag (note: the one given bellow will become out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# Detector simulation
add_simulation(main)

# reconstruction
add_cosmics_reconstruction(main, components=['CDC'], merge_tracks=False)

# output
output = b2.register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'ExtHits', 'Tracks', 'TrackFitResults',
                             'MCParticles', 'TOPBarHits', 'EventT0'])
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
