#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from basf2 import *
from ROOT import Belle2
import math as m
import ROOT as r
import os.path
import sys

##################################################################################
#
# test SVD SensorInfo getters
#
# 1. add the database you want to test
# 2. > set the Experiment and Run number you want to test
# 3. > basf2 testSimulationPar.py
#
##################################################################################


class printElectronicNoise(basf2.Module):

    def beginRun(self):

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


class printAduEquivalent(basf2.Module):

    def beginRun(self):

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


# add your database here
# use_database_chain()
# use_central_database("release-02-00-00")
# use_central_database("release-02-01-00")
# use_central_database("master_2018-12-24")
# use_central_database("release-03-00-02")
# use_local_database('data_reprocessing_prompt')
# Create paths


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [1003])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module('Geometry', useDB=True)

main.add_module(printElectronicNoise())
main.add_module(printAduEquivalent())

print_path(main)

# Process events
process(main)

print(statistics)
