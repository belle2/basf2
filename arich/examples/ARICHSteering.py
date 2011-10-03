#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.INFO

# Uncomment the line below to set the environment variable to automatically start the VRML viewer
# for example http://freewrl.sourceforge.net/
# os.environ['G4VRMLFILE_VIEWER'] = 'freewrl'

# -------------------------
# here we register modules
# -------------------------

# creates event meta data
evtmetagen = register_module('EvtMetaGen')
# Load XML parameters
paramloader = register_module('Gearbox')
# Create Geometry
geobuilder = register_module('Geometry')
# Particle gun
pguninput = register_module('PGunInput')
# Simulation module
g4sim = register_module('FullSim')
# ARICH digitizer module
arichDigi = register_module('ARICHDigi')
# ARICH reconstruction module
arichRec = register_module('ARICHRec')
# Module to save the data in DataStore into root file
# output = register_module('SimpleOutput')

# --------------------------------------
# here we set the parameters of modules
# --------------------------------------

# set number of events in each run
evtmetagen.param('EvtNumList', [2])
# set number of runs
evtmetagen.param('RunList', [1])

# This line is necessary if you want to simulate Cerenkov photons!
# By default optical processes are not registered.
g4sim.param('RegisterOptics', 1)
# To speed up the simulation you can propagate only a selected fraction of photons.
# By default all photons are propagated.
g4sim.param('PhotonFraction', 0.3)
# Set up the visualization
g4sim.param('EnableVisualization', True)
# Here you can select visualization driver and visualization commands.
# This creates VRML file, change VRML2FILE to HepRepFile to create HepRep file.
g4sim.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
            '/vis/scene/add/axes 0 0 0 100 mm',
            '/vis/scene/add/trajectories smooth',
            '/vis/modeling/trajectories/create/drawByCharge'])
# short explanation of above commands:
# g4sim.param('UICommands', [
## Use VRML2 backend
    # '/vis/open VRML2FILE',
## Draw the geometry
    # '/vis/drawVolume',
## Draw coordinate axes at the origin with a length of 100mm in each direction
    # '/vis/scene/add/axes 0 0 0 100 mm',
## Draw simulated tracks
    # '/vis/scene/add/trajectories smooth',
    # '/vis/modeling/trajectories/create/drawByCharge'
## Uncomment the following two lines to have yellow dots at each step boundary along the trajectory
##    '/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true',
##    '/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 0.5',
# ])

# Parameters for particle gun
# number of tracks
pguninput.param('nTracks', 1)
# Type of particles (PDG codes)
pguninput.param('PIDcodes', [321])
# Particle momentum (pPar1 < p < pPar2)
pguninput.param('pPar1', 4)
pguninput.param('pPar2', 5)
# theta angle
pguninput.param('thetaPar1', 20)
pguninput.param('thetaPar2', 25)
# Many more parameters for PGun are available...
# pguninput.param('xVertexPar1',0)
# pguninput.param('xVertexPar2',8)
# pguninput.param('phiPar1', 0)
# pguninput.param('phiPar2', 0)
# output.param('outputFileName', 'arich.root')

# create path
main = create_path()

# add modules to path
main.add_module(evtmetagen)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pguninput)
main.add_module(g4sim)
main.add_module(arichDigi)
main.add_module(arichRec)
# main.add_module(output)

process(main)
