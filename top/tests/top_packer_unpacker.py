#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
import os
import numpy


set_random_seed(12345)


class DigitsTest(Module):

    """
    module which ckecks if two collections of TOPDigits are equal
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
                x.getChannel(),
                x.getRawTime())
        )

    def event(self):
        """
        load original TOPDigits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        digits = Belle2.PyStoreArray("TOPDigits")
        # processed by packer and unpacker
        digitsUnpacked = Belle2.PyStoreArray("TOPDigitsUnpacked")

        # sort digits
        digits_sorted = self.sortDigits(digits)
        digitsUnpacked_sorted = self.sortDigits(digitsUnpacked)

        # check the sizes
        if not len(digits_sorted) == len(digitsUnpacked_sorted):
            print('size: ', len(digits_sorted), len(digitsUnpacked_sorted))
            B2FATAL("TOPDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        precision = 0.0001  # precision for floats (e.g. in [ns])
        for i in range(len(digits_sorted)):

            digit = digits_sorted[i]
            digitUnpacked = digitsUnpacked_sorted[i]

            # check the content of the digit
            assert digit.getModuleID() == digitUnpacked.getModuleID()
            assert digit.getPixelID() == digitUnpacked.getPixelID()
            assert digit.getRawTime() == digitUnpacked.getRawTime()
            assert digit.getPulseHeight() == digitUnpacked.getPulseHeight()
            assert digit.getIntegral() == digitUnpacked.getIntegral()
            assert digit.getChannel() == digitUnpacked.getChannel()
            assert digit.getHitQuality() == digitUnpacked.getHitQuality()
            assert abs(digit.getTime() - digitUnpacked.getTime()) < precision
            assert abs(digit.getTimeError() - digitUnpacked.getTimeError()) < precision
            assert abs(digit.getPulseWidth() - digitUnpacked.getPulseWidth()) < precision
            assert digit.getFirstWindow() == digitUnpacked.getFirstWindow()


class RawDigitsTest(Module):

    """
    module which ckecks if two collections of TOPRawDigits are equal
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
                x.getScrodID(),
                x.getCarrierNumber(),
                x.getASICNumber(),
                x.getASICChannel(),
                x.getSampleRise())
        )

    def event(self):
        """
        load original TOPRawDigits and the packed/unpacked ones, sort and compare them
        """

        # direct from simulation
        digits = Belle2.PyStoreArray("TOPRawDigits")
        # processed by packer and unpacker
        digitsUnpacked = Belle2.PyStoreArray("TOPRawDigitsUnpacked")

        # sort digits
        digits_sorted = self.sortDigits(digits)
        digitsUnpacked_sorted = self.sortDigits(digitsUnpacked)

        # check the sizes
        if not len(digits_sorted) == len(digitsUnpacked_sorted):
            print('size: ', len(digits_sorted), len(digitsUnpacked_sorted))
            B2FATAL("TOPRawDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        for i in range(len(digits_sorted)):

            digit = digits_sorted[i]
            digitUnpacked = digitsUnpacked_sorted[i]

            # check the content of the digit
            assert digit.getScrodID() == digitUnpacked.getScrodID()
            assert digit.getCarrierNumber() == digitUnpacked.getCarrierNumber()
            assert digit.getASICNumber() == digitUnpacked.getASICNumber()
            assert digit.getASICChannel() == digitUnpacked.getASICChannel()
            assert digit.getASICWindow() == digitUnpacked.getASICWindow()
            assert digit.getTFine() == digitUnpacked.getTFine()
            assert digit.getSampleRise() == digitUnpacked.getSampleRise()
            assert digit.getDeltaSamplePeak() == digitUnpacked.getDeltaSamplePeak()
            assert digit.getDeltaSampleFall() == digitUnpacked.getDeltaSampleFall()
            assert digit.getValueRise0() == digitUnpacked.getValueRise0()
            assert digit.getValueRise1() == digitUnpacked.getValueRise1()
            assert digit.getValuePeak() == digitUnpacked.getValuePeak()
            assert digit.getValueFall0() == digitUnpacked.getValueFall0()
            assert digit.getValueFall1() == digitUnpacked.getValueFall1()
            assert digit.getIntegral() == digitUnpacked.getIntegral()
            assert digit.getErrorFlags() == digitUnpacked.getErrorFlags()


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
main.add_module(particlegun)

add_simulation(main, components=['TOP'])

Packer = register_module('TOPPacker')
main.add_module(Packer)

unPacker = register_module('TOPUnpacker')
unPacker.param('outputRawDigitsName', 'TOPRawDigitsUnpacked')
unPacker.param('outputDigitsName', 'TOPDigitsUnpacked')
main.add_module(unPacker)

converter = register_module('TOPRawDigitConverter')
converter.param('inputRawDigitsName', 'TOPRawDigitsUnpacked')
converter.param('outputDigitsName', 'TOPDigitsUnpacked')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('subtractOffset', True)
main.add_module(converter)

main.add_module(RawDigitsTest())
main.add_module(DigitsTest())

progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
