#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Create EKLM database payloads.

import basf2
from ROOT.Belle2 import EKLMDatabaseImporter

basf2.set_log_level(basf2.LogLevel.INFO)

eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.initialize()
gearbox = basf2.register_module('Gearbox')
gearbox.initialize()

dbImporter = EKLMDatabaseImporter()
dbImporter.importSimulationParameters()
