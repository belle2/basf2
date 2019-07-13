#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM scintillator digitization parameters.

import sys
import basf2
import ROOT
import math
from ROOT.Belle2 import BKLMDatabaseImporter, BKLMADCThreshold

basf2.set_log_level(basf2.LogLevel.INFO)

eventinfosetter = basf2.register_module('EventInfoSetter')

# Gearbox
gearbox = basf2.register_module('Gearbox')

# Create main path
main = basf2.create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

basf2.process(main)

dbImporter = BKLMDatabaseImporter()

adc = BKLMADCThreshold()
# ADC pedestal.
adc.setADCOffset(3700)
# Photoelectron amplitude.
adc.setMPPCGain(20)
# ADC threshold (the difference with pedestal is 3 photoelectron amplitudes).
adc.setADCThreshold(3640)

dbImporter.importBklmADCThreshold(adc)
