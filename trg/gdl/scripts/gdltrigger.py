#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import modularAnalysis as ma


def add_gdl_trigger(path, SimulationMode=1, FilterEvents=False, simulateT0jitter=False, Belle2Phase="Phase2"):
    """
    add the gdl module to path
    @param path            module is added to this path
    @param SimulationMode  the simulation mode in TSIM, 1: fast simulation,
                           trigger algorithm simulation only, no firmware simulation
                           2: full simulation, both trigger algorithm and firmware
                           are simulated
    @param FilterEvents    if True only the events that pass the L1 trigger will
                           survive simulation, the other are discarded.
                           Make sure you do need to filter events before you set
                           the value to True.
    """
    trggdl = b2.register_module('TRGGDL')
    trggdl.param('SimulationMode', SimulationMode)
    trggdl.param('Belle2Phase', Belle2Phase)
    trggdl.param('simulateT0jitter', simulateT0jitter)
    path.add_module(trggdl)
    if FilterEvents:
        ma.applyEventCuts('L1Trigger == 1', path)
