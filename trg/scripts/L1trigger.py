#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import B2INFO, B2FATAL
from cdctrigger import add_cdc_trigger
from ecltrigger import add_ecl_trigger
from klmtrigger import add_klm_trigger
from grltrigger import add_grl_trigger
from gdltrigger import add_gdl_trigger


def add_trigger_simulation(
        path,
        SimulationMode=1,
        shortTracks=False,
        FilterEvents=False,
        Belle2Phase='Phase3',
        components=['CDC', 'ECL', 'KLM', 'GRL', 'GDL'],
        simulateT0jitter=False,
        PrintInfo=False):
    '''
    Add the L1 trigger simulation (TSIM) modules to path.

    @param path: Modules are added to this path.
    @param SimulationMode: The simulation mode in TSIM: 1) fast simulation, trigger algoritm simulation only,
        no firmware simulation; 2) full simulation, both trigger algorithm and firmware are simulated.
    @param shortTracks: The standard CDC track finding requires hits in 4 axial super layers. With the shortTracks
        option, tracks with hits in the 3 innermost super layers are also found.
    @param FilterEvents: if True only the events that pass the L1 trigger will survive simulation, the other are discarded.
        Make sure you do need to filter events before you set the value to True.
    @param Belle2Phase: The trigger menu at the given Phase is applied. Available options: Phase2, Phase3.
    @param components: List of sub-trigger components to be included in TSIM.
    @param simulateT0jitter: if True L1 trigger jitter is simulated by EventT0Generator.
    '''

    add_subdetector_tsim(
        path=path,
        SimulationMode=SimulationMode,
        shortTracks=shortTracks,
        components=components)
    add_grl_gdl_tsim(
        path=path,
        SimulationMode=SimulationMode,
        FilterEvents=FilterEvents,
        Belle2Phase=Belle2Phase,
        simulateT0jitter=simulateT0jitter,
        components=components)
    path.add_module('StatisticsSummary').set_name('Sum_TriggerSimulation')
    if PrintInfo:
        B2INFO('The L1 trigger simulation (TSIM) is set up with the following configuration:',
               SimulationMode=SimulationMode,
               ShortTracks=shortTracks,
               FilterEvents=FilterEvents,
               Belle2Phase=Belle2Phase,
               Components=', '.join(components))


def add_subdetector_tsim(
        path,
        SimulationMode=1,
        shortTracks=False,
        components=['CDC', 'ECL', 'KLM']):
    '''
    Add subdetector modules to the TSIM with no GRL and no GDL.

    @param path: Modules are added to this path.
    @param SimulationMode: The simulation mode in TSIM: 1) fast simulation, trigger algoritm simulation only,
        no firmware simulation; 2) full simulation, both trigger algorithm and firmware are simulated.
    @param shortTracks: The standard CDC track finding requires hits in 4 axial super layers. With the shortTracks
        option, tracks with hits in the 3 innermost super layers are also found.
    @param components: List of subdetector components to be included in TSIM.
    '''

    if ('CDC' in components):
        add_cdc_trigger(path=path, SimulationMode=SimulationMode, shortTracks=shortTracks, thetaDef='avg', zDef='min')
    if ('ECL' in components):
        add_ecl_trigger(path=path)
    if ('KLM' in components):
        add_klm_trigger(path=path)


def add_grl_gdl_tsim(
        path,
        SimulationMode=1,
        FilterEvents=False,
        Belle2Phase='Phase3',
        simulateT0jitter=False,
        components=['GRL', 'GDL']):
    '''
    Add GRL and GDL modules to the TSIM with no subdetectors. The function have to applied based on the dataobjects
    produced by add_subdetector_tsim.

    @param SimulationMode: The simulation mode in TSIM: 1) fast simulation, trigger algoritm simulation only,
        no firmware simulation; 2) full simulation, both trigger algorithm and firmware are simulated.
    @param FilterEvents: if True only the events that pass the L1 trigger will survive simulation, the other are discarded.
        Make sure you do need to filter events before you set the value to True.
    @param Belle2Phase: The trigger menu at the given Phase is applied. Available options: Phase2, Phase3.
    @param simulateT0jitter: if True L1 trigger jitter is simulated by EventT0Generator.
    @param components: List of logic components to be included in TSIM.
    '''

    if ('GRL' in components):
        add_grl_trigger(path=path, SimulationMode=SimulationMode)
    if ('GDL' in components):
        add_gdl_trigger(
            path=path,
            SimulationMode=SimulationMode,
            FilterEvents=FilterEvents,
            Belle2Phase=Belle2Phase,
            simulateT0jitter=simulateT0jitter)


def add_tsim(
        path,
        SimulationMode=1,
        shortTracks=False,
        FilterEvents=False,
        Belle2Phase='Phase3',
        components=['CDC', 'ECL', 'KLM', 'GRL', 'GDL'],
        PrintInfo=False):
    '''
    This convenience function is DEPRECATED!

    The L1 trigger simulation (TSIM) is now included in ``add_simulation``.

    If you already have a ``add_simulation`` in your path, you already get L1 trigger simulation.

    If you do not have ``add_simulation``, and you need the L1 trigger simulation,\
    please use ``add_trigger_simulation()``.

    '''

    B2FATAL("add_tsim() is deprecated. The L1 trigger simulation is now included\
 in add_simulation(). If you do not have add_simulation in your path, and you\
 need the L1 trigger simulation, please use add_trigger_simulation().")
