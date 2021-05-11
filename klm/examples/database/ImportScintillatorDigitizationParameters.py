#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM scintillator digitization parameters payloads.

import sys
import basf2
import math
from ROOT.Belle2 import KLMDatabaseImporter, KLMScintillatorDigitizationParameters

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

dbImporter = KLMDatabaseImporter()

digPar = KLMScintillatorDigitizationParameters()
# ADC resolution (maximal output value + 1).
digPar.setADCRange(4096)
# ADC frequency in GHz.
digPar.setADCSamplingFrequency(1.017728000)
# Number of digitizations.
digPar.setNDigitizations(200)
# ADC pedestal.
digPar.setADCPedestal(3700)
# Photoelectron amplitude.
digPar.setADCPEAmplitude(20)
# ADC threshold (the difference with pedestal is 3 photoelectron amplitudes).
digPar.setADCThreshold(3640)
# ADC output value corresponding to saturation.
digPar.setADCSaturation(1000)
# Number of photoelectrons per 1 MeV.
digPar.setNPEperMeV(22)
# Cosine of the maximal angle of total internal reflection in the fiber.
digPar.setMinCosTheta(math.cos(26.7 / 180.0 * math.pi))
# Mirror reflective index.
digPar.setMirrorReflectiveIndex(0.95)
# Scintillator deexcitation time in ns.
digPar.setScintillatorDeExcitationTime(3.0)
# Fiber deexcitation time in ns.
digPar.setFiberDeExcitationTime(10.0)
# Effective light speed in the fiber in cm/ns.
digPar.setFiberLightSpeed(17.0)
# Fiber attenuation length.
digPar.setAttenuationLength(300.0)
# Photoelectron signal attenuation frequency in ns^{-1}.
# Value from T2K paper by F. Retiere: PoS (PD07) 017: 1.0 / 8.75.
digPar.setPEAttenuationFrequency(0.11428571428571428571)
# Mean SiPM noise (negative: turned off).
digPar.setMeanSiPMNoise(-1)
# Constant background.
digPar.setEnableConstBkg(False)

if (mc):
    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importScintillatorDigitizationParameters(digPar)

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importScintillatorDigitizationParameters(digPar)

    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importScintillatorDigitizationParameters(digPar)

else:
    dbImporter.setIOV(0, 0, -1, -1)
    dbImporter.importScintillatorDigitizationParameters(digPar)
