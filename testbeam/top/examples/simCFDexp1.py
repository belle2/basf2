#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import math

# LEPS beam test 2013-june using 2.1 GeV/c e+ beam: runs with CFD electronics

# beam definition for exp001 (cosTheta=0.0), new alignment with basf2 (2013/9/5)
xBeam = 0  # cm
yBeam = 0.0  # cm
zBeam = 124.8  # cm
thetaBeam = 90.2  # degrees
phiBeam = 0.0  # degress
shiftAlongBeam = 49.0  # cm

# beam spreads (half width of uniform distributions)
dtheta = 20e-3  # rad
dphi = 20e-3  # rad
dz = 0.25  # cm
dy = 0.25  # cm

# conversions
theta = math.radians(thetaBeam)
phi = math.radians(phiBeam)
x0 = xBeam + shiftAlongBeam * math.cos(phi) * math.sin(theta)
y0 = yBeam + shiftAlongBeam * math.sin(phi) * math.sin(theta)
z0 = zBeam + shiftAlongBeam * math.cos(theta)
dtheta = math.degrees(dtheta)
dphi = math.degrees(dphi)

beam = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'uniformCosinus',
    'thetaParams': [thetaBeam - dtheta, thetaBeam + dtheta],
    'phiGeneration': 'uniform',
    'phiParams': [phiBeam - dphi, phiBeam + dphi],
    'vertexGeneration': 'uniform',
    'xVertexParams': [x0, x0],
    'yVertexParams': [y0 - dy, y0 + dy],
    'zVertexParams': [z0 - dz, z0 + dz],
    }

outfile = 'LEPS-CFDbeamtest01.root'
dbFile = 'testbeam/top/LEPS2013-6/TOP-CFD.xml'
tracker = 'TriggerTelescopeExp1'
electronicJitter = 35e-3  # ns
timeZeroJitter = 25e-3  # ns

# -------------------------------------------------------------------------

# Number of events to generate
eventinfo = register_module('EventInfoSetter')
eventinfo.param({'evtNumList': [100], 'runList': [1]})

# particle gun
particlegun = register_module('ParticleGun')
particlegun.param(beam)
print_params(particlegun)

# Show progress of processing
progress = register_module('Progress')

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', dbFile)

# Geometry
geometry = register_module('Geometry')
geometry.param('Components', ['TOP', tracker])

# Simulation
simulation = register_module('FullSim')
simulation.param('SecondariesEnergyCut', 0.5)
simulation.param('StoreAllSecondaries', True)

# Trigger
trigger = register_module('TOPbeamTrigger')
trigger.param('detectorIDs', [1, 2, 7, 8])
trigger.param('thresholds', [0.5, 0.5, 0.5, 0.5])
emptyPath = create_path()
trigger.if_false(emptyPath)

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', electronicJitter)
TOPdigi.param('timeZeroJitter', timeZeroJitter)

# Output
output = register_module('RootOutput')
output.param('outputFileName', outfile)

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()
main.add_module(eventinfo)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(trigger)
main.add_module(TOPdigi)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
