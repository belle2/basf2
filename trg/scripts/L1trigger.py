#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from cdctrigger import add_cdc_trigger
from ecltrigger import add_ecl_trigger
from klmtrigger import add_klm_trigger
from grltrigger import add_grl_trigger
from gdltrigger import add_gdl_trigger
from effCalculation import EffCalculation


def add_tsim(path, SimulationMode=1, minHits=4, OpenFilter=False):
    """
    add the gdl module to path
    @param path            module is added to this path
    @param SimulationMode  the simulation mode in TSIM, 1: fast simulation,
                           trigger algoritm simulation only, no firmware simulation
                           2: full simulation, both trigger algorithm and firmware
                           are simulated
    @param minHits         the minimum number of CDC super layers with hits, the default values is 4
    @param OpenFilter      if OpenFilter is True, the events failed to pass L1 trigger
                           will be discarded. Make sure you do need open filter before you
                           set the value to True
    """
    add_cdc_trigger(path, SimulationMode, minHits)
    add_ecl_trigger(path)
    add_klm_trigger(path)
    add_grl_trigger(path, SimulationMode)
    add_gdl_trigger(path, SimulationMode, OpenFilter)
    EffCalculation(path)
