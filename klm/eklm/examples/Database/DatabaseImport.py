#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

from basf2 import *
import ROOT
from ROOT.Belle2 import EKLMDatabaseImporter

set_log_level(LogLevel.INFO)

dbImporter = EKLMDatabaseImporter()
dbImporter.importDigitizationParameters()
dbImporter.importReconstructionParameters()
dbImporter.importSimulationParameters()
