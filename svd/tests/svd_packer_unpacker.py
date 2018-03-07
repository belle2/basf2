#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from svd import add_svd_simulation
from ROOT import Belle2
import os
import numpy

import simulation

svd_digits_pack_unpack_collection = "SVDDigits_test"
set_random_seed(42)


class SvdPackerUnpackerTestModule(Module):

    """
    module which ckecks if two collection of SVDDigits are equal
    """

    def sortDigits(self, unsortedPyStoreArray):
        """ use a some digit information to sort the SVDDigits list
            Returns a python-list containing the SVDDigits
        """

        # first convert to a python-list to be abple to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getSensorID().getLayerNumber(),
                x.getSensorID().getLadderNumber(),
                x.getSensorID().getSensorNumber(),
                x.isUStrip()))

    def event(self):
        """ load the SVD Digits of the simulation and the packed/unpacked ones
        and compare them"""

        # load the digits and the collection which results from the packer and unpacker
        # processed by packer and unpacker
        svdDigitsPackedUnpacked = Belle2.PyStoreArray(svd_digits_pack_unpack_collection)
        # direct from simulation
        svdDigits = Belle2.PyStoreArray("SVDDigits")

        svdDigits_sorted = self.sortDigits(svdDigits)
        svdDigitsPackedUnpacked_sorted = self.sortDigits(svdDigitsPackedUnpacked)

        if not len(svdDigits_sorted) == len(svdDigitsPackedUnpacked_sorted):
            B2FATAL("SVDDigits count not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked svd digits
        for i in range(len(svdDigits_sorted)):

            # sort the digits, because the packer sorts the
            # the 32-bit frames are sorted by FADC numbers by the packer module
            hit = svdDigits_sorted[i]
            hitPackedUnpacked = svdDigitsPackedUnpacked_sorted[i]

            # check content of the digit
            assert hit.getTime() == hitPackedUnpacked.getTime()
            assert hit.getIndex() == hitPackedUnpacked.getIndex()
            # note: not checking the getCellPosition(), because it is intentionally not set
            # by the unpacker

            # note: The charge will only be checked if between 0 and 255
            # There are some cases where the charge coming from the simulation is either
            # negative or larger than 255. In these cases, an 8-bit integer over/underflow occurs
            # during the packer/unpacker process which results in a very different charge.
            # These two cases will be addressed in future SVDDigits and packer/unpacker version
            # Once this two effects are handled, the if condition below can be removed and the
            # charge conversion can be compared independent of the quantity
            if hit.getCharge() >= 0 and hit.getCharge() <= 255:
                assert numpy.isclose(hit.getCharge(), hitPackedUnpacked.getCharge())

            # check the VxdID information
            assert hit.getSensorID().getID() == hitPackedUnpacked.getSensorID().getID()
            assert hit.getSensorID().getLayerNumber() == hitPackedUnpacked.getSensorID().getLayerNumber()
            assert hit.getSensorID().getLadderNumber() == hitPackedUnpacked.getSensorID().getLadderNumber()
            assert hit.getSensorID().getSensorNumber() == hitPackedUnpacked.getSensorID().getSensorNumber()
            assert hit.getSensorID().getSegmentNumber() == hitPackedUnpacked.getSensorID().getSegmentNumber()


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
# add simulation for svd only
add_svd_simulation(main, createDigits=True)

main.add_module(progress)

nodeid = 0
Packer = register_module('SVDPacker')
Packer.param('NodeID', nodeid)
Packer.param('svdShaperDigitListName', 'SVDShaperDigits')
Packer.param('rawSVDListName', 'SVDRaw')
main.add_module(Packer)

unPacker = register_module('SVDUnpacker')
unPacker.param('rawSVDListName', 'SVDRaw')
unPacker.param('svdDigitListName', svd_digits_pack_unpack_collection)
main.add_module(unPacker)

# run custom test module to check if the SVDDigits and the
# svd_digits_pack_unpack_collection collections are equal
main.add_module(SvdPackerUnpackerTestModule())

# Process events
process(main)
