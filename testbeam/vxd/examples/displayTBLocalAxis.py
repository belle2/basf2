#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2014 @ DESY Simulation
# This is the default simulation scenario for VXD beam test without telescopes

import os
import random
import sys
import math
from basf2 import *

# avoid race conditions beetween pyroot and GUI thread
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf
from ROOT import PyConfig
#from ROOT import EventData
PyConfig.StartGuiThread = False

from ROOT import TVector3

ShowLocal = 1
ShowCounting = 1
ShowPlane = -1

argvs = sys.argv
argc = len(argvs)
print "Number of arguments: ", argc - 1
if argc >= 2:
    ShowLocal = argvs[1]
if argc >= 3:
    ShowCounting = argvs[2]
if argc >= 4:
    ShowPlane = argvs[3]
if argc >= 5:
    print argvs[4]

GeometryFileName = 'testbeam/vxd/FullTelescopeVXDTB_v2.xml'

print "*****************************************************************"
print "first arg: Show local coordinates:                 ", ShowLocal
print "second arg: Show pixel/strip counting coordinates: ", ShowCounting
print "third arg: Show plane [Tel:1-3,PXD:4,5,SVD:6-9,Tel:10-12]:", ShowPlane
print "        plane = -1: all planes"
print "        Defaults: local = 1, counting = 1, plane = -1 (all planes)"
print "Geometry File Name:", GeometryFileName
print "*****************************************************************"

# Important parameters of the simulation:
events = 1  # Number of events to simulate
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # # degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding
# + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)


class DisplayAxis(Module):
    """Test DisplayData"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerAsPersistent()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        pos = TVector3(30, 5, 5)
        displayData.obj().addArrow("x", pos, pos + TVector3(20, 0, 0))
        displayData.obj().addArrow("y", pos, pos + TVector3(0, 20, 0))
        displayData.obj().addArrow("z", pos, pos + TVector3(0, 0, 20))
        displayData.obj().addLabel("Global axes", pos + TVector3(1, 1, 1))

        posBeam = TVector3(30, -5, -5)
        displayData.obj().addArrow("Beam direction, -x=20cm", posBeam,
                posBeam + TVector3(-20, 0, 0))

        displayData.obj().addLabel(GeometryFileName, TVector3(-20, 20, 0))

        geo = Belle2.VXD.GeoCache.getInstance()
        Plane = 1
        for sensor in geo.getListOfSensors():
            info = geo.get(sensor)
            if info.getID().getLayerNumber() == 7:
                if info.getID().getSensorNumber() < 4:
                    SensorID = 'Tel' + str(info.getID().getSensorNumber())
                    PlaneOK = info.getID().getSensorNumber()
                else:
                    SensorID = 'Tel' + str(info.getID().getSensorNumber()\
                     + 6)
                    PlaneOK = info.getID().getSensorNumber() + 6
            else:
                if info.getID().getLayerNumber() < 3:
                    SensorID = 'PXD' + str(info.getID().getLayerNumber())
                    PlaneOK = info.getID().getLayerNumber() + 3
                else:
                    SensorID = 'SVD' + str(info.getID().getLayerNumber())
                    PlaneOK = info.getID().getLayerNumber() + 3
            if int(ShowPlane) == -1 or int(ShowPlane) == PlaneOK:
                if int(ShowLocal) != 0:
                    detPos = geo.get(sensor).\
                        pointToGlobal(TVector3(0., 0., 0.))
                    u = geo.get(sensor).vectorToGlobal(TVector3(2., 0., 0.))
                    v = geo.get(sensor).vectorToGlobal(TVector3(0., 2., 0.))
                    w = geo.get(sensor).vectorToGlobal(TVector3(0., 0., 0.5))
                    LableU = SensorID + ', dir U, 2cm'
                    displayData.obj().addArrow(LableU, detPos, detPos + u, \
                        ROOT.kBlue)
                    LableV = SensorID + ', dir V, 2cm'
                    displayData.obj().addArrow(LableV, detPos, detPos + v, \
                        ROOT.kGreen)
                    LableW = SensorID + ', dir W, 0.5cm'
                    displayData.obj().addArrow(LableW, detPos, detPos + w), \
                        ROOT.kGray

                if int(ShowCounting) != 0:
                    pixelDir0 = geo.get(sensor).pointToGlobal(TVector3(\
                        info.getUCellPosition(0), \
                        info.getVCellPosition(0), 0.))
                    pixelDirU = geo.get(sensor).pointToGlobal(TVector3(\
                        info.getUCellPosition(200), \
                        info.getVCellPosition(0), 0.))
                    pixelDirV = geo.get(sensor).pointToGlobal(TVector3(\
                        info.getUCellPosition(0), \
                        info.getVCellPosition(200), 0.))
                    LableU = SensorID + ', dir U, 200 pixels, pitch ' \
                        + str(int(info.getUPitch() * 10000)) + 'um'
                    LableV = SensorID + ', dir V, 200 pixels, pitch ' \
                        + str(int(info.getVPitch() * 10000)) + 'um'
                    displayData.obj().addArrow(LableU,
                       pixelDir0, pixelDirU, ROOT.kCyan)
                    displayData.obj().addArrow(LableV, pixelDir0, \
                        pixelDirV, ROOT.kYellow)
                    print str(PlaneOK) + ':' + SensorID + ', dir U, pitch ' \
                        + str(info.getUPitch() * 10000) + 'um, size ' + \
                        str(info.getUSize() * 10) + ' mm, pixels ' + \
                        str(info.getUCells())
                    print str(PlaneOK) + ':' + SensorID + ', dir V, pitch ' + \
                        str(info.getVPitch() * 10000) + 'um, size ' + \
                        str(info.getVSize() * 10) + ' mm, pixels ' + \
                        str(info.getVCells())
            Plane += 1

# ParticleGun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 10)
# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB
# setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.0])
particlegun.param('yVertexParams', [0.0, beamspot_size_y])
particlegun.param('zVertexParams', [0.0, beamspot_size_z])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD (no Telescopes) beam test geometry including
#the real PCMAG magnetic field
gearbox.param('fileName', GeometryFileName)

# Create geometry
geometry = register_module('Geometry')
# You can specify components to be created
geometry.param('components', ['MagneticField', 'TB'])
# To turn off magnetic field:
# geometry.param('components', ['TB'])

# Full simulation module
simulation = register_module('FullSim')
simulation.logging.log_level = LogLevel.ERROR
# and make it store the particle trajectories so we can draw them
simulation.param("trajectoryStore", 2)

# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# PXD/SVD digitizer
PXDDigi = register_module('PXDDigitizer')
SVDDigi = register_module('SVDDigitizer')
# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
#PXDClust.param('ClusterCacheSize', 576)
SVDClust = register_module('SVDClusterizer')
# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulation.root')
# You can specify branch names to be saved (only), see module doc
# output.param('branchNames[0]', ['PXDTrueHits', 'SVDTrueHits'])

# Export used geometry for checking
geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.WARNING
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    }
mctrackfinder.param(param_mctrackfinder)
# mctrackfinder.logging.log_level = LogLevel.DEBUG

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('UseClusters', True)

display = register_module('Display')

# The Options parameter is a combination of:
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# M draw track markers - intersections of track with detector planes
#   (use with T)
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
# T draw track (straight line between detector planes)
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
#
# This option only makes sense when ShowGFTracks is true
display.param('options', 'HTM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
# with this option off, many tracking hits will be assigned to secondary e-
display.param('assignHitsToPrimaries', 0)

# show all primary MCParticles?
display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
display.param('showCharged', False)

# show tracks?
display.param('showTrackLevelObjects', True)

# save events non-interactively (without showing window)?
display.param('automatic', False)

# Use clusters to display tracks
display.param('useClusters', True)

# Display the testbeam geometry rather than Belle II extract
display.param('fullGeometry', True)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDigi)
main.add_module(SVDDigi)
main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(mctrackfinder)
main.add_module(geosaver)
main.add_module(trackfitter)

main.add_module(DisplayAxis())

main.add_module(display)

# Process events
process(main)

print statistics
