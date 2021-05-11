#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2
from svd import add_svd_simulation
from ROOT import Belle2
import numpy


svd_digits_pack_unpack_collection = "SVDShaperDigits_test"
b2.set_random_seed(42)


class SvdPackerUnpackerTestModule(b2.Module):

    """
    module which ckecks if two collection of SVDShaperDigits are equal
    """

    def sortDigits(self, unsortedPyStoreArray):
        """ use some digit information to sort the SVDShaperDigits list
            Returns a python-list containing the SVDShaperDigits
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
        """ load SVDShaperDigits of the simulation and the packed/unpacked ones
        and compare them"""

        svdDigitsPackedUnpacked = Belle2.PyStoreArray(svd_digits_pack_unpack_collection)
        # direct from simulation
        svdDigits = Belle2.PyStoreArray("SVDShaperDigits")

        svdDigits_sorted = self.sortDigits(svdDigits)
        svdDigitsPackedUnpacked_sorted = self.sortDigits(svdDigitsPackedUnpacked)

        if not len(svdDigits_sorted) == len(svdDigitsPackedUnpacked_sorted):
            b2.B2FATAL("SVDShaperDigits count not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked svd digits
        for i in range(len(svdDigits_sorted)):

            # sort the digits, because the packer sorts the
            # the 32-bit frames are sorted by FADC numbers by the packer module
            hit = svdDigits_sorted[i]
            hitPackedUnpacked = svdDigitsPackedUnpacked_sorted[i]

            # check content of the digit
            assert hit.getTime() == hitPackedUnpacked.getTime()
            assert hit.getIndex() == hitPackedUnpacked.getIndex()

            # note: The charge will only be checked if between 0 and 255
            # These two cases will be addressed in future SVDShaperDigits and packer/unpacker version
            assert numpy.isclose(hit.getCharge(), hitPackedUnpacked.getCharge())

            # check the VxdID information
            assert hit.getSensorID().getID() == hitPackedUnpacked.getSensorID().getID()
            assert hit.getSensorID().getLayerNumber() == hitPackedUnpacked.getSensorID().getLayerNumber()
            assert hit.getSensorID().getLadderNumber() == hitPackedUnpacked.getSensorID().getLadderNumber()
            assert hit.getSensorID().getSensorNumber() == hitPackedUnpacked.getSensorID().getSensorNumber()
            assert hit.getSensorID().getSegmentNumber() == hitPackedUnpacked.getSensorID().getSegmentNumber()


# to run the framework the used modules need to be registered
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)

# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
# Show progress of processing
progress = b2.register_module('Progress')

main = b2.create_path()
# init path
main.add_module(eventinfosetter)
main.add_module(particlegun)
# add simulation for svd only
add_svd_simulation(main)

main.add_module(progress)

nodeid = 0
Packer = b2.register_module('SVDPacker')
Packer.param('NodeID', nodeid)
Packer.param('svdShaperDigitListName', 'SVDShaperDigits')
Packer.param('rawSVDListName', 'SVDRaw')
main.add_module(Packer)

unPacker = b2.register_module('SVDUnpacker')
unPacker.param('rawSVDListName', 'SVDRaw')
unPacker.param('svdShaperDigitListName', svd_digits_pack_unpack_collection)
main.add_module(unPacker)

# run custom test module to check if the SVDShaperDigits and the
# svd_digits_pack_unpack_collection collections are equal
main.add_module(SvdPackerUnpackerTestModule())

# Process events
b2.process(main)
