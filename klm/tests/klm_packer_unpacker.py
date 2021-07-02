#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

    def sort_klm_digits(self, unsortedPyStoreArray):
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
                x.getSubdetector(),
                x.getSection(),
                x.getSector(),
                x.getLayer(),
                x.getPlane(),
                x.getStrip())
        )

    def event(self):
        """
        load original digits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        klm_digits = Belle2.PyStoreArray("KLMDigits")
        # processed by packer and unpacker
        klm_digits_unpacked = Belle2.PyStoreArray("KLMDigitsUnpacked")

        # sort digits
        klm_digits_sorted = self.sort_klm_digits(klm_digits)
        klm_digits_unpacked_sorted = self.sort_klm_digits(klm_digits_unpacked)

        # check the size
        if not len(klm_digits_sorted) == len(klm_digits_unpacked_sorted):
            basf2.B2FATAL("KLMDigits: size not equal after packing and unpacking")
        # check all quantities between the direct and the packed/unpacked
        for i in range(len(klm_digits_sorted)):

            digit = klm_digits_sorted[i]
            digit_unpacked = klm_digits_unpacked_sorted[i]

            # check the content of the digit
            assert digit.getSubdetector() == digit_unpacked.getSubdetector()
            assert digit.getSection() == digit_unpacked.getSection()
            assert digit.getSector() == digit_unpacked.getSector()
            assert digit.getLayer() == digit_unpacked.getLayer()
            assert digit.getPlane() == digit_unpacked.getPlane()
            assert digit.getStrip() == digit_unpacked.getStrip()
            assert digit.inRPC() == digit_unpacked.inRPC()
            assert digit.getCharge() == digit_unpacked.getCharge()
            assert digit.getCTime() == digit_unpacked.getCTime()
            assert digit.getTDC() == digit_unpacked.getTDC()
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

klm_packer = basf2.register_module('KLMPacker')
main.add_module(klm_packer)

unpacker = basf2.register_module('KLMUnpacker')
unpacker.param('outputKLMDigitsName', 'KLMDigitsUnpacked')
main.add_module(unpacker)

main.add_module(PackerUnpackerTest())

progress = basf2.register_module('Progress')
main.add_module(progress)

basf2.process(main)
print(basf2.statistics)
