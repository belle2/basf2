#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Disabling this test for the time being as it fails the incremental builds for
# a long time now which gives no additional information to anyone.
# See https://agira.desy.de/browse/BII-1647
# FIXME: remove once packer is fixed
import sys
# print("TEST SKIPPED: Test fails due to changes in packer which were not propagated to unpacker. See BII-1647", file=sys.stderr)
# sys.exit(1)

from basf2 import *
from ROOT import Belle2
import numpy

import simulation
from rawdata import add_packers

pxd_rawhits_pack_unpack_collection = "PXDRawHits_test"
pxd_rawhits_pack_unpack_collection_digits = "PXDDigits_test"
pxd_rawhits_pack_unpack_collection_adc = pxd_rawhits_pack_unpack_collection + "_adc"
pxd_rawhits_pack_unpack_collection_pedestal = pxd_rawhits_pack_unpack_collection + "_pedestal"
pxd_rawhits_pack_unpack_collection_roi = pxd_rawhits_pack_unpack_collection + "_roi"
set_random_seed(42)


class PxdPackerUnpackerTestModule(Module):

    """
    Module which checks if a collection of PXDDigits and
    a collection of PXDRawHits from the packing/unpacking procedure are equal.
    The PXDUnpacker does not create PXDDigits but PXDRawHits and therefore these two lists
    must be compared.
    """

    def __init__(self, rawhits_collection='PXDRawHits', digits_collection="PXDDigits"):
        """constructor"""
        # call constructor of base class, required if you implement __init__ yourself!
        super().__init__()
        # and do whatever else is necessary like declaring member variables
        self.rawhits_collection = rawhits_collection
        self.digits_collection = digits_collection

    def sortDigits(self, unsortedPyStoreArray):
        """ use a some digit information to sort the PXDDigits list
            Returns a python-list containing the PXDDigts
        """

        # first convert to a python-list to be abple to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getSensorID(),
                x.getVCellID(),
                x.getUCellID()))

    def sortRawHits(self, unsortedPyStoreArray):
        """ use a some digit information to sort the PXDRawHits list
            Returns a python-list containing the PXDRawHits
        """

        # first convert to a python-list to be able to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getSensorID(),
                x.getRow(),
                x.getColumn()))

    def event(self):
        """ load the PXD Digits of the simulation and the packed/unpacked ones
        and compare them"""

        # load the digits and the collection which results from the packer and unpacker
        # processed by packer and unpacker
        pxdRawHitsPackedUnpacked_unsorted = Belle2.PyStoreArray(self.rawhits_collection)
        # direct from simulation
        pxdDigits_unsorted = Belle2.PyStoreArray(self.digits_collection)

        # sort the digits, because the order gets
        # lost during the packing/unpacking process
        pxdDigits = self.sortDigits(pxdDigits_unsorted)
        pxdRawHitsPackedUnpacked = self.sortRawHits(pxdRawHitsPackedUnpacked_unsorted)

        if not len(pxdDigits) == len(pxdRawHitsPackedUnpacked):
            B2FATAL("PXDDigits and PXDRawHits count not equal after packing and unpacking")

        print("Comparing %i pxd digits " % len(pxdDigits))

        # check all quantities between the direct and the packed/unpacked pxd digits
        for i in range(len(pxdDigits)):
            digit = pxdDigits[i]
            rawHitPackedUnpacked = pxdRawHitsPackedUnpacked[i]

            # compare all available quantities
            # cannot compare frame number, because it is not availabl on PXDDigits
            assert rawHitPackedUnpacked.getSensorID().getID() == digit.getSensorID().getID()
            assert rawHitPackedUnpacked.getRow() == digit.getVCellID()
            assert rawHitPackedUnpacked.getColumn() == digit.getUCellID()
            # There are some rare cases (~ every 10th event), where the PXD Digits have a charge
            # larger than 255 which will be clipped by the packer to 8bit (at most 255)
            # therefor, limit the maximal charge of the digit here in the comparison
            assert numpy.isclose(min(255.0, digit.getCharge()), rawHitPackedUnpacked.getCharge())


# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
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
# add simulation for pxd only
simulation.add_simulation(main, components=['PXD'], usePXDDataReduction=False)

main.add_module(progress)

# create raw data
add_packers(main, components=['PXD'])

# unpack raw data. creates collection PXDRawHits_test
pxdunpacker = register_module('PXDUnpacker')
pxdunpacker.param('HeaderEndianSwap', True)
pxdunpacker.param("PXDRawHitsName", pxd_rawhits_pack_unpack_collection)
pxdunpacker.param("PXDRawAdcsName", pxd_rawhits_pack_unpack_collection_adc)
pxdunpacker.param("PXDRawPedestalsName", pxd_rawhits_pack_unpack_collection_pedestal)
pxdunpacker.param("PXDRawROIsName", pxd_rawhits_pack_unpack_collection_roi)
main.add_module(pxdunpacker)

# sort rawhits. creates collection PXDDigits_test
pxdhitsorter = register_module('PXDRawHitSorter')
pxdhitsorter.param('mergeFrames', True)
pxdhitsorter.param('rawHits', pxd_rawhits_pack_unpack_collection)
pxdhitsorter.param('digits', pxd_rawhits_pack_unpack_collection_digits)
main.add_module(pxdhitsorter)


# run custom test module to check if the simulated PXDDigits and the
# pxd_digits_pack_unpack_collection collections are equal
main.add_module(PxdPackerUnpackerTestModule(rawhits_collection=pxd_rawhits_pack_unpack_collection, digits_collection="PXDDigits"))


# run custom test module 2nd time to check if the collection pxd_rawhits_pack_unpack_collection_digits
# and the pxd_digits_pack_unpack_collection collections are equal
main.add_module(
    PxdPackerUnpackerTestModule(
        rawhits_collection=pxd_rawhits_pack_unpack_collection,
        digits_collection=pxd_rawhits_pack_unpack_collection_digits))


# Process events
process(main)
