#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# ----------------------------------------------------------------
# example of using OpticalGun to simulate laser calibration system
# ----------------------------------------------------------------

# Create path
main = create_path()


# define a single laser source (a fiber)

def fiber(x, barID=1, path=main):
    lightSource = register_module('OpticalGun')
    lightSource.param('barID', barID)  # if nonzero, local (bar) frame, otherwise Belle II
    lightSource.param('startTime', 0)
    lightSource.param('pulseWidth', 10.0e-3)
    lightSource.param('numPhotons', 10)
    lightSource.param('diameter', 10.0e-3)
    lightSource.param('alpha', 60.0)
    lightSource.param('wavelength', 405.0)
    lightSource.param('x', x)
    lightSource.param('y', -2.7)
    lightSource.param('z', -131)
    lightSource.param('phi', 0.0)
    lightSource.param('theta', 163.0)
    lightSource.param('psi', 0.0)
    path.add_module(lightSource)


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

# Optical calibration system
for barID in range(16):
    for i in range(9):
        x = (i - 4) * 5.0
        fiber(x, barID + 1)

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
