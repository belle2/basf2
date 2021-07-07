#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

##################################################################################
#
# test SVD SensorInfo getters
#
# 1. > prepend the GT you want to test
# 2. > set the Experiment and Run number you want to test
# 3. > basf2 dumpSimulationParameters.py
#
##################################################################################


class printElectronicNoise(b2.Module):
    """ print electronic noise """

    def beginRun(self):
        """do everything here"""

        geoCache = Belle2.VXD.GeoCache.getInstance()

        print('printing ElectronicNoise for all ladder = 1 sensors')
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                if not ladder.getLadderNumber() == 1:
                    continue
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    print(str(layerNumber) + '.1.' + str(sensorNumber))
                    sensorInfo = geoCache.getSensorInfo(sensor)
                    print('   U-side noise = '+str(sensorInfo.getElectronicNoiseU())+' e-')
                    print('   V-side noise = '+str(sensorInfo.getElectronicNoiseV())+' e-')


class printAduEquivalent(b2.Module):
    """ print ADU equivalent"""

    def beginRun(self):
        """do everything here"""
        geoCache = Belle2.VXD.GeoCache.getInstance()

        print('printing AduEquivalent for all ladder = 1 sensors')
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                if not ladder.getLadderNumber() == 1:
                    continue
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    print(str(layerNumber) + '.1.' + str(sensorNumber))
                    sensorInfo = geoCache.getSensorInfo(sensor)
                    print('   U-side ADU equivalent = '+str(sensorInfo.getAduEquivalentU())+' e-/ADC')
                    print('   V-side ADU equivalent = '+str(sensorInfo.getAduEquivalentV())+' e-/ADC')


# add your GT here:
# gt = ""
# b2conditions.prepend_globaltag(gt)

main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [1003])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module('Geometry')

main.add_module(printElectronicNoise())
main.add_module(printAduEquivalent())

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
