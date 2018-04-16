#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Inspired by cdc/tests/cdc_packer_unpacker.py

# This test checks if ECLDigits that are packed and then unpacked are identical to the originals.
# ECLDigits are obtained in 2 ways:
# 1) From simulated muons
# 2) "Custom" ECLDigits that have different combinations of parameters spanning the allowed range.

from basf2 import *
from ROOT import Belle2
from unittest import TestCase
from ROOT import gRandom
import simulation
import itertools

logLevel = LogLevel.INFO  # LogLevel.DEBUG #
set_random_seed(42)

eclDigitsDatastoreName = 'ECLDigits'
unpackerOutputDatastoreName = 'someECLUnpackerDatastoreName'

import random


class addECLDigitsModule(Module):
    """
    Adds ECLDigits with very large/small time/amplitude values
    """

    def __init__(self):
        super().__init__()
        amps = [0, 1, 100000, 262015]
        times = [-2048, -100, 0, 100, 2047]
        qualitys = [0, 1, 2, 3]
        chis = [0, 1, 254, 511]
        paramNames = ["amp", "time", "quality", "chi"]
        self.digitParams = [dict(zip(paramNames, params)) for params in itertools.product(amps, times, qualitys, chis)]

    def event(self):
        """
        event function
        """
        eclDigitsFromSimulation = Belle2.PyStoreArray(eclDigitsDatastoreName)

        # Check for used cellIds
        usedCellIds = list(map(lambda eclDigit: eclDigit.getCellId(), eclDigitsFromSimulation))

        # Create new ECLDigits and add them to datastore
        for digitParam in self.digitParams:
            # Skip combination of quality != 2 and chi != 0. Electronics can't output this
            if (digitParam['quality'] is not 2) and (digitParam['chi'] is not 0):
                continue

            # Skip combination of quality == 2 and amp != 0. Electronics can't output this
            if (digitParam['quality'] is 2) and (digitParam['time'] is not 0):
                continue

            # Choose cellId that's not already used
            cellId = int(gRandom.Uniform(1, 8736))
            while cellId in usedCellIds:
                cellId = int(gRandom.Uniform(1, 8736))

            usedCellIds.append(cellId)
            # Create new ECLDigit
            eclDigit = Belle2.ECLDigit()

            # Fill ECLDigit
            eclDigit.setCellId(cellId)
            eclDigit.setAmp(digitParam['amp'])
            eclDigit.setTimeFit(digitParam['time'])
            eclDigit.setQuality(digitParam['quality'])
            eclDigit.setChi(digitParam['chi'])

            # Add ECLDigit to datastore
            newDigit = eclDigitsFromSimulation.appendNew()
            newDigit.__assign__(eclDigit)


class ECLPackerUnpackerTestModule(Module):
    """
    module which checks if two collections of ECLDigits are equal
    """

    def sortECLDigits(self, unsortedPyStoreArray):
        """ use a some information to sort the ECLDigits list
            Returns a python-list containing the ECLDigits
        """
        # first convert to a python-list to be able to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getCellId()))

    def event(self):
        """
        event function
        """

        # load the ECLDigits that were packed and unpacked
        eclDigitsPackedUnpacked_unsorted = Belle2.PyStoreArray(unpackerOutputDatastoreName)
        # ECLDigits direct from simulation
        eclDigitsFromSimulation_unsorted = Belle2.PyStoreArray(eclDigitsDatastoreName)

        if not len(eclDigitsFromSimulation_unsorted) == len(eclDigitsPackedUnpacked_unsorted):
            B2FATAL("Different number of ECLDigits from simulation and after packing+unpacking")

        eclDigitsPackedUnpacked = self.sortECLDigits(eclDigitsPackedUnpacked_unsorted)
        eclDigitsFromSimulation = self.sortECLDigits(eclDigitsFromSimulation_unsorted)

        tc = TestCase('__init__')

        # check all quantities between the direct and the packed/unpacked ECLDigits
        for idx in range(len(eclDigitsPackedUnpacked)):
            digit = eclDigitsFromSimulation[idx]
            digitPackedUnpacked = eclDigitsPackedUnpacked[idx]

            B2DEBUG(5, 'MC digit: cellid = ' +
                    str(digit.getCellId()) +
                    ', amp = ' +
                    str(digit.getAmp()) +
                    ', time = ' +
                    str(digit.getTimeFit()) +
                    ', quality = ' +
                    str(digit.getQuality()) +
                    ', chi = ' +
                    str(digit.getChi()) +
                    '\nUnpackedDigit: cellid = ' +
                    str(digitPackedUnpacked.getCellId()) +
                    ', amp = ' +
                    str(digitPackedUnpacked.getAmp()) +
                    ', time = ' +
                    str(digitPackedUnpacked.getTimeFit()) +
                    ', quality = ' +
                    str(digitPackedUnpacked.getQuality()) +
                    ', chi = ' +
                    str(digitPackedUnpacked.getChi()))

            tc.assertEqual(digit.getCellId(), digitPackedUnpacked.getCellId())
            tc.assertEqual(digit.getAmp(), digitPackedUnpacked.getAmp())
            tc.assertEqual(digit.getTimeFit(), digitPackedUnpacked.getTimeFit())
            tc.assertEqual(digit.getQuality(), digitPackedUnpacked.getQuality())
            tc.assertEqual(digit.getChi(), digitPackedUnpacked.getChi())


main = create_path()
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)
main.add_module(particlegun)

# add simulation for ECL only
simulation.add_simulation(main, components=['ECL'])

# Add ECLDigits with wide range of amp, time, quality, chi2
main.add_module(addECLDigitsModule())

# add the packer which packs the ECLDigits from the simulation
ecl_packer = register_module('ECLPacker')
main.add_module(ecl_packer)

# add the unpacker which unpacks the RawECL into ECLDigits and store them in a seperate datstore container
ecl_unpacker = register_module('ECLUnpacker')
# set the unpacker output datastore name, so that we don't overwrite the "normal" ECLDigits
ecl_unpacker.param('ECLDigitsName', unpackerOutputDatastoreName)
main.add_module(ecl_unpacker)

# Run test module to check if the original ECLDigits and the ones that were packed and unpacked are identical
eclPackUnpackerChecker = ECLPackerUnpackerTestModule()
main.add_module(eclPackUnpackerChecker, logLevel=logLevel)


# Process events
process(main)
