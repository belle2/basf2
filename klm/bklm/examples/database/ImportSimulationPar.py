#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import BKLM simulation parameters payloads.

import basf2
from ROOT.Belle2 import BKLMDatabaseImporter

basf2.set_log_level(basf2.LogLevel.INFO)

# The Gearbox is needed for BKLMSimulationPar
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.initialize()
gearbox = basf2.register_module('Gearbox')
gearbox.initialize()

dbImporter = BKLMDatabaseImporter()

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importSimulationPar()
