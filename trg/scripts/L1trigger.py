#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from cdctrigger import add_cdc_trigger
from ecltrigger import add_ecl_trigger
from klmtrigger import add_klm_trigger
from grltrigger import add_grl_trigger
from gdltrigger import add_gdl_trigger
from effCalculation import EffCalculation


def add_tsim(path, SimulationMode=1, shortTracks=False, OpenFilter=False, Belle2Phase="Phase2", PrintResult=False):
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
    @param Belle2Phase      the trigger menu at the phase is applied. Option: Phase2, Phase3
    """
    add_cdc_trigger(path=path, SimulationMode=SimulationMode, shortTracks=shortTracks, thetaDef='avg', zDef='min')
    add_ecl_trigger(path)
    add_klm_trigger(path)
    add_grl_trigger(path, SimulationMode)
    add_gdl_trigger(path=path, SimulationMode=SimulationMode, OpenFilter=OpenFilter, Belle2Phase=Belle2Phase)
    if PrintResult:
        EffCalculation(path, Belle2Phase=Belle2Phase)
    path.add_module('StatisticsSummary').set_name('Sum_TriggerSimulation')


def add_subdetector_tsim(path, SimulationMode=1, shortTracks=False, OpenFilter=False, Belle2Phase="Phase2"):
    """
    add the trigger simlation of subdetector, no grl and gdl
    the parameters are the same as above
    """
    add_cdc_trigger(path=path, SimulationMode=SimulationMode, shortTracks=shortTracks)
    add_ecl_trigger(path=path)
    add_klm_trigger(path=path)


def add_grl_gdl_tsim(path, SimulationMode=1, OpenFilter=False, Belle2Phase="Phase2", PrintResult=False):
    """
    add grl and gdl, the function have to applied based on the
    dataobjects produced in add_subdetector_trigger_simulation
    the parameters are the same as above
    """
    add_grl_trigger(path, SimulationMode)
    add_gdl_trigger(path, SimulationMode, OpenFilter, Belle2Phase)
    if PrintResult:
        EffCalculation(path, Belle2Phase=Belle2Phase)
