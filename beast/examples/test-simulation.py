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

# Don't show all the messages :)
b2.set_log_level(b2.LogLevel.ERROR)

# Register modules

# lets generate empty events
eventinfosetter = b2.register_module('EventInfoSetter')
# and shoot some particles into them
particlegun = b2.register_module('ParticleGun')
# load the simulation parameters
gearbox = b2.register_module('Gearbox')
# build the microtpc geometry
geometry = b2.register_module('Geometry')
# simulate our microtpc detector
simulation = b2.register_module('FullSim')
# do something with the microtpc data
# analysis = register_module('Csi')
# analysis = register_module('He3tube')
# analysis = register_module('Bgo')
# analysis = register_module('Pindiode')
# analysis = register_module('Srsensor')
# analysis = register_module('Microtpc')
# analysis = register_module('FANGS')
analysis = b2.register_module('CLAWS')
# save the microtpc results
output = b2.register_module('RootOutput')
# an show some progress of the microtpc simulation
progress = b2.register_module('Progress')

# Now lets set some parameters ...

# Generate run 1 with 500 events
eventinfosetter.param({'evtNumList': [500], 'runList': [1]})

# Set the parameters for the particle gun
particlegun.param({  # Shoot electrons and positrons
                     # 5 particles per event
                     # but let the number be poisson distributed
                     # with a fixed momentum
                     # of 7 GeV
                     # and a gaussian distributed theta angle
                     # with mean 0 degree and width 1 degree
                     # and a uniform distributed phi angle
                     # between 0 and 360 degree
                     # but from a fixed position
                     # namely 0,0,0
                     # and the same vertex vor all particles
    'pdgCodes': [11, 22, -11],
    'nTracks': 1000,
    'varyNTracks': True,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.00004],
    'thetaGeneration': 'normal',
    'thetaParams': [0.0, 1.0],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360.0],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.0],
    'yVertexParams': [0.0],
    'zVertexParams': [0.0],
    'independentVertices': False,
})

# Main XML parameter file to load, relative to global data directory
# gearbox.param('fileName', 'beast/microtpc/detector.xml')
# gearbox.param('fileName', 'beast/fangs/detector.xml')
gearbox.param('fileName', 'beast/claws/detector.xml')
# gearbox.param('fileName', 'beast/he3tube/detector.xml')
# gearbox.param('fileName', 'beast/bgo/detector.xml')
# gearbox.param('fileName', 'beast/pindiode/detector.xml')
# gearbox.param('fileName', 'beast/srsensor/detector.xml')
# gearbox.param('fileName', 'beast/plume/detector.xml')
# gearbox.param('fileName', 'beast/csi/detector.xml')
# Lets see some more information on geometry building
geometry.set_log_level(b2.LogLevel.INFO)

# and also on our own module
analysis.set_log_level(b2.LogLevel.INFO)

# And write the results to microtpc-simulation.root
# output.param('outputFileName', 'microtpc-simulation.root')
# output.param('outputFileName', 'fangs-simulation.root')
output.param('outputFileName', 'claws-simulation.root')
# output.param('outputFileName', 'plume-simulation.root')
# output.param('outputFileName', 'csi-simulation.root')
# output.param('outputFileName', 'he3tube-simulation.root')
# output.param('outputFileName', 'bgo-simulation.root')
# output.param('outputFileName', 'pindiode-simulation.root')
# output.param('outputFileName', 'srsensor-simulation.root')
output.param('updateFileCatalog', False)

# Here we create a processing path and add the modules
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(analysis)
main.add_module(output)
main.add_module(progress)

# Now lets do the processing of the microtpc events
b2.process(main)

# Print call statistics of our microtpc event processing
print(b2.statistics)
