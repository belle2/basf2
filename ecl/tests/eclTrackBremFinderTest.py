#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from basf2 import *
from ROOT import Belle2
import numpy

import simulation
import reconstruction
from rawdata import add_packers

set_random_seed(42)


class AddBremClusterTestModule(Module):

    """
    Module which checks if a collection of PXDDigits and
    a collection of PXDRawHits from the packing/unpacking procedure are equal.
    The PXDUnpacker does not create PXDDigits but PXDRawHits and therefore these two lists
    must be compared.
    """

    def __init__(self):
        """constructor"""
        # call constructor of base class, required if you implement __init__ yourself!
        super().__init__()
        # and do whatever else is necessary like declaring member variables
        # self.rawhits_collection = rawhits_collection
        # self.digits_collection = digits_collection

    def event(self):
        """ load the PXD Digits of the simulation and the packed/unpacked ones
        and compare them"""

        # eclClusters = Belle2.PyStoreArray("ECLClusters")
        # eclClusters.

        pass


# to run the framework the used modules need to be registered
# limit the involved components so the unit test can run faster
components = ['MagneticField', 'CDC', 'ECL']

main = create_path()

main.add_module('ParticleGun', pdgCodes=[13, -13], nTracks=1,
                momentumGeneration='fixed', momentumParams=0.3,
                thetaGeneration='fixed', thetaParams=95,
                phiGeneration='fixed', phiParams=30)

# Create Event information
main.add_module('EventInfoSetter')  # ,evtNumList=[1], 'runList': [1]})
simulation.add_simulation(main, components=components)
reconstruction.add_reconstruction(main, components=components)

main.add_module("PruneDataStore", matchEntries="ECLClusters", keepMatchedEntries=False)

main.add_module("ECLTrackBremFinder")
# run custom test module 2nd time to check if the collection pxd_rawhits_pack_unpack_collection_digits
# and the pxd_digits_pack_unpack_collection collections are equal
# main.add_module(
#    PxdPackerUnpackerTestModule(
#        rawhits_collection=pxd_rawhits_pack_unpack_collection,
#        digits_collection=pxd_rawhits_pack_unpack_collection_digits))

# Process events
process(main)
