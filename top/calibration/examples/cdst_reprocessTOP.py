#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from reconstruction import add_top_modules, add_cdst_output

use_central_database('development')  # some new stuff from TOP with unlimited IOV's
use_central_database('data_reprocessing_prod6')  # production global tag

# Create path
main = create_path()

# input: cdst
roinput = register_module('RootInput')
main.add_module(roinput)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'TOP'])
geometry.param('useDB', False)
main.add_module(geometry)

# Time Recalibrator
recalibrator = register_module('TOPTimeRecalibrator')
recalibrator.param('subtractBunchTime', False)
main.add_module(recalibrator)

# top reconstruction
add_top_modules(main)

# replace TOP in PID likelihoods with new values

# output: cdst
add_cdst_output(main)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
