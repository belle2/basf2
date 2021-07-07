#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import BKLM ADC threshold payloads.

import basf2
from ROOT.Belle2 import BKLMDatabaseImporter, BKLMADCThreshold

basf2.set_log_level(basf2.LogLevel.INFO)

dbImporter = BKLMDatabaseImporter()

adc = BKLMADCThreshold()
# ADC pedestal.
pedestal = 3700
adc.setADCOffset(pedestal)
# Photoelectron amplitude.
amplitude = 20
adc.setMPPCGain(amplitude)
# ADC threshold (the difference with pedestal is 3 photoelectron amplitudes).
adc.setADCThreshold(pedestal - 3 * amplitude)

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importADCThreshold(adc)
