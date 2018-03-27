#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
from conditions_db import require_database_for_test

# Make sure database is available for the test
require_database_for_test()

set_random_seed(321)
set_log_level(LogLevel.INFO)


class PackerUnpackerTest(Module):

    """
    module which ckecks if two collections of BKLMDigits are equal
    """

    def sortDigits(self, unsortedPyStoreArray):
        """
        Use some digit information to sort the digits
        Returns a python-list containing the sorted digits
        """

        # first convert to a python-list to be able to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.isForward(),
                x.getSector(),
                x.getLayer(),
                x.getStrip())
        )

    def event(self):
        """
        load original BKLMDigits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        digits = Belle2.PyStoreArray("BKLMDigits")
        # processed by packer and unpacker
        digitsUnpacked = Belle2.PyStoreArray("BKLMDigitsUnpacked")

        # sort digits
        digits_sorted = self.sortDigits(digits)
        digitsUnpacked_sorted = self.sortDigits(digitsUnpacked)

        # check the sizes
        if not len(digits_sorted) == len(digitsUnpacked_sorted):
            B2FATAL("BKLMDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        for i in range(len(digits_sorted)):

            digit = digits_sorted[i]
            digitUnpacked = digitsUnpacked_sorted[i]

            # check the content of the digit
            # the following content is not consistent due to incomplete mapping file,  data/geometry/BKLMElectronicsMapping.xml
            assert digit.isForward() == digitUnpacked.isForward()
            assert digit.getSector() == digitUnpacked.getSector()
            assert digit.getLayer() == digitUnpacked.getLayer()
            assert digit.getStrip() == digitUnpacked.getStrip()
            assert digit.isPhiReadout() == digitUnpacked.isPhiReadout()
            assert digit.inRPC() == digitUnpacked.inRPC()
            assert digit.isForward() == digitUnpacked.isForward()
            assert digit.getCTime() == digitUnpacked.getCTime()
            assert digit.getCharge() == digitUnpacked.getCharge()
            assert digit.isAboveThreshold() == digitUnpacked.isAboveThreshold()
            # assert digit.getModuleID() == digitUnpacked.getModuleID()
            # assert digit.getTime() == digitUnpacked.getTime()
            # assert digit.getNPixel() == digitUnpacked.getNPixel()
            # assert digit.getEDep() == digitUnpacked.getEDep()


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [500], 'runList': [1]})
main.add_module(eventinfosetter)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
particlegun.param('momentumParams', [0.5, 4.0])
main.add_module(particlegun)

add_simulation(main, components=['BKLM'])

Packer = register_module('BKLMRawPacker')
Packer.param("loadMapFromDB", 1)
main.add_module(Packer)

unPacker = register_module('BKLMUnpacker')
unPacker.param('outputDigitsName', 'BKLMDigitsUnpacked')
# unPacker.param("useDefaultModuleId", 1)
unPacker.param("loadMapFromDB", 1)
main.add_module(unPacker)

main.add_module(PackerUnpackerTest())

progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
