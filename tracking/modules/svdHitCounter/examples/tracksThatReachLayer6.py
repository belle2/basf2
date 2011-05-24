#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file generates MCParticles, performs
# a full Geant4 simulation and saves the result to disk
# and tests which layers of the SVD were hit by the particles
########################################################

import os
import random
from basf2 import *

# Set the log level to show everything
set_log_level(LogLevel.INFO)

tracks = 100
pGSeed = 0
g4Seed = 0

# EvtMetaGen - generate event meta data
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [1])  # ONLY TESTED WITH NUMBER OF EVENTS = 1

# Particle gun
particlegun = register_module('PGunInput')
particlegun.param('nTracks', tracks)
particlegun.param('PIDcodes', [211, -211])
particlegun.param('pPar1', 0.04)  # min p in ?? GeV ??
particlegun.param('pPar2', 0.5)  # max p in ?? GeV ??
particlegun.param('phiPar1', 0)  # min phi in °
particlegun.param('phiPar2', 360)  # max phi in °
particlegun.param('thetaPar1', 5)  # min theta in °
particlegun.param('thetaPar2', 175)  # max theta in °
particlegun.param('xVertexPar1', 0)  # min x in cm
particlegun.param('xVertexPar2', 0)  # max x in cm
particlegun.param('yVertexPar1', 0)  # min y in cm
particlegun.param('yVertexPar2', 0)  # max y in cm
particlegun.param('yVertexPar1', 0)  # min z in cm
particlegun.param('yVertexPar2', 0)  # max z in cm

particlegun.param('Rseed', pGSeed)

# Geometry parameter loader
paramloader = register_module('ParamLoaderXML')
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# Geometry builder
geobuilder = register_module('GeoBuilder')

# Full Geant4 simulation
g4sim = register_module('FullSim')

# Test cheatTrackFinder
# myModule       = register_module("my")

svdHits = register_module('svdHitCounter')
svdHits.param('tracks', tracks)

svdHits.param('outputFileName', 'testpt')
svdHits.param('svdLayerId', 6)
# svdHits.param(  'option' , 'notEven')

# Root file output
# simpleoutput = register_module('SimpleOutput')
# simpleoutput.param('outputFileName', 'cdc0wtf.root')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(svdHits)
# main.add_module(myModule)
# main.add_module(simpleoutput)

# Process 100 events
process(main)
