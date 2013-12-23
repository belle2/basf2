#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# ---------------------------------------------------------------
# example of using OpticalGun to simulate laser light sources
# two sources at the left and right side of prism, outside quartz
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
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Optical sources
source1 = register_module('OpticalGun')
source1.param('alpha', 45.0)
source1.param('startTime', 0)
source1.param('pulseWidth', 10.0e-3)
source1.param('numPhotons', 10)
source1.param('diameter', 10.0e-3)
source1.param('barID', 5)  # if nonzero, local (bar) frame, otherwise Belle II
source1.param('x', -22.6)
source1.param('y', 0.0)
source1.param('z', -129.9)
source1.param('phi', 0.0)
source1.param('theta', 180.0)
source1.param('psi', 0.0)
main.add_module(source1)

source2 = register_module('OpticalGun')
source2.param('alpha', 45.0)
source2.param('startTime', 0)
source2.param('pulseWidth', 10.0e-3)
source2.param('numPhotons', 10)
source2.param('diameter', 10.0e-3)
source2.param('barID', 5)  # if nonzero, local (bar) frame, otherwise Belle II
source2.param('x', 22.6)
source2.param('y', 0.0)
source2.param('z', -129.9)
source2.param('phi', 0.0)
source2.param('theta', 180.0)
source2.param('psi', 0.0)
main.add_module(source2)

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
