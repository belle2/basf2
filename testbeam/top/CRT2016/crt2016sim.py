#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# CRT tests 2016

# module position: z = 0 at bar-prism boundary, y = 0 bar central plane.

# specify here the position and sizes of rectangular surfaces cosmic ray has to cross,
# time when the upper surface is crossed and time resolutions
# (numbers below are arbitrary!)
# ----------------------------------------------------------------------------------
# upperPad = [14.7, 0, 25, 30, 37]  # upper trig. paddle: z, x, size_z, size_x, y [cm]
# lowerPad = [14.7, 0, 25, 25, -63]  # lower trig. paddle: z, x, size_z, size_x, y [cm]
upperPad = [69.5, 0, 25, 30, 37]  # upper trig. paddle: z, x, size_z, size_x, y [cm]
lowerPad = [69.5, 0, 25, 25, -63]  # lower trig. paddle: z, x, size_z, size_x, y [cm]
# upperPad = [242.5, 0, 25, 30, 37]  # upper trig. paddle: z, x, size_z, size_x, y [cm]
# lowerPad = [242.5, 0, 25, 25, -63]  # lower trig. paddle: z, x, size_z, size_x, y [cm]
startTime = 0  # time at crossing the upperPad [ns]
sigmaAsic = 1.5  # electronic time resolution [ns]
sigmaTrig = 2.5  # trigger paddles time resolution [ns]
# ----------------------------------------------------------------------------------


def momDistribution(p):
    '''
    momentum distribution
    '''
    if p == 0:
        return 0
    else:
        return pow((5. / p), (1. + 0.04 * (p - 5.)))

# Construct polyline describing momentum distribution
n = 500
p = [i / 10.0 for i in range(n)]
y = [momDistribution(p[i]) for i in range(n)]
for i in range(n):
    p.append(y[i])
polyline = p


# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# cosmic gun
cosmicgun = register_module('TOPCosmicGun')
cosmicgun.param('upperPad', upperPad)
cosmicgun.param('lowerPad', lowerPad)
cosmicgun.param('startTime', startTime)
cosmicgun.param('momentumDistributionType', 'polyline')
cosmicgun.param('momentumPolyline', polyline)
# cosmicgun.param('momentumCutOff', 1.0)
main.add_module(cosmicgun)

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('override', [("/DetectorComponent[@name='TOP']//Nbar", '1', ''),
                           ("/DetectorComponent[@name='TOP']//Phi0", '-90', 'deg'),
                           ("/DetectorComponent[@name='TOP']//QZBackward", '0.0', 'cm'),
                           ("/DetectorComponent[@name='TOP']//Bars/Radius", '-1.0', 'cm'),
                           ("/DetectorComponent[@name='TOP']//numWindows", '16', ''),
                           ])
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', sigmaAsic)
TOPdigi.param('timeZeroJitter', sigmaTrig)
main.add_module(TOPdigi)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'crt2016sim.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
