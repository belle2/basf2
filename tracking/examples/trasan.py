#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *

# ...Particle Gun...
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13])
particlegun.param('nTracks', 10)
particlegun.param('momentumGeneration', 'uniformPt')
# particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.1, 5.0])
particlegun.param('thetaGeneration', 'uniform')
# particlegun.param('thetaParams', [35, 127])
particlegun.param('thetaParams', [45, 45])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
# particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [0, 0.00])
particlegun.param('yVertexParams', [0, 0.00])
particlegun.param('zVertexParams', [0, 0.00])

# ...G4Sim...
g4sim = register_module('FullSim')
# Turn off physics processes "physics.mac" is located at "trg/examples/" or
# "tracking/examples" g4sim.param('UICommands',['/control/execute
# physics.mac'])
#
# ...EvtMetaGen...
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'EvtNumList': [10], 'RunList': [1]})

# ...GeoBuilder...
geobuilder = register_module('Geometry')
# Exclude detectors other than CDC
geobuilder.param('Components', ['MagneticField', 'CDC'])

# ...MCPrinter...
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.param('maxLevel', -1)

# ...CDC Digitizer...
cdcdigitizer = register_module('CDCDigitizer')
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.00, 'Resolution2': 0.00}
cdcdigitizer.param(param_cdcdigi)

# ...Trasan...
trasan = register_module('Trasan')
trasan.param('DebugLevel', 10)
trasan.param('GFTrackCandidatesColName', 'GFTrackCands_Trasan')

# ...Other modules...
evtmetainfo = register_module('Progress')
paramloader = register_module('Gearbox')

# ...Path...
main = create_path()
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(particlegun)
main.add_module(mcparticleprinter)
main.add_module(g4sim)
main.add_module(cdcdigitizer)
main.add_module(trasan)

# ...Process events...
process(main)

# ...Print call statistics...
print statistics
