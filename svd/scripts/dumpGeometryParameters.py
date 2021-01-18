#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
import math as m
import ROOT as r

##################################################################################
#
# test SVD geometry
#
# 1. > prepend the GT you want to test
# 2. > set the Experiment and Run number you want to test
# 3. > basf2 dumpGeometryParameters.py
#
##################################################################################


class printSVDPitches(b2.Module):
    '''class to print SVD pitches'''

    def printPitch(self, layer, ladder, sensor):
        '''print pitch function'''

        geoCache = Belle2.VXD.GeoCache.getInstance()

        vxdID = Belle2.VxdID(layer, 1, sensor)
        sensorInfo = geoCache.getSensorInfo(vxdID)
        print('pitches of sensor ' + str(layer) + '.' + str(ladder) + '.' + str(sensor) + ' at:')
        print('  at other coordinate = 0')
        print("U pitch = " + str(sensorInfo.getUPitch(0) * 10000) + ' mu')
        print("V pitch = " + str(sensorInfo.getVPitch(0) * 10000) + ' mu')
        print('  at other coordinate = +length/2')
        print("U pitch = " + str(sensorInfo.getUPitch(sensorInfo.getLength() / 2) * 10000) + ' mu')
        print("V pitch = " + str(sensorInfo.getVPitch(sensorInfo.getLength() / 2) * 10000) + ' mu')
        print('  at other coordinate = -length/2')
        print("U pitch = " + str(sensorInfo.getUPitch(-sensorInfo.getLength() / 2) * 10000) + ' mu')
        print("V pitch = " + str(sensorInfo.getVPitch(-sensorInfo.getLength() / 2) * 10000) + ' mu')

    def beginRun(self):
        '''begin run'''

        print('Layer 3')
        self.printPitch(3, 1, 2)

        print('Barrel')
        self.printPitch(4, 1, 2)

        print('Slanted')
        self.printPitch(4, 1, 1)


class printSVDSizes(b2.Module):
    ''' class to print SVD sizes'''

    def printSize(self, layer, ladder, sensor):
        '''function to print the sizes'''

        geoCache = Belle2.VXD.GeoCache.getInstance()

        vxdID = Belle2.VxdID(layer, ladder, sensor)
        sensorInfo = geoCache.getSensorInfo(vxdID)
        print('sizes of sensor ' + str(layer) + '.' + str(ladder) + '.' + str(sensor) + ' at:')
        print("W size = " + str(sensorInfo.getWSize() * 10000) + ' mu')
        print("V size = " + str(sensorInfo.getVSize() * 10) + ' mm')
        print("U size at V=-length/2 = " + str(sensorInfo.getUSize(-sensorInfo.getLength() / 2) * 10) + ' mm')
        print("U size at V=0 = " + str(sensorInfo.getUSize(0) * 10) + ' mm')
        print("U size at V=+length/2 = " + str(sensorInfo.getUSize(sensorInfo.getLength() / 2) * 10) + ' mm')

    def beginRun(self):
        '''begin run'''

        print('Layer 3')
        self.printSize(3, 1, 2)

        print('Barrel')
        self.printSize(4, 1, 2)

        print('Slanted')
        self.printSize(4, 1, 1)


class printSVDStripLengths(b2.Module):
    '''class to print strip length'''

    def printStripLength(self, layer, ladder, sensor):
        '''function to print strip length'''

        geoCache = Belle2.VXD.GeoCache.getInstance()

        vxdID = Belle2.VxdID(layer, ladder, sensor)
        sensorInfo = geoCache.getSensorInfo(vxdID)
        nUcells = sensorInfo.getUCells() - 1
        nVcells = sensorInfo.getVCells() - 1
        print('strip lengths ' + str(layer) + '.' + str(ladder) + '.' + str(sensor) + ' at:')
        print(' V strips at uID=0 = ' + str(sensorInfo.getStripLengthV(0) * 10) + ' mm')
        print(' V strips at uID=' + str(nUcells) + ' = ' + str(sensorInfo.getStripLengthV(nUcells) * 10) + ' mm')
        print(' U strips at vID=0 = ' + str(sensorInfo.getStripLengthU(0) * 10) + ' mm')
        print(' U strips at vID=' + str(nVcells) + ' = ' + str(sensorInfo.getStripLengthU(nVcells) * 10) + ' mm')

    def beginRun(self):
        '''begin run'''

        print('Layer 3')
        self.printStripLength(3, 1, 2)

        print('Barrel')
        self.printStripLength(4, 1, 2)

        print('Slanted')
        self.printStripLength(4, 1, 1)


class printSVDStripPositions(b2.Module):
    '''class to print strip positions'''

    def printStripPosition(self, layer, ladder, sensor):
        '''function to print strip positions'''

        geoCache = Belle2.VXD.GeoCache.getInstance()

        vxdID = Belle2.VxdID(layer, ladder, sensor)
        sensorInfo = geoCache.getSensorInfo(vxdID)
        nUcells = sensorInfo.getUCells() - 1
        nVcells = sensorInfo.getVCells() - 1
        print('strip positions ' + str(layer) + '.' + str(ladder) + '.' + str(sensor) + ' at:')
        print(' vID=0 = ' + str(sensorInfo.getVCellPosition(0) * 10) + ' mm')
        print(' vID=' + str(nVcells) + ' = ' + str(sensorInfo.getVCellPosition(nVcells) * 10) + ' mm')

        print(' uID=0 at v=0 = ' + str(sensorInfo.getUCellPosition(0, 0) * 10) + ' mm')
        print(' uID=0 at v=' + str(nVcells) + ' = ' + str(sensorInfo.getUCellPosition(0, nVcells) * 10) + ' mm')
        print(' uID=' + str(nUcells) + ' at v=0 = ' + str(sensorInfo.getUCellPosition(nUcells, 0) * 10) + ' mm')
        print(' uID=' + str(nUcells) + ' at v=' + str(nVcells) + ' = ' +
              str(sensorInfo.getUCellPosition(nUcells, nVcells) * 10) + ' mm')

        print('insideout')
        print('U cellID at vID=nVcell (0) = ' +
              str(sensorInfo.getUCellID(sensorInfo.getUCellPosition(0, nVcells), sensorInfo.getVCellPosition(nVcells))))
        print('U cellID at vID=0 (0) = ' +
              str(sensorInfo.getUCellID(sensorInfo.getUCellPosition(0, 0), sensorInfo.getVCellPosition(0))))

    def beginRun(self):
        '''begin run'''

        print('Layer 3')
        self.printStripPosition(3, 1, 2)

        print('Barrel')
        self.printStripPosition(4, 1, 2)

        print('Slanted')
        self.printStripPosition(4, 1, 1)


class printSVDSensors(b2.Module):
    '''class to print svd sensors'''

    def beginRun(self):
        '''begin run'''
        geoCache = Belle2.VXD.GeoCache.getInstance()

        print("printing all sensors of the SVD in the geometry:")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    print(str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))


class printSVDLadders(b2.Module):
    '''class to print svd ladders'''

    def beginRun(self):
        '''begin run'''

        geoCache = Belle2.VXD.GeoCache.getInstance()

        print("printing all L3 backward sensors position of the SVD in the geometry:")
        print("format: sensorID X(cm) Y(cm) phi(rad) phi(deg)")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()

            if not layerNumber == 3:
                continue

            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    if not(sensorNumber == layerNumber - 1):
                        continue
                    # sensor ID string
                    sensorID = str(layerNumber) + "." + str(ladderNumber)
                    # get global coordinates:
                    local = r.TVector3(0, 0, 0)
                    glob = geoCache.getSensorInfo(sensor).pointToGlobal(local)

                    angleRad = m.atan2(glob.Y(), glob.X())
                    print(angleRad)
                    if(angleRad < 0):
                        angleRad = 2 * m.pi + angleRad
                        print("second = " + str(angleRad))

                    angleDeg = m.degrees(angleRad)

                    xStr = "{0:.2f}".format(round(glob.X(), 2))
                    yStr = "{0:.2f}".format(round(glob.Y(), 2))
                    radStr = "{0:.2f}".format(round(angleRad, 4))
                    degStr = "{0:.2f}".format(round(angleDeg, 2))
                    print(sensorID + '     ' + xStr + '     ' + yStr + '     ' + radStr + '     ' + degStr)


# add your GT here:
# gt = ""
# b2conditions.prepend_globaltag(gt)

# Create paths
main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [1003])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module('Geometry')

main.add_module(printSVDSensors())
main.add_module(printSVDLadders())
main.add_module(printSVDPitches())
main.add_module(printSVDSizes())
main.add_module(printSVDStripLengths())
main.add_module(printSVDStripPositions())

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
