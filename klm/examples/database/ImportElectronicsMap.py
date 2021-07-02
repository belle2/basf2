#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import BKLM electronics mapping payloads.

import sys
import basf2
from ROOT.Belle2 import KLMElectronicsMapImporter, KLMElementNumbers, BKLMElementNumbers

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

# Database importer.
importer = KLMElectronicsMapImporter()


def clear_electronics_map():
    """
    Clear electronics map (to load another version).
    """
    importer.clearElectronicsMap()


def load_bklm_electronics_map(version, mc):
    """
    Load BKLM electronics map.
    Versions:
    1 = before experiment 10.
    2 = experiment 10 and later (mapping in chimney sector has changed).
    """
    if mc:
        importer.loadBKLMElectronicsMap(1)
    else:
        importer.loadBKLMElectronicsMap(version)
        # Switch lanes for real-data map.
        # The fibers of layer 1 and 2 are switched in BB6.
        importer.setLane(KLMElementNumbers.c_BKLM,
                         BKLMElementNumbers.c_BackwardSection,
                         7, 1, 2)
        importer.setLane(KLMElementNumbers.c_BKLM,
                         BKLMElementNumbers.c_BackwardSection,
                         7, 2, 1)


def load_eklm_electronics_map(version, mc):
    """
    Load EKLM electronics map.
    Versions:
    1 = phase 2 (wrong connection of cables for backward sectors 2 and 3).
    2 = phase 3
    """
    importer.loadEKLMElectronicsMap(version, mc)


if mc:
    # MC map: a single version of map is used for all periods.

    load_bklm_electronics_map(1, True)
    load_eklm_electronics_map(1, True)

    importer.setIOV(0, 0, 0, -1)
    importer.importElectronicsMap()

    importer.setIOV(1002, 0, 1002, -1)
    importer.importElectronicsMap()

    importer.setIOV(1003, 0, 1003, -1)
    importer.importElectronicsMap()

else:
    # Data map.

    # Experiments 0 - 3.
    load_bklm_electronics_map(1, False)
    load_eklm_electronics_map(1, False)
    importer.setIOV(1, 0, 3, -1)
    importer.importElectronicsMap()

    # Experiments 4 - 9.
    clear_electronics_map()
    load_bklm_electronics_map(1, False)
    load_eklm_electronics_map(2, False)
    importer.setIOV(4, 0, 9, -1)
    importer.importElectronicsMap()

    # Experiment 10 and later.
    clear_electronics_map()
    load_bklm_electronics_map(2, False)
    load_eklm_electronics_map(2, False)
    importer.setIOV(10, 0, -1, -1)
    importer.importElectronicsMap()
