#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
from basf2 import *
import ROOT
from ROOT import Belle2

logging.log_level = LogLevel.WARNING


class CheckMCParticles(Module):

    """Lists MCParticles that are not related to any TrueHits. Desirably,
    there should be no such MCParticles.
    VXD::GeoCache.
     """

    def __init__(self):
        """Initialize the module"""

        super(CheckMCParticles, self).__init__()
        self.nSecondariesPXD = 0
        self.nSecondariesSVD = 0
        self.nMCParticles = 0
        self.nSecondaries = 0
        self.processesPXD = []
        self.processesSVD = []

    def initialize(self):
        """ Does nothing """

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Count the number of MCParticles that are not related to a VXD
        TrueHit
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
        B2INFO('Found {nu} secondary MC Particles out of {n}.'\
            .format(nu=self.nSecondaries, n=self.nMCParticles))
        B2INFO('Of these, found {n1} in PXD and {n2} in SVD.'\
            .format(n1=self.nSecondariesPXD, n2=self.nSecondariesSVD))
        B2INFO('Secondary processes for PXD: {list1}; for SVD: {list2}'\
            .format(list1=str(self.processesPXD), \
                    list2=str(self.processesSVD)))


# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# simulation.param('StoreAllSecondaries', True)
# PXD digitization module
printParticles = CheckMCParticles()
printParticles.set_log_level(LogLevel.INFO)

# Specify number of events to generate
evtmetagen.param({'evtNumList': [100], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [2, 1],
    'phiGeneration': 'normal',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCosinus',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 1],
    'yVertexParams': [0, 1],
    'zVertexParams': [0, 1],
    'independentVertices': False,
    })

# Select subdetectors to be built
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

# create processing path
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(printParticles)

# generate events
process(main)

# show call statistics
print statistics
