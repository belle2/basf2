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

pxd_rawhits_pack_unpack_collection = "PXDRawHits_test"
pxd_rawhits_pack_unpack_collection_adc = pxd_rawhits_pack_unpack_collection + "_adc"
pxd_rawhits_pack_unpack_collection_pedestal = pxd_rawhits_pack_unpack_collection + "_pedestal"
pxd_rawhits_pack_unpack_collection_roi = pxd_rawhits_pack_unpack_collection + "_roi"
set_random_seed(42)


class PxdPackerUnpackerTestModule(Module):

    """
    module which checks if the collection of PXDDigits from the simulation and
    the PXDRawHits from the packing/unpacking procedure are equal.
    The Unpackr does not create PXDDigits but PXDRawHits and therefore these two lists
    must be compared in both parameters they have in common
    """

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
        pxdRawHitsPackedUnpacked_unsorted = Belle2.PyStoreArray(pxd_rawhits_pack_unpack_collection)
        # direct from simulation
        pxdDigits_unsorted = Belle2.PyStoreArray("PXDDigits")

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
simulation.add_simulation(main, components=['PXD'])

main.add_module(progress)

packer = register_module('PXDPacker')
# [[dhhc1, dhh1, dhh2, dhh3, dhh4, dhh5] [ ... ]]
# -1 is disable port
packer.param('dhe_to_dhc', [
    [0, 2, 4, 34, 36, 38],
    [1, 6, 8, 40, 42, 44],
    [2, 10, 12, 46, 48, 50],
    [3, 14, 16, 52, 54, 56],
    [4, 3, 5, 35, 37, 39],
    [5, 7, 9, 41, 43, 45],
    [6, 11, 13, 47, 49, 51],
    [7, 15, 17, 53, 55, 57],
])


main.add_module(packer)

unpacker = register_module('PXDUnpacker')
unpacker.param("IgnoreDATCON", True)
unpacker.param("PXDRawHitsName", pxd_rawhits_pack_unpack_collection)
unpacker.param("PXDRawAdcsName", pxd_rawhits_pack_unpack_collection_adc)
unpacker.param("PXDRawPedestalsName", pxd_rawhits_pack_unpack_collection_pedestal)
unpacker.param("PXDRawROIsName", pxd_rawhits_pack_unpack_collection_roi)
unpacker.param("RemapFlag", False)
main.add_module(unpacker)

# run custom test module to check if the PXDDigits and the
# pxd_digits_pack_unpack_collection collections are equal
main.add_module(PxdPackerUnpackerTestModule())

# Process events
process(main)
