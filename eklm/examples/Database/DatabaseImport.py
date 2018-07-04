#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

from basf2 import *
import ROOT
from ROOT.Belle2 import EKLMDatabaseImporter

set_log_level(LogLevel.INFO)

eventinfosetter = register_module('EventInfoSetter')

# Gearbox
gearbox = register_module('Gearbox')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

process(main)

dbImporter = EKLMDatabaseImporter()
dbImporter.importDigitizationParameters()
dbImporter.importReconstructionParameters()
dbImporter.importSimulationParameters()
dbImporter.importTimeConversion()
