#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# CRT tests 2013

# specify here the number of events to simulate,
# position and sizes of rectangular surfaces cosmic ray has to cross,
# time when upper surface is crossed,
# and output file
# -------------------------------------------------------------------------
nevents = [100]
upperPad = [100, 0, 10, 10, 0]  # z, x, size_z, size_x, y [cm]
lowerPad = [100, 0, 10, 10, -100]
startTime = 0  # time at crossing the upperPad [ns]
outfile = 'TOPcrt.root'
# -------------------------------------------------------------------------

# Number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': nevents, 'runList': [1]})

# cosmic gun
cosmicgun = register_module('TOPCosmicGun')
cosmicgun.param('upperPad', upperPad)
cosmicgun.param('lowerPad', lowerPad)
cosmicgun.param('startTime', startTime)
print_params(cosmicgun)

# Show progress of processing
progress = register_module('Progress')

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/top/CRT2013/TOP.xml')

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])

# Simulation
simulation = register_module('FullSim')

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', 100e-3)
TOPdigi.param('timeZeroJitter', 25e-3)

# Output
output = register_module('RootOutput')
output.param('outputFileName', outfile)

# suppress messages during processing:
set_log_level(LogLevel.WARNING)

# Create path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(cosmicgun)
main.add_module(simulation)
main.add_module(TOPdigi)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
