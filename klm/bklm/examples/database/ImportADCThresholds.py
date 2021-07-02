#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
