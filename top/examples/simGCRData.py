#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------
# Simulation using MCParticles made of reconstructed cosmic tracks from GCR data
# Input file can be prepared with top/analysis/makeMCParticlesFromGCRData.py
# ---------------------------------------------------------------------------------

from basf2 import *
from simulation import add_simulation
from reconstruction import add_cosmics_reconstruction

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input
roinput = register_module('RootInput')
main.add_module(roinput)

# geometry parameters
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')  # phase 2 geometry
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# Detector simulation
add_simulation(main)

# reconstruction
add_cosmics_reconstruction(main, components=['CDC'], merge_tracks=False)

# output
output = register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'ExtHits', 'Tracks', 'TrackFitResults',
                             'MCParticles', 'TOPBarHits', 'EventT0'])
main.add_module(output)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
