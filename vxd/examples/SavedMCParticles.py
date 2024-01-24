#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

b2.logging.log_level = b2.LogLevel.WARNING


class CheckMCParticles(b2.Module):

    """
    Counts MCParticles that generate TrueHits.
    """

    def __init__(self):
        """Initialize the module"""

        super().__init__()
        #: Number of secondaries that generated a PXDTrueHit
        self.nSecondariesPXD = 0
        #: Number of secondaries that generated an SVD TrueHit
        self.nSecondariesSVD = 0
        #: Total number of MCParticles
        self.nMCParticles = 0
        #: Total number of secondary MCParticles
        self.nSecondaries = 0
        #: List of processes that generated secondaries in PXD
        self.processesPXD = []
        #: List of processes that generated secondaries in SVD
        self.processesSVD = []

    def initialize(self):
        """ Does nothing """

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """
        Count the number of MCParticles related to a VXD TrueHit
        """

        mc_particles = Belle2.PyStoreArray('MCParticles')
        self.nMCParticles += mc_particles.getEntries()
        for particle in mc_particles:
            if not particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                self.nSecondaries += 1
                if (len(particle.getRelationsTo('PXDTrueHits')) > 0):
                    self.nSecondariesPXD += 1
                    self.processesPXD\
                        .append(particle.getSecondaryPhysicsProcess())
                if (len(particle.getRelationsTo('SVDTrueHits')) > 0):
                    self.nSecondariesSVD += 1
                    self.processesSVD\
                        .append(particle.getSecondaryPhysicsProcess())

    def terminate(self):
        """ Write results """
        b2.B2INFO(f'Found {self.nSecondaries} secondary MC Particles out of total {self.nMCParticles}.')
        b2.B2INFO(f'Of these, found {self.nSecondariesPXD} secondaries in PXD and {self.nSecondariesSVD} in SVD.')
        b2.B2INFO(f'Secondary processes for PXD: {str(self.processesPXD)}; for SVD: {str(self.processesSVD)}')


# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
# Show progress of processing
progress = b2.register_module('Progress')
# Load parameters
gearbox = b2.register_module('Gearbox')
# Create geometry
geometry = b2.register_module('Geometry')
# Run simulation
simulation = b2.register_module('FullSim')
# simulation.param('StoreAllSecondaries', True)
# PXD digitization module
printParticles = CheckMCParticles()
printParticles.set_log_level(b2.LogLevel.INFO)

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [2, 1],
    'phiGeneration': 'normal',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 1],
    'yVertexParams': [0, 1],
    'zVertexParams': [0, 1],
    'independentVertices': False,
})

# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

# create processing path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(printParticles)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
