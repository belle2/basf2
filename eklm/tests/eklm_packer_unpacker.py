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
    module which ckecks if two collections of EKLMDigits are equal
    """

    def sortDigits(self, unsortedPyStoreArray):
        """
        Use some digit information to sort the digits
        Returns a python-list containing the sorted digits
        """

        # first convert to a python-list to be able to sort
        py_list = [x for x in unsortedPyStoreArray if x.isGood()]

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
        digits = Belle2.PyStoreArray("EKLMDigits")
        # processed by packer and unpacker
        digitsUnpacked = Belle2.PyStoreArray("EKLMDigitsUnpacked")

        # sort digits
        digits_sorted = self.sortDigits(digits)
        digitsUnpacked_sorted = self.sortDigits(digitsUnpacked)

        # check the sizes
        if not len(digits_sorted) == len(digitsUnpacked_sorted):
            B2FATAL("EKLMDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        for i in range(len(digits_sorted)):

            digit = digits_sorted[i]
            digitUnpacked = digitsUnpacked_sorted[i]

            # check the content of the digit
            # From EKLMHitBase
            assert digit.getEndcap() == digitUnpacked.getEndcap()
            assert digit.getLayer() == digitUnpacked.getLayer()
            assert digit.getSector() == digitUnpacked.getSector()
            # assert digit.getEDep() == digitUnpacked.getEDep()
            assert digit.getTime() == digitUnpacked.getTime()
            # From EKLMDigit
            assert digit.getPlane() == digitUnpacked.getPlane()
            assert digit.getStrip() == digitUnpacked.getStrip()
            assert digit.getCharge() == digitUnpacked.getCharge()
            assert digit.getCTime() == digitUnpacked.getCTime()
            assert digit.getFitStatus() == digitUnpacked.getFitStatus()


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [500], 'runList': [1]})
main.add_module(eventinfosetter)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
particlegun.param('momentumParams', [0.5, 4.0])
main.add_module(particlegun)

add_simulation(main, components=['EKLM'])

packer = register_module('EKLMRawPacker')
main.add_module(packer)

unpacker = register_module('EKLMUnpacker')
unpacker.param('outputDigitsName', 'EKLMDigitsUnpacked')
main.add_module(unpacker)

main.add_module(PackerUnpackerTest())

progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
