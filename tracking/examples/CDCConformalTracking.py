#!/usr/bin/env python
# -*- coding: utf-8 -*-
##########################################################
# This steering file creates the Belle II detector geometry,
# perfoms the simulation and pattern recognition in the CDC.
##########################################################

import os
from basf2 import *

# Register modules

evtmetagen = register_module('EvtMetaGen')

# Create geometry
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')

# Simulation
pGun = register_module('PGunInput')
g4sim = register_module('FullSim')
mcparticle = register_module('PrintMCParticles')

# Digitizer and tracking in the CDC
cdcdigitizer = register_module('CDCDigi')
cdctracking = register_module('CDCTracking')
# output = register_module("SimpleOutput")

# Set parameters

param_evtmetagen = {
    'ExpList': [1],
    'RunList': [1],
    'EvtStartList': [0],
    'EvtEndList': [0],
    }
evtmetagen.param(param_evtmetagen)

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

param_pGun = {
    'Rseed': 2392567,
    'ntracks': 3,
    'p_par1': 0.9,
    'p_par2': 1.1,
    }

pGun.param(param_pGun)

param_cdctracking = {
    'InputSimHitsColName': 'CDCSimHitArray',
    'InputHitsColName': 'HitCDCArray',
    'OutputTracksColName': 'TrackCDCArray',
    'TextFileOutput': 0,
    }
cdctracking.param(param_cdctracking)

# Create paths

main = create_path()

# Add modules to paths

main.add_module(evtmetagen)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mcparticle)
main.add_module(cdcdigitizer)
main.add_module(cdctracking)
# main.add_module(output)

# Process events

process(main)
