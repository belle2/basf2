#!/usr/bin/env python3

import basf2 as b2
from ROOT import Belle2


class AccessingDataStoreModule(b2.Module):
    """An example of a basf2 module in python which accesses things in the datastore."""

    def initialize(self):
        """Create a member to access event info and the MCParticles
        StoreArray
        """
        #: an example object from the datastore (the metadata collection for the event)
        self.eventinfo = Belle2.PyStoreObj("EventMetaData")
        #: an example array from the datastore (the list of MC particles)
        self.particles = Belle2.PyStoreArray("MCParticles")

    def event(self):
        """Print the event number and the PDG code of the MCParticle"""
        for particle in self.particles:
            b2.B2INFO(
                f"We're in event {self.eventinfo.getEvent()} "
                f"and have a particle with PDG code {particle.getPDG()}"
            )


# create a path
main = b2.Path()

# generate events
main.add_module("EventInfoSetter", evtNumList=[10])

# the ParticleGun generates simple tracks
main.add_module("ParticleGun")

# and add our module
main.add_module(AccessingDataStoreModule())

# run the path
b2.process(main)
