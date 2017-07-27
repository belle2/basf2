#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

gb2_setuprel = 'build-2017-06-14'

# Usage: basf2 t0LaserCalMC.py -n 10000

# Create path
main = create_path()

# Optical sources


def fiber(
    x,
    angle,
    barID=1,
    path=main,
):

    source1 = register_module('OpticalGun')
    source1.param('alpha', 60.0)
    source1.param('na', 0.50)
    source1.param('startTime', 0)
    source1.param('pulseWidth', 10.0e-3)
    source1.param('numPhotons', 10)
    source1.param('diameter', 10.0e-3)
    source1.param('barID', barID)  # if nonzero, local (bar) frame, otherwise Belle II
    source1.param('x', x)
    source1.param('y', -3.26)
    source1.param('z', -131.33)
    source1.param('theta', 180 + angle)
    source1.param('phi', 0.0)
    source1.param('psi', 0.0)
    source1.param('angularDistribution', 'Gaussian')
    # source1.param('angularDistribution', 'Lambertian')
    path.add_module(source1)


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
# for (pos,angle) in [(0.9,17), (5.7,15), (11.3,15), (16.9,15), (22.5,15), (28.1,15), (33.7,15), (39.3,15), (44.1,17)]:
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

# TOP Channel T0 MC
channelt0mc = register_module('TOPChannelT0MC')
channelt0mc.param('outputFile', "t0mc.root")
channelt0mc.param('fitRange', [100, 0, 1])  # fit range[nbins, xmin, xmax]
main.add_module(channelt0mc)


# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
