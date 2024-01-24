#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

import simulation

cdc_hits = "CDCHits"
cdc_hits_pack_unpack_collection = "CDCHits_test_output"
b2.set_random_seed(42)


class PackerUnpackerTestModule(b2.Module):

    """
    module which checks if two collection of CDCHits are equal
    """

    def sortHits(self, unsortedPyStoreArray):
        """ use a some information to sort the CDCHits list
            Returns a python-list containing the CDCHits
        """

        # first convert to a python-list to be able to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getILayer(),
                x.getISuperLayer(),
                x.getIWire()))

    def event(self):
        """
        event function
        """

        # load the cdcHits and the collection which results from the packer and unpacker
        # processed by packer and unpacker
        cdcHitsPackedUnpacked_unsorted = Belle2.PyStoreArray(cdc_hits_pack_unpack_collection)
        # direct from simulation
        cdcHits_unsorted = Belle2.PyStoreArray(cdc_hits)

        # sort the hits, because they have been shuffled in the packing/unpacking process
        cdcHitsPackedUnpacked = self.sortHits(cdcHitsPackedUnpacked_unsorted)
        cdcHits = self.sortHits(cdcHits_unsorted)

        if not len(cdcHits) == len(cdcHitsPackedUnpacked):
            b2.B2FATAL("CDC Hit count not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked CDC hit
        for i in range(len(cdcHits)):
            hit = cdcHits[i]
            hitPackedUnpacked = cdcHitsPackedUnpacked[i]

            assert hit.getILayer() == hitPackedUnpacked.getILayer()
            assert hit.getISuperLayer() == hitPackedUnpacked.getISuperLayer()
            assert hit.getIWire() == hitPackedUnpacked.getIWire()
            assert hit.getID() == hitPackedUnpacked.getID()
            assert hit.getTDCCount() == hitPackedUnpacked.getTDCCount()
            assert hit.getADCCount() == hitPackedUnpacked.getADCCount()
            assert hit.getStatus() == hitPackedUnpacked.getStatus()


main = b2.create_path()
# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10]})
main.add_module(eventinfosetter)

# to run the framework the used modules need to be registered
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
main.add_module(particlegun)

# add simulation for CDC only
simulation.add_simulation(main, components=['CDC'])
b2.set_module_parameters(main, type="Geometry", useDB=False, components=["CDC"])

# add the packer which packs the CDCHits resulting from the simulation
cdc_packer = b2.register_module('CDCPacker')
cdc_packer.param('cdcHitName', "CDCHits")
main.add_module(cdc_packer)

# add the unpacker which unpacks the RawCDC hits and stores
# them in the dedicated store array "cdcHit_test_output"
cdc_unpacker = b2.register_module('CDCUnpacker')
cdc_unpacker.param('cdcHitName', cdc_hits_pack_unpack_collection)
cdc_unpacker.param('enablePrintOut', False)
cdc_unpacker.param('pedestalSubtraction', False)
main.add_module(cdc_unpacker)

# run custom test module to check if the CDCHits and the
# cdcHit_test_output collections are equal
main.add_module(PackerUnpackerTestModule())

# Process events
b2.process(main)
