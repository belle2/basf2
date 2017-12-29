#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
import os
import numpy


set_random_seed(12345)


class PackerUnpackerTest(Module):

    """
    module which ckecks if two collections of ARICHDigits are equal
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
                x.getModuleID(),
                x.getChannelID())
        )

    def event(self):
        """
        load original ARICHDigits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        digits = Belle2.PyStoreArray("ARICHDigits")
        # processed by packer and unpacker
        digitsUnpacked = Belle2.PyStoreArray("ARICHDigitsUnpacked")

        # sort digits
        digits_sorted = self.sortDigits(digits)
        digitsUnpacked_sorted = self.sortDigits(digitsUnpacked)

        # check the sizes
        if not len(digits_sorted) == len(digitsUnpacked_sorted):
            B2FATAL("ARICHDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        for i in range(len(digits_sorted)):

            digit = digits_sorted[i]
            digitUnpacked = digitsUnpacked_sorted[i]

            # check the content of the digit
            assert digit.getModuleID() == digitUnpacked.getModuleID()
            assert digit.getChannelID() == digitUnpacked.getChannelID()
            assert digit.getBitmap() == digitUnpacked.getBitmap()


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [20.0, 25.0])

main.add_module(particlegun)

add_simulation(main, components=['ARICH'])

Packer = register_module('ARICHPacker')
main.add_module(Packer)

unPacker = register_module('ARICHUnpacker')
unPacker.param('outputDigitsName', 'ARICHDigitsUnpacked')
main.add_module(unPacker)

main.add_module(PackerUnpackerTest())

progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
