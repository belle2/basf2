#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# ---------------------------------------------------------------
# example of using OpticalGun to simulate laser light source
# laser source at the mirror (inside bar) emitting toward PMT's
# ---------------------------------------------------------------

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('Components', ['TOP'])
main.add_module(geometry)

# Optical gun
opticalgun = register_module('OpticalGun')
opticalgun.param('angularDistribution', 'Lambertian')
opticalgun.param('alpha', 30.0)
opticalgun.param('startTime', 0)
opticalgun.param('pulseWidth', 10.0e-3)
opticalgun.param('numPhotons', 10)
opticalgun.param('diameter', 10.0e-3)
opticalgun.param('barID', 1)  # if nonzero, local (bar) frame, otherwise Belle II
opticalgun.param('x', 22.4)
opticalgun.param('y', 0.0)
opticalgun.param('z', 128.0)
opticalgun.param('phi', 0.0)
opticalgun.param('theta', 180.0)
opticalgun.param('psi', 0.0)
main.add_module(opticalgun)

# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

# TOP digitization
topdigi = register_module('TOPDigitizer')
main.add_module(topdigi)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'opticalGun.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics
