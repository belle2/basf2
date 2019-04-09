#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
import os
import numpy

set_random_seed(321)
set_log_level(LogLevel.INFO)


class PackerUnpackerTest(Module):

    """
    module which checks if two collections of EKLMDigits are equal
    """

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
                x.isForward(),
                x.getSector(),
                x.getLayer(),
                x.getStrip())
        )

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
                x.getEndcap(),
                x.getLayer(),
                x.getSector(),
                x.getPlane(),
                x.getStrip())
        )

    def event(self):
        """
        load original EKLMDigits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        bklm_digits = Belle2.PyStoreArray("BKLMDigits")
        eklm_digits = Belle2.PyStoreArray("EKLMDigits")
        # processed by packer and unpacker
        bklm_digits_unpacked = Belle2.PyStoreArray("EKLMDigitsUnpacked")
        eklm_digits_unpacked = Belle2.PyStoreArray("EKLMDigitsUnpacked")

        # sort digits
        bklm_digits_sorted = self.sortDigits(bklm_digits)
        bklm_digits_unpacked_sorted = self.sortDigits(bklm_digits_unpacked)
        eklm_digits_sorted = self.sortDigits(eklm_digits)
        eklm_digits_unpacked_sorted = self.sortDigits(eklm_digits_unpacked)

        # check the sizes
        if not len(bklm_digits_sorted) == len(bklm_digits_unpacked_sorted):
            B2FATAL("BKLMDigits: size not equal after packing and unpacking")
        if not len(eklm_digits_sorted) == len(eklm_digits_unpacked_sorted):
            B2FATAL("EKLMDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        for i in range(len(bklm_digits_sorted)):

            digit = bklm_digits_sorted[i]
            digit_unpacked = bklm_digits_unpacked_sorted[i]

            # check the content of the digit
            # the following content is not consistent due to incomplete mapping file,  data/geometry/BKLMElectronicsMapping.xml
            assert digit.isForward() == digit_unpacked.isForward()
            assert digit.getSector() == digit_unpacked.getSector()
            assert digit.getLayer() == digit_unpacked.getLayer()
            assert digit.getStrip() == digit_unpacked.getStrip()
            assert digit.isPhiReadout() == digit_unpacked.isPhiReadout()
            assert digit.inRPC() == digit_unpacked.inRPC()
            assert digit.isForward() == digit_unpacked.isForward()
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
            assert digit.getEndcap() == digit_unpacked.getEndcap()
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


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [50]})
main.add_module(eventinfosetter)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
particlegun.param('momentumParams', [0.5, 4.0])
main.add_module(particlegun)

add_simulation(main, components=['BKLM', 'EKLM'])
set_module_parameters(main, type='Geometry', useDB=False,
                      components=['BKLM', 'EKLM'])

bklm_packer = register_module('BKLMRawPacker')
eklm_packer = register_module('EKLMRawPacker')
main.add_module(bklm_packer)
main.add_module(eklm_packer)

unpacker = register_module('KLMUnpacker')
unpacker.param('outputDigitsName', 'BKLMDigitsUnpacked')
unpacker.param('outputDigitsName', 'EKLMDigitsUnpacked')
main.add_module(unpacker)

main.add_module(PackerUnpackerTest())

progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
