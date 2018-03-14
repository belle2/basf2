#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
emptypath = create_path()


def add_gdl_trigger(path, SimulationMode=1, OpenFilter=False, Belle2Phase="Phase2"):
    """
    add the gdl module to path
    @param path            module is added to this path
    @param SimulationMode  the simulation mode in TSIM, 1: fast simulation,
                           trigger algoritm simulation only, no firmware simulation
                           2: full simulation, both trigger algorithm and firmware
                           are simulated
    @param OpenFilter      if OpenFilter is True, the events failed to pass L1 trigger
                           will be discarded. Make sure you do need open filter before you
                           set the value to True
    """
    trggdl = register_module('TRGGDL')
    trggdl.param('SimulationMode', SimulationMode)
    trggdl.param('Belle2Phase', Belle2Phase)
    if OpenFilter:
        trggdl.if_value('<1', emptypath)
    path.add_module(trggdl)
