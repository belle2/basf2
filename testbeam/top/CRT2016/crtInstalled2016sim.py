#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# CRT tests 2016 for installed modules

# Trigger paddles' parameters
# ----------------------------------------------------------------------------------
slot = 4  # slot number of the module closest to upper paddle
z0 = -0.64  # paddles z position [cm]
sigmaTrig = 2.5  # trigger paddles time resolution [ns]

# note: paddle sizes are enlarged to accomodate for multiple scattering in KLM/ECL
upperPad = [z0, 0, 50, 30, 93]   # upper trig. paddle: z, x, size_z, size_x, y [cm]
lowerPad = [z0, 0, 50, 30, -43]  # lower trig. paddle: z, x, size_z, size_x, y [cm]
alpha = (slot - 4.5) * 360.0 / 16.0  # paddle rotation angle [deg]
swimBack = 500  # swim back a muon by this distance [cm]
startTime = 0  # time at crossing the upper paddle [ns]
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
set_log_level(LogLevel.WARNING)

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
cosmicgun.param('alpha', alpha)
cosmicgun.param('swimBackDistance', swimBack)
cosmicgun.param('startTime', startTime)
cosmicgun.param('momentumDistributionType', 'polyline')
cosmicgun.param('momentumPolyline', polyline)
cosmicgun.param('momentumCutOff', 1.5)
main.add_module(cosmicgun)

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/top/CRT2016/TOPinBelle2.xml')
par = [("/DetectorComponent[@name='TOP']//TDC/numWindows", '16', ''),
       ("/DetectorComponent[@name='TriggerPaddles']//CRTexp1/psi", str(-alpha), 'deg'),
       ("/DetectorComponent[@name='TriggerPaddles']//CRTexp1/z", str(z0), 'cm'),
       ]
gearbox.param('override', par)
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP', 'ECL', 'BKLM', 'TriggerPaddles'])
main.add_module(geometry)

# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

# Trigger simulation
trigger = register_module('TOPbeamTrigger')
trigger.param('detectorIDs', [1, 2])  # upper, lower paddle
trigger.param('thresholds', [2.0, 2.0])  # in MeV
main.add_module(trigger)
emptyPath = create_path()
trigger.if_false(emptyPath)

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('timeZeroJitter', sigmaTrig)
main.add_module(TOPdigi)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'crtInstalled2016sim.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
