#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create several EKLM database payloads.

from ROOT.Belle2 import EKLMDatabaseImporter

dbImporter = EKLMDatabaseImporter()
dbImporter.importDigitizationParameters()
dbImporter.importReconstructionParameters()
dbImporter.importSimulationParameters()
