#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Create KLM scintillator FEE parameters payloads.

from ROOT import Belle2

data = Belle2.KLMScintillatorFEEData()
data.setPedestal(3700)
data.setPhotoelectronAmplitude(20)
# ADCPedestal - 3.0 * ADCPEAmplitude = 3700 - 3.0 * 20 = 3640
data.setThreshold(3640)
data.setVoltage(72.2)
data.setAdjustmentVoltage(0)
data.setLookbackTime(0)
data.setLookbackWindowWidth(0)

parameters = Belle2.KLMScintillatorFEEParameters()
index = Belle2.KLMChannelIndex()
index2 = Belle2.KLMChannelIndex()
while (index != index2.end()):
    if not (index.getSubdetector == Belle2.KLMElementNumbers.c_BKLM and
            inded.getLayer >= Belle2.BKLMElementNumbers.c_FirstRPCLayer):
        channel = index.getKLMChannelNumber()
        parameters.setFEEData(channel, data)
    index.increment()

db = Belle2.Database.Instance()

iov = Belle2.IntervalOfValidity(0, 0, 0, -1)
db.storeData("KLMScintillatorFEEParameters", parameters, iov)

iov = Belle2.IntervalOfValidity(1002, 0, 1002, -1)
db.storeData("KLMScintillatorFEEParameters", parameters, iov)

iov = Belle2.IntervalOfValidity(1003, 0, 1003, -1)
db.storeData("KLMScintillatorFEEParameters", parameters, iov)
