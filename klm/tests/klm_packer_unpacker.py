#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
import simulation as sim
from ROOT import Belle2

basf2.set_random_seed(321)
basf2.set_log_level(basf2.LogLevel.INFO)


class PackerUnpackerTest(basf2.Module):

    """
    module which checks if two collections of EKLMDigits and BKLMDigits are
    equal
    """

    def sort_bklm_digits(self, unsortedPyStoreArray):
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
                x.getSection(),
                x.getSector(),
                x.getLayer(),
                x.getStrip())
        )

    def sort_eklm_digits(self, unsortedPyStoreArray):
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
                x.getSection(),
                x.getLayer(),
                x.getSector(),
                x.getPlane(),
                x.getStrip())
        )

    def event(self):
        """
        load original digits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        bklm_digits = Belle2.PyStoreArray("BKLMDigits")
        eklm_digits = Belle2.PyStoreArray("EKLMDigits")
        # processed by packer and unpacker
        bklm_digits_unpacked = Belle2.PyStoreArray("BKLMDigitsUnpacked")
        eklm_digits_unpacked = Belle2.PyStoreArray("EKLMDigitsUnpacked")

        # sort digits
        bklm_digits_sorted = self.sort_bklm_digits(bklm_digits)
        bklm_digits_unpacked_sorted = self.sort_bklm_digits(bklm_digits_unpacked)
        eklm_digits_sorted = self.sort_eklm_digits(eklm_digits)
        eklm_digits_unpacked_sorted = self.sort_eklm_digits(eklm_digits_unpacked)

        # check the sizes
        if not len(bklm_digits_sorted) == len(bklm_digits_unpacked_sorted):
            basf2.B2FATAL("BKLMDigits: size not equal after packing and unpacking")
        if not len(eklm_digits_sorted) == len(eklm_digits_unpacked_sorted):
            basf2.B2FATAL("EKLMDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        for i in range(len(bklm_digits_sorted)):

            digit = bklm_digits_sorted[i]
            digit_unpacked = bklm_digits_unpacked_sorted[i]

            # check the content of the digit
            # the following content is not consistent due to incomplete mapping file,  data/geometry/BKLMElectronicsMapping.xml
            assert digit.getSection() == digit_unpacked.getSection()
            assert digit.getSector() == digit_unpacked.getSector()
            assert digit.getLayer() == digit_unpacked.getLayer()
            assert digit.getStrip() == digit_unpacked.getStrip()
            assert digit.isPhiReadout() == digit_unpacked.isPhiReadout()
            assert digit.inRPC() == digit_unpacked.inRPC()
            assert digit.getCTime() == digit_unpacked.getCTime()
            assert digit.getCharge() == digit_unpacked.getCharge()
            assert digit.isAboveThreshold() == digit_unpacked.isAboveThreshold()
            # assert digit.getModuleID() == digit_unpacked.getModuleID()
            # assert digit.getTime() == digit_unpacked.getTime()
            # assert digit.getNPixel() == digit_unpacked.getNPixel()
            # assert digit.getEDep() == digit_unpacked.getEDep()

        for i in range(len(eklm_digits_sorted)):

            digit = eklm_digits_sorted[i]
            digit_unpacked = eklm_digits_unpacked_sorted[i]

            # check the content of the digit
            # From EKLMHitBase
            assert digit.getSection() == digit_unpacked.getSection()
            assert digit.getLayer() == digit_unpacked.getLayer()
            assert digit.getSector() == digit_unpacked.getSector()
            # assert digit.getEDep() == digit_unpacked.getEDep()
            # assert digit.getTime() == digit_unpacked.getTime()
            # From EKLMDigit
            assert digit.getPlane() == digit_unpacked.getPlane()
            assert digit.getStrip() == digit_unpacked.getStrip()
            assert digit.getCharge() == digit_unpacked.getCharge()
            assert digit.getCTime() == digit_unpacked.getCTime()
            assert digit.getFitStatus() == digit_unpacked.getFitStatus()


main = basf2.create_path()

eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', 50)
main.add_module(eventinfosetter)

particlegun = basf2.register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
particlegun.param('momentumParams', [0.5, 4.0])
main.add_module(particlegun)

sim.add_simulation(main, components=['KLM'])
basf2.set_module_parameters(main, type='Geometry', useDB=False, components=['KLM'])

bklm_packer = basf2.register_module('BKLMRawPacker')
eklm_packer = basf2.register_module('EKLMRawPacker')
main.add_module(bklm_packer)
main.add_module(eklm_packer)

unpacker = basf2.register_module('KLMUnpacker')
unpacker.param('outputBKLMDigitsName', 'BKLMDigitsUnpacked')
unpacker.param('outputEKLMDigitsName', 'EKLMDigitsUnpacked')
main.add_module(unpacker)

main.add_module(PackerUnpackerTest())

progress = basf2.register_module('Progress')
main.add_module(progress)

basf2.process(main)
print(basf2.statistics)
