#!/usr/bin/env python3


import basf2 as b2
from ROOT import Belle2


class AccessingDataStoreModule(b2.Module):
    def initialize(self):
        """Create a member to access event info and the MCParticles
        StoreArray
        """
        self.eventinfo = Belle2.PyStoreObj("EventMetaData")
        self.particles = Belle2.PyStoreArray("MCParticles")

    def event(self):
        """Print the number of charged particles and the total charge"""
        n_charged = 0
        total_charge = 0
        for particle in self.particles:
            charge = particle.getCharge()
            if charge:
                n_charged += 1
            total_charge += charge

        b2.B2INFO(
            f"Number of charged particles = {n_charged}, "
            f"total charge of event = {total_charge}"
        )


# create a path
main = b2.Path()

# generate events
main.add_module("EventInfoSetter", evtNumList=[10])

# generate events with 3 tracks (not all of them are charged tracks)
main.add_module("ParticleGun", nTracks=3)

# and add our module
main.add_module(AccessingDataStoreModule())

# run the path
b2.process(main)
