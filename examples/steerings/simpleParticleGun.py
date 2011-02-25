#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file generates MCParticles via the parti-
# cle gun, performs a full Geant4 simulation and saves
# the result in a root file.
# In total 100 events are processed.
#
# The following parameters are used for the particle gun:
#  Tracks per event:      10
#  Particles:             electrons / pions / protons (and anti particles)
#  Theta [default]:       17 to 150 degree
#  Phi [default]:         0 to 360 degree
#  Momentum:              200 MeV to 1 GeV
#
# For more details on the options for the particle gun
# look at the example steering file in the generators
# package.
# For more details on the options for the simulation look
# at the example steering file in the simulation package.
#
# Example steering file - 2011 Belle II Collaboration
########################################################

from basf2 import *

set_log_level(4)

# Register modules
evtmetagen = register_module('EvtMetaGen')
pguninput = register_module('PGunInput')
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# setting infos for evtmetagen module
# Here we can set how many events should be processed per
# run or experiment.
# For the current setting (MC), only 1 number is needed.
# Note: this parameter is a list (brackets), to allow for different runs
# and experiments with a different number of events to be processed
evtmetagen.param('EvtNumList', [100])
# see the example for EvtMetaDataGen in the framework package
# for more details.

# preparing the particle gun
# get the random seed from a random number, so you don't have to update
# it for every job.
import random
intseed = random.randint(1, 10000000)

## Particle Gun Parameters
pguninput.param('nTracks', 10)
pguninput.param('PIDcodes', [
    11,
    -11,
    211,
    -211,
    2212,
    -2212,
    ])
pguninput.param('pPar1', 0.200)
pguninput.param('pPar2', 1)
pguninput.param('Rseed', intseed)
# for random generation of angles and vertex coordinates the standard
# settings for the particle gun are used.

# change verbosity for the particle gun only
pguninput.set_log_level(2)

# Set parameters for paramloader:
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# setting verbosity for simulation (5 very high, 0 low)
# this sets how verbose the Geant4 simulation is
g4sim.param('TrackingVerbosity', 0)

# Set parameters for simpleoutput:
# simpleoutput creates a simple root files where a tree structure gives
# access to all containers in the data store at the moment
# the module is called
simpleoutput.param('outputFileName', 'simpleParticleGun_output.root')

# the path sets the order of the module execution
# Create paths
main = create_path()
main.add_module(evtmetagen)  # this module coordinates the event loop
main.add_module(pguninput)  # generate particles for simulation
main.add_module(paramloader)  # get the geometry parameters
main.add_module(geobuilder)  # build the geometry
main.add_module(g4sim)  # run the simulation
main.add_module(simpleoutput)  # put the information from datastore to root file

# Process events
process(main)

