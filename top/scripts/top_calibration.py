#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration functions
#
# author: M. Staric
# ---------------------------------------------------------------------------------------

import basf2
from caf.framework import Calibration, Collection
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
from ROOT import Belle2
from ROOT.Belle2 import TOP


def BS13d_calibration_local(inputFiles, look_back=28, globalTags=None, localDBs=None):
    '''
    Returns calibration object for carrier shift calibration of BS13d with local runs
    (laser, single-pulse or double-pulse).
    :param inputFiles: A list of input files in sroot format
    :param look_back: look-back window setting (set it to 0 to use the one from DB)
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('SeqRootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter', lookBackWindows=look_back,
                    useAsicShiftCalibration=False, useChannelT0Calibration=False)

    #   collector module
    collector = basf2.register_module('TOPAsicShiftsBS13dCollector')

    #   algorithm
    algorithm = TOP.TOPAsicShiftsBS13dAlgorithm()
    algorithm.setWindowSize(0)

    #   define calibration
    cal = Calibration(name='TOP_BS13dCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


def BS13d_calibration_rawdata(inputFiles, globalTags=None, localDBs=None):
    '''
    Returns calibration object for carrier shift calibration of BS13d with raw data.
    :param inputFiles: A list of input files in raw data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter',
                    useAsicShiftCalibration=False, useChannelT0Calibration=False)

    #   collector module
    collector = basf2.register_module('TOPAsicShiftsBS13dCollector')

    #   algorithm
    algorithm = TOP.TOPAsicShiftsBS13dAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_BS13dCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


def BS13d_calibration_cdst(inputFiles, time_offset=0, globalTags=None, localDBs=None):
    '''
    Returns calibration object for carrier shift calibration of BS13d with processed data.
    :param inputFiles: A list of input files in cdst data format
    :param time_offset: time offset [ns]
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator',
                    useAsicShiftCalibration=False, useChannelT0Calibration=False)

    #   collector module
    collector = basf2.register_module('TOPAsicShiftsBS13dCollector',
                                      timeOffset=time_offset, requireRecBunch=True)

    #   algorithm
    algorithm = TOP.TOPAsicShiftsBS13dAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_BS13dCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


def moduleT0_calibration_DeltaT(inputFiles, globalTags=None, localDBs=None):
    '''
    Returns calibration object for rough module T0 calibration with method DeltaT
    :param inputFiles: A list of input files in cdst data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPModuleT0DeltaTCollector')

    #   algorithm
    algorithm = TOP.TOPModuleT0DeltaTAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_moduleT0_rough', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


def moduleT0_calibration_LL(inputFiles, sample='dimuon', globalTags=None, localDBs=None):
    '''
    Returns calibration object for final module T0 calibration with method LL
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPModuleT0LLCollector')
    collector.param('sample', sample)

    #   algorithm
    algorithm = TOP.TOPModuleT0LLAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_moduleT0_final', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


def commonT0_calibration_BF(inputFiles, globalTags=None, localDBs=None):
    '''
    Returns calibration object for common T0 calibration with method BF
    :param inputFiles: A list of input files in cdst data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPCommonT0BFCollector')

    #   algorithm
    algorithm = TOP.TOPCommonT0BFAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_commonT0Calibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


def commonT0_calibration_LL(inputFiles, sample='dimuon', globalTags=None, localDBs=None):
    '''
    Returns calibration object for common T0 calibration with method LL
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPCommonT0LLCollector')
    collector.param('sample', sample)

    #   algorithm
    algorithm = TOP.TOPCommonT0LLAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_commonT0Calibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


def pulseHeight_calibration_laser(inputFiles, t_min=-50.0, t_max=0.0, look_back=28,
                                  globalTags=None, localDBs=None):
    '''
    Returns calibration object for calibration of pulse-height distributions and
    threshold efficiencies with local laser runs.
    :param inputFiles: A list of input files in sroot format
    :param t_min: lower edge of time window to select laser signal [ns]
    :param t_max: upper edge of time window to select laser signal [ns]
    :param look_back: look-back window setting (set it to 0 to use the one from DB)
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('SeqRootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter', lookBackWindows=look_back)

    #   collector module
    collector = basf2.register_module('TOPPulseHeightCollector')
    collector.param('timeWindow', [t_min, t_max])

    #   algorithm
    algorithm = TOP.TOPPulseHeightAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_pulseHeightCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


def pulseHeight_calibration_rawdata(inputFiles, globalTags=None, localDBs=None):
    '''
    Returns calibration object for calibration of pulse-height distributions and
    threshold efficiencies with raw data
    :param inputFiles: A list of input files in raw data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter')

    #   collector module
    collector = basf2.register_module('TOPPulseHeightCollector')

    #   algorithm
    algorithm = TOP.TOPPulseHeightAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_pulseHeightCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


def module_alignment(inputFiles, sample='dimuon', fixedParameters=['dn/n'],
                     recalibrate=True, globalTags=None, localDBs=None):
    '''
    Returns calibration object for alignment of TOP modules.
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :fixedParameters: a list of parameters to be fixed (parameter names: basf2 -m TOPAlignmentCollector)
    :recalibrate: on True recalibrate time and reconstruct bunch again
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   define calibration
    cal = Calibration(name='TOP_alignment')
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.strategies = SingleIOV

    #   add collections
    for slot in range(1, 17):
        #   create path
        main = basf2.create_path()

        #   add basic modules
        main.add_module('RootInput')
        main.add_module('TOPGeometryParInitializer')
        if recalibrate:
            main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
            main.add_module('TOPChannelMasker')
            main.add_module('TOPBunchFinder',
                            usePIDLikelihoods=True, subtractRunningOffset=False)
        else:
            main.add_module('TOPChannelMasker')

        #   collector module
        collector = basf2.register_module('TOPAlignmentCollector')
        collector.param('sample', sample)
        collector.param('parFixed', fixedParameters)
        collector.param('targetModule', slot)

        #   define collection
        collection = Collection(collector=collector, input_files=inputFiles,
                                pre_collector_path=main, max_files_per_collector_job=-1)
        if globalTags:
            for globalTag in reversed(globalTags):
                collection.use_central_database(globalTag)
        if localDBs:
            for localDB in reversed(localDBs):
                collection.use_local_database(localDB)
        collection.backend_args = {"queue": "l"}

        #   add collection to calibration
        cal.add_collection(name='slot_' + '{:0=2d}'.format(slot), collection=collection)

    #   algorithm
    algorithm = TOP.TOPAlignmentAlgorithm()
    cal.algorithms = algorithm

    return cal
