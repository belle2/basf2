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
from basf2 import conditions as b2conditions

##################################################################################
#
# test SVD SensorInfo getters
#
# 1. > prepend the GT you want to test
# 2. > set the Experiment and Run number you want to test
# 3. > basf2 dumpSensorInfoParameters.py
#
##################################################################################


class printVelocity(basf2.Module):

    def beginRun(self):

        geoCache = Belle2.VXD.GeoCache.getInstance()

        layer = 3
        ladder = 1
        sensor = 1
        vxdID = Belle2.VxdID(layer, ladder, sensor)
        sensorInfo = geoCache.getSensorInfo(vxdID)
        print('printing velocity VS thickness for' + str(layer) + '.' + str(ladder) + '.' + str(sensor))
        thickness = sensorInfo.getThickness()
        N = 10
        step = thickness / N
        for s in range(0, N + 1):
            print()
            z = - thickness / 2 + step * s
            print('Z = ' + str(z * 1e4) + ' um')
            print('   electrons')
            sensorInfo.getVelocity(-1, r.TVector3(0, 0, z)).Print()
            print('   holes')
            sensorInfo.getVelocity(+1, r.TVector3(0, 0, z)).Print()


class printMobility(basf2.Module):

    def beginRun(self):

        geoCache = Belle2.VXD.GeoCache.getInstance()

        layer = 3
        ladder = 1
        sensor = 1
        vxdID = Belle2.VxdID(layer, ladder, sensor)
        sensorInfo = geoCache.getSensorInfo(vxdID)
        print('printing mobility VS thickness for' + str(layer) + '.' + str(ladder) + '.' + str(sensor))
        thickness = sensorInfo.getThickness()
        N = 10
        step = thickness / N
        for s in range(0, N + 1):
            print()
            z = - thickness / 2 + step * s
            print('Z = ' + str(z * 1e4) + ' um')
            print('   E-field')
            sensorInfo.getEField(r.TVector3(0, 0, z)).Print()
            print('   B-field')
            sensorInfo.getBField(r.TVector3(0, 0, z)).Print()
            print('electorn hall factor = ' + str(sensorInfo.getHallFactor(-1)))
            print('   electron mobility = ' + str(sensorInfo.getElectronMobility(sensorInfo.getEField(r.TVector3(0, 0, z)).Mag())))
            print('   hole mobility = ' + str(sensorInfo.getHoleMobility(sensorInfo.getEField(r.TVector3(0, 0, z)).Mag())))
            print('hole hall factor = ' + str(sensorInfo.getHallFactor(+1)))

        print('TEMPERATURE = ' + str(sensorInfo.getTemperature))

# add your GT here:
# gt = ""
# b2conditions.prepend_globaltag(gt)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [1003])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module('Geometry')

main.add_module(printVelocity())
main.add_module(printMobility())

print_path(main)

# Process events
process(main)

print(statistics)