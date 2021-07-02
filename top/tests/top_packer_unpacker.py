#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
from simulation import add_simulation


b2.set_random_seed(12345)


class DigitsTest(b2.Module):

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
            b2.B2FATAL("TOPDigits: size not equal after packing and unpacking")

        # check all quantities between the direct and the packed/unpacked
        precision = 0.0001  # precision for floats (e.g. in [ns])
        for i in range(len(digits_sorted)):

            digit = digits_sorted[i]
            digitUnpacked = digitsUnpacked_sorted[i]

            # check the content of the digit
            assert digit.getModuleID() == digitUnpacked.getModuleID()
            assert digit.getPixelID() == digitUnpacked.getPixelID()
            assert digit.getChannel() == digitUnpacked.getChannel()
            assert digit.getRawTime() == digitUnpacked.getRawTime()
            assert abs(digit.getTime() - digitUnpacked.getTime()) < precision
            assert abs(digit.getTimeError() - digitUnpacked.getTimeError()) < precision
            assert digit.getPulseHeight() == digitUnpacked.getPulseHeight()
            assert abs(digit.getPulseWidth() - digitUnpacked.getPulseWidth()) < precision
            # assert digit.getIntegral() == digitUnpacked.getIntegral()
            assert digit.getFirstWindow() == digitUnpacked.getFirstWindow()
            assert digit.getHitQuality() == digitUnpacked.getHitQuality()
            assert digit.getStatus() == digitUnpacked.getStatus()
            assert digit.isChargeShare() == digitUnpacked.isChargeShare()
            assert digit.isPrimaryChargeShare() == digitUnpacked.isPrimaryChargeShare()


class RawDigitsTest(b2.Module):

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
            b2.B2FATAL("TOPRawDigits: size not equal after packing and unpacking")

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
            # assert digit.getIntegral() == digitUnpacked.getIntegral()
            assert digit.getRevo9Counter() == digitUnpacked.getRevo9Counter()
            assert digit.getPhase() == digitUnpacked.getPhase()
            assert digit.getErrorFlags() == digitUnpacked.getErrorFlags()
            assert digit.getDataType() == digitUnpacked.getDataType()


main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10]})
main.add_module(eventinfosetter)

particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
main.add_module(particlegun)

add_simulation(main, components=['TOP'])
b2.set_module_parameters(main, type="Geometry", useDB=False, components=["TOP"])

Packer = b2.register_module('TOPPacker')
main.add_module(Packer)

unPacker = b2.register_module('TOPUnpacker')
unPacker.param('outputRawDigitsName', 'TOPRawDigitsUnpacked')
unPacker.param('outputDigitsName', 'TOPDigitsUnpacked')
main.add_module(unPacker)

converter = b2.register_module('TOPRawDigitConverter')
converter.param('inputRawDigitsName', 'TOPRawDigitsUnpacked')
converter.param('outputDigitsName', 'TOPDigitsUnpacked')
converter.param('minPulseWidth', 0.0)
converter.param('maxPulseWidth', 1000.0)
main.add_module(converter)

main.add_module(RawDigitsTest())
main.add_module(DigitsTest())

progress = b2.register_module('Progress')
main.add_module(progress)

b2.process(main)
print(b2.statistics)
