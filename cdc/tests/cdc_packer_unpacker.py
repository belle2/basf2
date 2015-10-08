#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file which shows all usage options for the particle gun module
# in the generators package. The generated particles from the particle gun are
# then fed through a full Geant4 simulation and the output is stored in a root
# file.
#
# The different options for the particle gun are explained below.
# Uncomment/comment different lines to get the wanted settings
#
# Example steering file - 2012 Belle II Collaboration
##############################################################################

from basf2 import *
from ROOT import Belle2
import os

import simulation

cdc_mapping_file = "cdc/examples/ch_map.dat"
cdc_hits_filtered = "CDCHits_filtered"
cdc_hits_pack_unpack_collection = "CDCHits_test_output"
set_random_seed(42)


class PackerUnpackerTestModule(Module):

    """
    module which checks if two collection of CDCHits are equal
    """

    def event(self):
        # load the cdcHits and the collection which results from the packer and unpacker
        # processed by packer and unpacker
        cdcHitsPackedUnpacked = Belle2.PyStoreArray(cdc_hits_pack_unpack_collection)
        # direct from simulation
        cdcHits = Belle2.PyStoreArray(cdc_hits_filtered)

        if not len(cdcHits) == len(cdcHitsPackedUnpacked):
            B2FATAL("CDC Hit count not equal after packing and unpacking")

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
            assert hit.getTDCCount2ndHit() == hitPackedUnpacked.getTDCCount2ndHit()
            assert hit.getStatus() == hitPackedUnpacked.getStatus()
        pass

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])

# limit the range of the particle gun to a 20 degree segment
# because only this range is covered by the mapping file
# cdc/examples/ch_map.dat yet.
# This limiation can be removed as soon as the mapping files contains all
# super layers
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 20])

particlegun.param('nTracks', 10)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
# Show progress of processing
progress = register_module('Progress')

main = create_path()
# init path
main.add_module(eventinfosetter)
main.add_module(particlegun)
# add simulation for CDC only
simulation.add_simulation(main, components=['CDC'])

main.add_module(progress)

# the range of super layers/layers/wire, which are listed
# in the cdc/examples/ch_map.dat mapping file yet.
# This limiation can be removed as soon as the mapping files contains all
# super layers
filter_module = register_module('CDCHitFilter')
filter_module.param("inputCDCHitListName", "CDCHits")
filter_module.param("outputCDCHitListName", cdc_hits_filtered)
filter_module.param("filterSuperLayer", 4)
filter_module.param("filterLayerMax", 5)
filter_module.param("filterWireMax", 15)
main.add_module(filter_module)

# add the packer which packs the CDCHits resulting from the simulation
cdc_packer = register_module('CDCPacker')
cdc_packer.param('xmlMapFileName', cdc_mapping_file)
cdc_packer.param('cdcHitName', "CDCHits_filtered")
main.add_module(cdc_packer)

# add the unpacker which unpacks the RawCDC hits and stores
# them in the dedicated store array "cdcHit_test_output"
cdc_unpacker = register_module('CDCUnpacker')
cdc_unpacker.param('xmlMapFileName', cdc_mapping_file)
cdc_unpacker.param('cdcHitName', cdc_hits_pack_unpack_collection)
cdc_unpacker.param('enablePrintOut', False)
main.add_module(cdc_unpacker)

# run custom test module to check if the CDCHits and the
# cdcHit_test_output collections are equal
main.add_module(PackerUnpackerTestModule())

# Process events
process(main)
