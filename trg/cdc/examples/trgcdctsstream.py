#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import os

# suppress messages and warnings during processing:
# set_log_level(LogLevel.ERROR)
# Register modules
particlegun = b2.register_module('ParticleGun')
# particlegun.param('randomSeed', 3452346)
# particlegun.param('randomSeed', 345)
particlegun.param('randomSeed', 346)
# The particle we are shooting
particlegun.param('pdgCodes', [11])
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'uniformPt')
# particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1.0, 1.0])
particlegun.param('thetaGeneration', 'uniform')
# particlegun.param('thetaParams', [35, 127])
particlegun.param('thetaParams', [45, 45])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [0, 0.0])
particlegun.param('yVertexParams', [0, 0.0])
particlegun.param('zVertexParams', [0, 0.0])

# Register modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfoprinter = b2.register_module('Progress')
# eventinfoprinter = fw.register_module("EventInfoPrinter")
paramloader = b2.register_module('Gearbox')
geobuilder = b2.register_module('Geometry')
# geobuilder.log_level = LogLevel.INFO
g4sim = b2.register_module('FullSim')
cdcdigitizer = b2.register_module('CDCDigitizer')
out = b2.register_module('SimpleOutput')
cdctrg = b2.register_module("TRGCDC")
tsstream = b2.register_module("TRGCDCTSStream")
# mcparticle  = fw.register_module('PrintMCParticles')

# ...EventInfoSetter...
eventinfosetter.param({'EvtNumList': [10], 'RunList': [1]})

# ...CDC Trigger...
cdctrg.param('ConfigFile',
             os.path.join(basf2datadir,
                          "trg/TRGCDCConfig_0_20101111_1051.dat"))
cdctrg.param('DebugLevel', 0)
cdctrg.param('CurlBackStop', 1)
cdctrg.param('HoughFinderPerfect', 1)

# ...CDC Trigger TS Stream...
tsstream.param('DebugLevel', 2)
tsstream.param('Mode', 0)
tsstream.param('OutputStreamFile', "TRGCDCTSStream.dat")


# set mcprinter
mcparticleprinter = b2.register_module('PrintMCParticles')
mcparticleprinter.param('maxLevel', -1)

# set geometry(geobuilder)
geobuilder.param('Components', ['MagneticField', 'CDC'
                                ])

# set digitizer to no smearing
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.00, 'Resolution2': 0.0}
cdcdigitizer.param(param_cdcdigi)

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(particlegun)
main.add_module(mcparticleprinter)
main.add_module(g4sim)
main.add_module(cdcdigitizer)
main.add_module(cdctrg)
main.add_module(tsstream)
# main.add_module(out)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
