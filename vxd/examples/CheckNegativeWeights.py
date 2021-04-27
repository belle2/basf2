#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
import svd
import pxd

b2.logging.log_level = b2.LogLevel.WARNING


class CheckNegativeWeights(b2.Module):

    """
    Lists signs of MCParticle relation weights for VXD Clusters based on
    MCParticle and TrueHit information.
    Breakdown of data:
    MCParticle primary/secondary/remapped/none (no relation to MCParticle)
    Relation sign positive/negative
    """

    def __init__(self):
        """Initialize the module"""

        super(CheckNegativeWeights, self).__init__()
        #: Relation sign statistics for PXDClusters
        self.sign_stats_pxd = {
            'primary': {'positive': 0, 'negative': 0},
            'secondary': {'positive': 0, 'negative': 0},
            'remapped': {'positive': 0, 'negative': 0},
            'none': {'positive': 0, 'negative': 0}
        }
        #: Relation sign statistics for PXDClusters
        self.sign_stats_svd = {
            'primary': {'positive': 0, 'negative': 0},
            'secondary': {'positive': 0, 'negative': 0},
            'remapped': {'positive': 0, 'negative': 0},
            'none': {'positive': 0, 'negative': 0}
        }

    def initialize(self):
        """ Does nothing """

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """
        Goes through event's PXD and SVD clusters and looks at MCParticles
        and TrueHits relations for the sign of relations.
        """
        # PXD part -------------------------------------------------------
        pxd_clusters = Belle2.PyStoreArray('PXDClusters')

        for cluster in pxd_clusters:
            # Determine MCParticle tag and MCParticle relation weight sign
            mcparticle_tag = 'none'
            sign_tag = 'positive'  # convention if there is no relation
            # From MCParticles we can determine if the particle is primary
            # or secondary, and we get the sign of the weight.
            # To determine if a MCParticle is remapped, we need TrueHit
            # and the sign of its relation to the MCParticle.
            mcparticle_relations = cluster.getRelationsTo('MCParticles')
            n_mcparticle_relations = mcparticle_relations.size()
            for mcparticle_index in range(n_mcparticle_relations):
                mcparticle = mcparticle_relations[mcparticle_index]
                mcparticle_weight = \
                    mcparticle_relations.weight(mcparticle_index)
                if mcparticle_weight < 0:
                    sign_tag = 'negative'
                if mcparticle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                    mcparticle_tag = 'primary'
                    # The primary particle may be remapped. Check TrueHits!
                    cluster_truehits = cluster.getRelationsTo('PXDTrueHits')
                    # Identify the TrueHit related to the current MCParticle.
                    mcparticle_array_index = mcparticle.getArrayIndex()
                    for hit in cluster_truehits:
                        hit_particle_relations = \
                            hit.getRelationsFrom('MCParticles')
                        n_relations = hit_particle_relations.size()
                        for particle_index in range(n_relations):
                            hit_particle = \
                                hit_particle_relations[particle_index]
                            if hit_particle.getArrayIndex() == \
                                    mcparticle_array_index:
                                # check sign of the weight
                                weight = hit_particle_relations.weight(
                                    particle_index)
                                if weight < 0:
                                    mcparticle_tag = 'remapped'
                                break
                else:
                    mcparticle_tag = 'secondary'
                # That's it, store the result
                # If there are more MCParticles per cluster (possible), we
                # make an entry for each.
                self.sign_stats_pxd[mcparticle_tag][sign_tag] += 1

        # SVD part -------------------------------------------------------
        svd_clusters = Belle2.PyStoreArray('SVDClusters')

        for cluster in svd_clusters:
            # Determine MCParticle tag and MCParticle relation weight sign
            mcparticle_tag = 'none'
            sign_tag = 'positive'  # convention if there is no relation
            # From MCParticles we can determine if the particle is primary
            # or secondary, and we get the sign of the weight.
            # To determine if a MCParticle is remapped, we need TrueHit
            # and the sign of its relation to the MCParticle.
            mcparticle_relations = cluster.getRelationsTo('MCParticles')
            n_mcparticle_relations = mcparticle_relations.size()
            for mcparticle_index in range(n_mcparticle_relations):
                mcparticle = mcparticle_relations[mcparticle_index]
                mcparticle_weight = \
                    mcparticle_relations.weight(mcparticle_index)
                if mcparticle_weight < 0:
                    sign_tag = 'negative'
                if mcparticle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                    mcparticle_tag = 'primary'
                    # The primary particle may be remapped. Check TrueHits!
                    cluster_truehits = cluster.getRelationsTo('SVDTrueHits')
                    # Identify the TrueHit related to the current MCParticle.
                    mcparticle_array_index = mcparticle.getArrayIndex()
                    for hit in cluster_truehits:
                        hit_particle_relations = \
                            hit.getRelationsFrom('MCParticles')
                        n_relations = hit_particle_relations.size()
                        for particle_index in range(n_relations):
                            hit_particle = \
                                hit_particle_relations[particle_index]
                            if hit_particle.getArrayIndex() == \
                                    mcparticle_array_index:
                                # check sign of the weight
                                weight = hit_particle_relations.weight(
                                    particle_index)
                                if weight < 0:
                                    mcparticle_tag = 'remapped'
                                break
                else:
                    mcparticle_tag = 'secondary'
                # That's it, store the result
                self.sign_stats_svd[mcparticle_tag][sign_tag] += 1

    def terminate(self):
        """ Write results """
        b2.B2INFO(
            '\nResults for PXD: \n{pxd}\nResults for SVD: \n{svd}\n'.format(
                pxd=str(self.sign_stats_pxd),
                svd=str(self.sign_stats_svd)
            )
        )


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
printWeights = CheckNegativeWeights()
printWeights.set_log_level(b2.LogLevel.INFO)

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

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


# create processing path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
pxd.add_pxd_simulation(main)
svd.add_svd_simulation(main)
pxd.add_pxd_reconstruction(main)
svd.add_svd_reconstruction(main)
main.add_module(printWeights)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
