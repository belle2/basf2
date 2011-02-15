#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')

# Create geometry
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')

# Simulation
pGun = register_module('PGunInput')
g4sim = register_module('FullSim')
mcparticle = register_module('PrintMCParticles')

# Digitizer and tracking in the CDC
svddigi = register_module('SVDDigi')

test = register_module('Test')
output = register_module('SimpleOutput')

# Parameter

evtmetagen.param('EvtNumList', [1])

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

param_svd = {'InputColName': 'SVDSimHits', 'OutputColName': 'SVDHits'}
svddigi.param(param_svd)

# Create paths
main = create_path()

# Add modules to paths

main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mcparticle)
# main.add_module(svddigi)
main.add_module(test)
main.add_module(output)

# Process events
process(main)

