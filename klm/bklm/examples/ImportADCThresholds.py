#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import BKLM ADC threshold payloads.

from ROOT.Belle2 import BKLMDatabaseImporter, BKLMADCThreshold

dbImporter = BKLMDatabaseImporter()

adc = BKLMADCThreshold()
# ADC pedestal.
pedestal = 3700
adc.setADCOffset(pedestal)
# Photoelectron amplitude.
amplitude = 20
adc.setMPPCGain(20)
# ADC threshold (the difference with pedestal is 3 photoelectron amplitudes).
adc.setADCThreshold(pedestal-3*amplitude)

dbImporter.importBklmADCThreshold(adc)
