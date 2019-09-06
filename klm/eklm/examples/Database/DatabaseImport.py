#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

import basf2
import ROOT
from ROOT.Belle2 import EKLMDatabaseImporter

basf2.set_log_level(basf2.LogLevel.INFO)

dbImporter = EKLMDatabaseImporter()
dbImporter.importDigitizationParameters()
dbImporter.importReconstructionParameters()
dbImporter.importSimulationParameters()
