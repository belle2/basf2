#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from cdctrigger import add_cdc_trigger
from ecltrigger import add_ecl_trigger
from klmtrigger import add_klm_trigger
from grltrigger import add_grl_trigger
from gdltrigger import add_gdl_trigger
from effCalculation import EffCalculation


def add_tsim(
    path,
    SimulationMode=1,
    shortTracks=False,
    OpenFilter=False,
    Belle2Phase="Phase3",
    PrintResult=False,
    component=[
        "CDC",
        "ECL",
        "KLM",
        "GRL",
        "GDL"]):
    """
    Add the L1 trigger simulation (TSIM) modules to path.

    @param path: Modules are added to this path.
    @param SimulationMode: The simulation mode in TSIM: 1) fast simulation, trigger algoritm simulation only,
        no firmware simulation; 2) full simulation, both trigger algorithm and firmware are simulated.
    @param shortTracks: The standard CDC track finding requires hits in 4 axial super layers. With the shortTracks
        option, tracks with hits in the 3 innermost super layers are also found.
    @param OpenFilter: If true, the events failed to pass L1 trigger will be discarded. Make sure you do need
        open filter before you set the value to True
    @param Belle2Phase: The trigger menu at the given Phase is applied. Available options: Phase2, Phase3
    @param components: List of sub-trigger components to be included in TSIM.
    """

    if ("CDC" in components):
        add_cdc_trigger(path=path, SimulationMode=SimulationMode, shortTracks=shortTracks, thetaDef='avg', zDef='min')
    if ("ECL" in components):
        add_ecl_trigger(path)
    if ("KLM" in components):
        add_klm_trigger(path)
    if ("GRL" in components):
        add_grl_trigger(path, SimulationMode)
    if ("GDL" in components):
        add_gdl_trigger(path=path, SimulationMode=SimulationMode, OpenFilter=OpenFilter, Belle2Phase=Belle2Phase)
    if PrintResult:
        EffCalculation(path, Belle2Phase=Belle2Phase)
    path.add_module('StatisticsSummary').set_name('Sum_TriggerSimulation')


def add_subdetector_tsim(
    path,
    SimulationMode=1,
    shortTracks=False,
    OpenFilter=False,
    Belle2Phase="Phase3",
    components=[
        "CDC",
        "ECL",
        "KLM"]):
    """
    Add subdetector modules to the TSIM with no GRL and no GDL.
    the parameters are the same as above
    """

    if ("CDC" in components):
        add_cdc_trigger(path=path, SimulationMode=SimulationMode, shortTracks=shortTracks)
    if ("ECL" in components):
        add_ecl_trigger(path=path)
    if ("KLM" in components):
        add_klm_trigger(path=path)


def add_grl_gdl_tsim(
        path,
        SimulationMode=1,
        OpenFilter=False,
        Belle2Phase="Phase3",
        PrintResult=False):
    """
    Add GRL and GDL modules to the TSIM with no subdetectors. The function have to applied based on the dataobjects
    produced by add_subdetector_tsim.
    the parameters are the same as above
    """

    add_grl_trigger(path, SimulationMode)
    add_gdl_trigger(path, SimulationMode, OpenFilter, Belle2Phase)
    if PrintResult:
        EffCalculation(path, Belle2Phase=Belle2Phase)
