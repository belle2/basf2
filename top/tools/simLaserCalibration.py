#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# gb2_setuprel = 'build-2014-10-22'

# ---------------------------------------------------------------
# Simulation of laser calibration system of TOP
# ---------------------------------------------------------------


# Create path
main = create_path()

# Define optical source


def fiber(x, angle, barID=1, path=main):
    source = register_module('OpticalGun')
    source.param('alpha', 60.0)
    source.param('na', 0.50)
    source.param('startTime', 0)
    source.param('pulseWidth', 10.0e-3)
    source.param('numPhotons', 10)
    source.param('diameter', 10.0e-3)
    source.param('barID', barID)  # if nonzero, local (bar) frame, otherwise Belle II
    source.param('x', x)
    source.param('y', -3.26)
    source.param('z', -131.33)
    source.param('theta', 180 + angle)
    source.param('phi', 0.0)
    source.param('psi', 0.0)
    source.param('angularDistribution', 'Gaussian')
    path.add_module(source)


# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Optical sources
for barId in range(16):
    for pos in [
        0.9,
        5.7,
        11.3,
        16.9,
        22.5,
        28.1,
        33.7,
        39.3,
        44.1,
    ]:
        angle = 17
        x = -45. / 2. + pos
        fiber(x, angle, barId + 1, main)

# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

# TOP digitization
topdigi = register_module('TOPDigitizer')
main.add_module(topdigi)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'laserSimulation.root')
output.param('branchNames', ['TOPDigits', 'TOPSimHits', 'TOPSimPhotons',
                             'TOPDigitsToTOPSimHits', 'TOPSimHitsToTOPSimPhotons'])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
