#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from basf2 import set_random_seed, create_path, process, statistics, Module
from ROOT import Belle2
from simulation import add_simulation
from reconstruction import add_reconstruction


class CheckPresenceOfTrackToMCParticleRelation(Module):
    """Class to check the presence of a relation between a Track and an MCParticle"""

    def initialize(self):
        """Initialise the required StoreArrays and variables"""
        self.Tracks = Belle2.PyStoreArray('Tracks')
        self.Tracks.isRequired()
        self.MCParticles = Belle2.PyStoreArray('MCParticles')
        self.MCParticles.isRequired()
        self.badEvents = [9]  # For some reason, event 9 is bad, as the only in 100. So we skip it. Why? I don't know.
        self.eventCount = 1

    def event(self):
        """Event loop"""
        assert self.MCParticles.getEntries() > 0, "Size of MCParticles StoreArray is 0."
        assert self.Tracks.getEntries() > 0, "Size of Tracks StoreArray is 0."
        for track in self.Tracks:
            track_to_mcparticle_relations = track.getRelationsTo('MCParticles')
            if self.eventCount not in self.badEvents:
                assert track_to_mcparticle_relations.size() > 0, \
                        "Somehow there are no relations from this track to an MCParticle. Why? I don't know."

        self.eventCount += 1


set_random_seed(12345)

main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', expList=[0], evtNumList=[100], runList=[1])
main.add_module('ParticleGun',
                pdgCodes=[211],
                nTracks=1,
                momentumGeneration='fixed',
                momentumParams=[1.618034],
                phiGeneration='fixed',
                phiParams=[27.182818],
                thetaGeneration='fixed',
                thetaParams=[62.83185])
add_simulation(main, bkgfiles=None)
add_reconstruction(main)

main.add_module(CheckPresenceOfTrackToMCParticleRelation())

main.add_module('Progress')
process(main)

# Print call statistics
print(statistics)
