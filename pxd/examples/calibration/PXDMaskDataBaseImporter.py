#!/usr/bin/env python3
# -*- coding: utf-8 -*-


##############################################################################
#
# Imports a PXD mask to local data base.
#
##############################################################################


from basf2 import *
from ROOT import Belle2
import ROOT

# set some random seed
set_random_seed(10346)


def importEmptyHotPixelMask(expLow=0, runLow=0, expHigh=-1, runHigh=-1):
    """
    This function imports an empty payloads for the PXD
    hot pixel masking.

    This function works for every Belle 2 geometry
    """

    # Create an empty hot pixel mask
    HotPixelMask = Belle2.PXDMaskedPixelPar()

    # Set the iov for the mask
    iov = Belle2.IntervalOfValidity(expLow, runLow, expHigh, runHigh)
    Belle2.Database.Instance().storeData('PXDMaskedPixelPar', HotPixelMask, iov)


def importEmptyDeadPixelMask(expLow=0, runLow=0, expHigh=-1, runHigh=-1):
    """
    This function imports an empty payloads for the PXD
    dead pixel masking.

    This function works for every Belle 2 geometry
    """

    # Create an empty mask
    DeadPixelMask = Belle2.PXDDeadPixelPar()

    # Set the iov for the mask
    iov = Belle2.IntervalOfValidity(expLow, runLow, expHigh, runHigh)
    Belle2.Database.Instance().storeData('PXDDeadPixelPar', DeadPixelMask, iov)


def importEmptyOccupancyInfo(expLow=0, runLow=0, expHigh=-1, runHigh=-1):
    """
    This function imports an empty payloads for the PXD
    pixel masking.

    This function works for every Belle 2 geometry
    """

    # Create a dummy occupancy info
    OccupancyInfo = Belle2.PXDOccupancyInfoPar()

    # Set the iov for the mask
    iov = Belle2.IntervalOfValidity(expLow, runLow, expHigh, runHigh)
    Belle2.Database.Instance().storeData('PXDOccupancyInfoPar', OccupancyInfo, iov)


def importRandomPixelMaskPhase2(HotPixelFraction=0.001, expLow=0, runLow=0, expHigh=-1, runHigh=-1):
    """
    This function imports payloads for the PXD
    pixel masking with random hot pixels

    This function works for Phase 2
    """

    # Create an empty mask
    pixelMask = Belle2.PXDMaskedPixelPar()

    # Create a list of sensors
    sensorIDList = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

    # Mean number of hot pixels per sensor
    AverageHotPixels = HotPixelFraction * 250 * 768

    # ... and mask some random pixels
    for sensorID in sensorIDList:
        # Randomized number of hot pixels
        nHotPixels = ROOT.gRandom.Poisson(AverageHotPixels)

        for i in range(nHotPixels):
            uid = ROOT.gRandom.Integer(250)
            vid = ROOT.gRandom.Integer(768)
            print("mask pixel uid={}, vid={}, id={}".format(uid, vid, uid * 768 + vid))
            pixelMask.maskSinglePixel(sensorID.getID(), uid * 768 + vid)

    # Print the final mask before committing to db
    for sensorMask in pixelMask.getMaskedPixelMap():
        sensorID = Belle2.VxdID(sensorMask.first)
        print("Mask for sensor {} contains {} pixels".format(sensorID, sensorMask.second.size()))

    iov = Belle2.IntervalOfValidity(expLow, runLow, expHigh, runHigh)
    Belle2.Database.Instance().storeData('PXDMaskedPixelPar', pixelMask, iov)


if __name__ == "__main__":

    importEmptyHotPixelMask()
    importEmptyDeadPixelMask()
    importEmptyOccupancyInfo()
