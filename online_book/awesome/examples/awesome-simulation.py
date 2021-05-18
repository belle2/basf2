#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ------------------------------------------------------------
#
# Minimal steering file to simulate the awesome detector
#
# Contributors: Martin Ritter
#               Giacomo De Pietro
#               Umberto Tamponi
#
# ------------------------------------------------------------

import basf2 as b2

# Don't show all the messages :)
b2.set_log_level(b2.LogLevel.ERROR)

# Here we create a processing path and add the modules
main = b2.create_path()

# create event meta-data
main.add_module('EventInfoSetter')

# Main XML parameter file to load, relative to global data directory
main.add_module('Gearbox', fileName='online_book/awesome/detector.xml')
main.add_module('Geometry', useDB=False)

# Shoot some particles into the detector
main.add_module('ParticleGun',
                # Shoot electrons and positrons
                pdgCodes=[11, -11],
                # 5 particles per event
                nTracks=5,
                # but let the number be poisson distributed
                varyNTracks=True,
                # with a fixed momentum
                momentumGeneration='fixed',
                # of 7 GeV
                momentumParams=[7.0],
                # and a gaussian distributed theta angle
                thetaGeneration='normal',
                # with mean 0 degree and width 1 degree
                thetaParams=[0.0, 1.0],
                # and a uniform distributed phi angle
                phiGeneration='uniform',
                # between 0 and 360 degree
                phiParams=[0, 360.0],
                # but from a fixed position
                vertexGeneration='fixed',
                # namely 0,0,0
                xVertexParams=[0.0],
                yVertexParams=[0.0],
                zVertexParams=[0.0],
                # and the same vertex vor all particles
                independentVertices=False)

# Simulate our awesome detector
main.add_module('FullSim')

# do something with the awesome data
main.add_module('AWESOMEBasic',
                logLevel=b2.LogLevel.INFO)

# save the awesome results
main.add_module('RootOutput', outputFileName='awesome-simulation.root')


# Now lets do the processing of the awesome events
b2.process(main)

# Print call statistics of our awesome event processing
print(b2.statistics)
