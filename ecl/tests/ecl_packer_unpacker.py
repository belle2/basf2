#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Inspired by cdc/tests/cdc_packer_unpacker.py

from basf2 import *
from ROOT import Belle2
from unittest import TestCase

import simulation

set_random_seed(42)

eclDigitsDatastoreName = 'ECLDigits'
unpackerOutputDatastoreName = 'someECLUnpackerDatastoreName'


class ECLPackerUnpackerTestModule(Module):

    """
    module which checks if two collection of ECLDigits are equal
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

# add the packer which packs the ECLDigits from the simulation
ecl_packer = register_module('ECLPacker')
main.add_module(ecl_packer)

# add the unpacker which unpacks the RawECL into ECLDigits and store them in a seperate datstore container
ecl_unpacker = register_module('ECLUnpacker')
# set the unpacker output datastore name, so that we don't overwrite the "normal" ECLDigits
ecl_unpacker.param('ECLDigitsName', unpackerOutputDatastoreName)
main.add_module(ecl_unpacker)

# Run test module to check if the original ECLDigits and the ones that were packed and unpacked are identical
main.add_module(ECLPackerUnpackerTestModule())

# Process events
process(main)
