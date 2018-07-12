#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from svd import add_svd_simulation
from ROOT import Belle2, TFile, TTree
import os
import numpy
from array import array

import simulation

# execute:
# basf2 svd_shaperdigit_test.py > test.log

# options
DigitsSort = True  # sort digits by sensor
printouts = False  # print SVDDigits <-> SVDShaperDigits comparison


svd_digits_pack_unpack_collection = "SVDDigits_test"
svd_shaperdigits_pack_unpack_collection = "SVDShaperDigits_test"
set_random_seed(42)


class SvdShaperDigitTestModule(Module):

    """
    module which ckecks equivalence between SVDDigits and SVDShaperDigits
    """

    def sortDigits(self, unsortedPyStoreArray):

        # convert to a python-list to be abple to sort
        py_list = [x for x in unsortedPyStoreArray]

        # sort via a hierachy of sort keys
        return sorted(
            py_list,
            key=lambda x: (
                x.getSensorID().getLayerNumber(),
                x.getSensorID().getLadderNumber(),
                x.getSensorID().getSensorNumber(),
                not x.isUStrip(),
                x.getCellID()))

    def initialize(self):
        B2INFO("test initialize")

    def terminate(self):
        B2INFO("SVDDigits <-> SVDShaperDigits comparison: SUCCESS !!!")

    def event(self):

        # load SVDDigit from the packer and unpacker
        svdDigits = Belle2.PyStoreArray(svd_digits_pack_unpack_collection)
        # load SVDShaperDigit from the packer and unpacker
        svdShaperDigits = Belle2.PyStoreArray(svd_shaperdigits_pack_unpack_collection)

        svdDigits_sorted = svdDigits
        svdShaperDigits_sorted = svdShaperDigits
        if DigitsSort:
            svdDigits_sorted = self.sortDigits(svdDigits)
            svdShaperDigits_sorted = svdShaperDigits  # self.sortDigits(svdShaperDigits)

        if not len(svdDigits_sorted) == len(svdShaperDigits_sorted) * 6:
            print("#SVDShaperDigits = ", len(svdShaperDigits_sorted), "\n#SVDDigits / 6 = ", len(svdDigits_sorted) / 6)
            B2FATAL("Numbers of SVDDigits and SVDShaperDigits objects dont't match !!!")

        # check all quantities between the SVDDigits & svdShaperDigits
        for i in range(len(svdShaperDigits_sorted)):
            hitShaper = svdShaperDigits_sorted[i]

            indS = 0
            stripS = hitShaper.getCellID()
            sensorS = hitShaper.getSensorID()
            isuS = hitShaper.isUStrip()
            chgS = hitShaper.getSamples()

            if printouts:
                print()
                print("object: ", i)
                print("SVDShaperDigits:")
                print("i", "sensor", "isU", "strip", "charge")
                print(indS, sensorS, isuS, stripS, chgS[0])
                print(" ", sensorS, isuS, stripS, chgS[1])
                print(" ", sensorS, isuS, stripS, chgS[2])
                print(" ", sensorS, isuS, stripS, chgS[3])
                print(" ", sensorS, isuS, stripS, chgS[4])
                print(" ", sensorS, isuS, stripS, chgS[5])

                print("SVDDigits:")
                print("i", "sensor", "isU", "strip", "charge")
            for j in range(6):
                hit = svdDigits_sorted[6 * i + j]

                ind = hit.getIndex()
                strip = hit.getCellID()
                sensor = hit.getSensorID()
                isu = hit.isUStrip()
                chg = hit.getCharge()

                if printouts:
                    print(ind, sensor, isu, strip, chg)

                assert (strip == stripS and sensor == sensorS and isu == isuS and chg ==
                        chgS[j]), B2FATAL("SVDDigits and SVDShaperDigits objects don't match !!!")


# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 10)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
# Show progress of processing
progress = register_module('Progress')

main = create_path()
# init path
main.add_module(eventinfosetter)
main.add_module(particlegun)
# add simulation for svd only
# add_svd_simulation(main, createDigits=True)
simulation.add_simulation(main, components=['SVD'])
main.add_module(progress)

nodeid = 0
Packer = register_module('SVDPacker')
Packer.param('NodeID', nodeid)
Packer.param('svdShaperDigitListName', 'SVDShaperDigits')
Packer.param('rawSVDListName', 'SVDRaw')

# optionally produce 3-sample data for specific FADCs
Packer.param('simulate3sampleData', False)

main.add_module(Packer)

unPacker = register_module('SVDUnpacker')
unPacker.param('rawSVDListName', 'SVDRaw')
unPacker.param('svdDigitListName', svd_digits_pack_unpack_collection)
unPacker.param('GenerateOldDigits', True)
unPacker.param('svdShaperDigitListName', svd_shaperdigits_pack_unpack_collection)
unPacker.param('svdDAQDiagnosticsListName', 'myDAQDiagnostics')
unPacker.param('badMappingFatal', False)
main.add_module(unPacker)

main.add_module(SvdShaperDigitTestModule())

# Process events
process(main)
