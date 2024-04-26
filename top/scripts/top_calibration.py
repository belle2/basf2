#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration functions
# ---------------------------------------------------------------------------------------

import basf2
from rawdata import add_unpackers
from reconstruction import add_cosmics_reconstruction
from softwaretrigger.constants import ALWAYS_SAVE_OBJECTS, RAWDATA_OBJECTS
from caf.framework import Calibration, Collection
from caf.strategies import SequentialRunByRun, SingleIOV, SequentialBoundaries
from ROOT.Belle2 import TOP


def BS13d_calibration_local(inputFiles, look_back=28, globalTags=None, localDBs=None, sroot=False):
    '''
    Returns calibration object for carrier shift calibration of BS13d with local runs
    (laser, single-pulse or double-pulse).
    :param inputFiles: A list of input files
    :param look_back: look-back window setting (set it to 0 to use the one from DB)
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param sroot: True if input files are in sroot format, False if in root format
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    if sroot:
        main.add_module('SeqRootInput')
    else:
        main.add_module('RootInput')
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
    cal.strategies = SequentialRunByRun

    return cal


def BS13d_calibration_cdst(inputFiles, time_offset=0, globalTags=None, localDBs=None,
                           new_cdst_format=True):
    '''
    Returns calibration object for carrier shift calibration of BS13d with processed data.
    :param inputFiles: A list of input files in cdst data format
    :param time_offset: time offset [ns]
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False, subtractRunningOffset=False)
        main.add_module('TOPTimeRecalibrator',
                        useAsicShiftCalibration=False, useChannelT0Calibration=True)
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator',
                        useAsicShiftCalibration=False, useChannelT0Calibration=True)

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
    cal.strategies = SequentialRunByRun

    return cal


def moduleT0_calibration_DeltaT(inputFiles, globalTags=None, localDBs=None,
                                new_cdst_format=True):
    '''
    Returns calibration object for rough module T0 calibration with method DeltaT
    :param inputFiles: A list of input files in cdst data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False, subtractRunningOffset=False)
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', usePIDLikelihoods=True, autoRange=True, useTimeSeed=False, useFillPattern=False,
                        subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPModuleT0DeltaTCollector')
    collector.param('granularity', 'run')

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
    cal.strategies = SequentialBoundaries  # Was SingleIOV before proc12

    return cal


def moduleT0_calibration_LL(inputFiles, sample='dimuon', globalTags=None, localDBs=None,
                            new_cdst_format=True):
    '''
    Returns calibration object for final module T0 calibration with method LL
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False, subtractRunningOffset=False)
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', usePIDLikelihoods=True, autoRange=True, useTimeSeed=False, useFillPattern=False,
                        subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPModuleT0LLCollector')
    collector.param('sample', sample)
    collector.param('granularity', 'run')

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
    cal.strategies = SequentialBoundaries  # Was SingleIOV before proc12

    return cal


def moduleT0_calibration_cosmics(inputFiles, globalTags=None, localDBs=None,
                                 data_format="raw", full_reco=True):
    '''
    Returns calibration object for module T0 calibration with cosmic data using DeltaT method.
    Note: by default cdst is processed with merging incoming and outcoming track segments of a cosmic particle,
    but we need here separate track segments in order to get ExtHits in incoming and outcoming module.
    :param inputFiles: A list of input files in cdst data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param data_format: "raw" for raw data or "cdst" for the new cdst format
    :param full_reco: on True, run full cosmics reconstruction also if data_format=="cdst"
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    if data_format == "cdst" and full_reco:
        main.add_module('RootInput', branchNames=ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS)
    else:
        main.add_module('RootInput')

    main.add_module('Gearbox')
    main.add_module('Geometry')
    if data_format == "raw" or full_reco:
        add_unpackers(main)
        add_cosmics_reconstruction(main, merge_tracks=False, reconstruct_cdst=True)
    else:
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')

    main.add_module('Ext')
    main.add_module('TOPChannelMasker')
    main.add_module('TOPCosmicT0Finder', useIncomingTrack=True, applyT0=False)
    main.add_module('TOPCosmicT0Finder', useIncomingTrack=False, applyT0=False)

    #   collector module
    collector = basf2.register_module('TOPModuleT0DeltaTCollector')
    collector.param('granularity', 'run')

    #   algorithm
    algorithm = TOP.TOPModuleT0DeltaTAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_moduleT0_cosmics', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.strategies = SingleIOV

    return cal


def commonT0_calibration_BF(inputFiles, globalTags=None, localDBs=None,
                            new_cdst_format=True):
    '''
    Returns calibration object for common T0 calibration with method BF
    :param inputFiles: A list of input files in cdst data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False, subtractRunningOffset=False)
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', usePIDLikelihoods=True, autoRange=True, useTimeSeed=False, useFillPattern=False,
                        subtractRunningOffset=False)

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
    cal.strategies = SequentialRunByRun

    return cal


def commonT0_calibration_LL(inputFiles, sample='dimuon', globalTags=None, localDBs=None,
                            new_cdst_format=True):
    '''
    Returns calibration object for common T0 calibration with method LL
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False, subtractRunningOffset=False)
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', usePIDLikelihoods=True, autoRange=True, useTimeSeed=False, useFillPattern=False,
                        subtractRunningOffset=False)

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
    cal.strategies = SequentialRunByRun

    return cal


def pulseHeight_calibration_laser(inputFiles, t_min=-50.0, t_max=0.0, look_back=28,
                                  globalTags=None, localDBs=None, sroot=False):
    '''
    Returns calibration object for calibration of pulse-height distributions and
    threshold efficiencies with local laser runs.
    :param inputFiles: A list of input files
    :param t_min: lower edge of time window to select laser signal [ns]
    :param t_max: upper edge of time window to select laser signal [ns]
    :param look_back: look-back window setting (set it to 0 to use the one from DB)
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param sroot: True if input files are in sroot format, False if in root format
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    if sroot:
        main.add_module('SeqRootInput')
    else:
        main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter', lookBackWindows=look_back)

    #   collector module
    collector = basf2.register_module('TOPPulseHeightCollector')
    collector.param('timeWindow', [t_min, t_max])
    collector.param('granularity', 'all')

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
    collector.param('granularity', 'all')

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
    cal.strategies = SingleIOV

    return cal


def module_alignment(inputFiles, sample='dimuon', fixedParameters=None,
                     globalTags=None, localDBs=None, new_cdst_format=True,
                     backend_args=None):
    '''
    Returns calibration object for alignment of TOP modules.
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :fixedParameters: a list of parameters to be fixed (parameter names: basf2 -m TOPAlignmentCollector)
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    :param backend_args: Dictionary of backend args for the Collection object to use
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
        if new_cdst_format:
            main.add_module('Gearbox')
            main.add_module('Geometry')
            main.add_module('Ext')
            main.add_module('TOPUnpacker')
            main.add_module('TOPRawDigitConverter')
            main.add_module('TOPChannelMasker')
            main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False,
                            subtractRunningOffset=False)
        else:
            main.add_module('TOPGeometryParInitializer')
            main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
            main.add_module('TOPChannelMasker')
            main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False,
                            usePIDLikelihoods=True, subtractRunningOffset=False)

        #   collector module: executing iterative alignment method
        collector = basf2.register_module('TOPAlignmentCollector')
        collector.param('sample', sample)
        if fixedParameters:
            collector.param('parFixed', fixedParameters)
        collector.param('targetModule', slot)
        collector.param('granularity', 'all')

        #   define collection:
        #   because of iterative nature of the method we must run exactly one job per collection
        #   which must process all the files from input list
        collection = Collection(collector=collector, input_files=inputFiles,
                                pre_collector_path=main, max_collector_jobs=1)
        if globalTags:
            for globalTag in reversed(globalTags):
                collection.use_central_database(globalTag)
        if localDBs:
            for localDB in reversed(localDBs):
                collection.use_local_database(localDB)
        if backend_args:
            collection.backend_args = backend_args

        #   add collection to calibration
        cal.add_collection(name='slot_' + f'{slot:02d}', collection=collection)

    #   algorithm: it just greps the last iterations of collections and prepares the payload
    algorithm = TOP.TOPAlignmentAlgorithm()
    cal.algorithms = algorithm

    return cal


def channel_mask_calibration(inputFiles, globalTags=None, localDBs=None, unpack=True):
    '''
    Returns calibration object for channel masking
    :param inputFiles: A list of input files in raw data or cdst format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param unpack: True if data unpacking is required (i.e. for raw data or for new cdst format)
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    if unpack:
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')

    #   collector module
    collector = basf2.register_module('TOPChannelMaskCollector')

    #   algorithm
    algorithm = TOP.TOPChannelMaskAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_ChannelMaskCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.strategies = SequentialRunByRun

    return cal


def offset_calibration(inputFiles, globalTags=None, localDBs=None,
                       new_cdst_format=True):
    '''
    Returns calibration object for the calibration of offsets
    :param inputFiles: A list of input files in cdst data format
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', autoRange=True, useTimeSeed=False, useFillPattern=False, subtractRunningOffset=False)
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', usePIDLikelihoods=True, autoRange=True, useTimeSeed=False, useFillPattern=False,
                        subtractRunningOffset=False)

    #   collector module
    collector = basf2.register_module('TOPOffsetCollector')

    #   algorithms
    algorithms = [TOP.TOPEventT0OffsetAlgorithm(), TOP.TOPFillPatternOffsetAlgorithm()]

    #   define calibration
    cal = Calibration(name='TOP_offsetCalibration', collector=collector,
                      algorithms=algorithms, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.strategies = SequentialRunByRun

    return cal


def photonYields_calibration(inputFiles, sample='dimuon', globalTags=None, localDBs=None):
    '''
    Returns calibration object for photon pixel yields aimed for PMT ageing studies and for finding optically decoupled PMT's
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    main.add_module('Gearbox')
    main.add_module('Geometry')
    main.add_module('Ext')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter')
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder')
    if sample == 'bhabha':
        main.add_module('TOPPDFDebugger', pdgCodes=[11])
    else:
        main.add_module('TOPPDFDebugger', pdgCodes=[13])

    #   collector module
    collector = basf2.register_module('TOPPhotonYieldsCollector')
    collector.param('sample', sample)
    collector.param('granularity', 'run')

    #   algorithm
    algorithm = TOP.TOPPhotonYieldsAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_photonYields', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.strategies = SequentialBoundaries  # Was SingleIOV before proc12

    return cal


def calibration_validation(inputFiles, sample='dimuon', globalTags=None, localDBs=None, new_cdst_format=True):
    '''
    Returns calibration object for calibration validation
    :param inputFiles: A list of input files in cdst data format
    :param sample: data sample ('dimuon' or 'bhabha')
    :param globalTags: a list of global tags, highest priority first
    :param localDBs: a list of local databases, highest priority first
    :param new_cdst_format: True or False for new or old cdst format, respectively
    '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('RootInput')
    if new_cdst_format:
        main.add_module('Gearbox')
        main.add_module('Geometry')
        main.add_module('Ext')
        main.add_module('TOPUnpacker')
        main.add_module('TOPRawDigitConverter')
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder')
    else:
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
        main.add_module('TOPChannelMasker')
        main.add_module('TOPBunchFinder', usePIDLikelihoods=True)

    #   collector module
    collector = basf2.register_module('TOPValidationCollector')
    collector.param('sample', sample)
    collector.param('granularity', 'run')

    #   algorithm
    algorithm = TOP.TOPValidationAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_validation', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    if globalTags:
        for globalTag in reversed(globalTags):
            cal.use_central_database(globalTag)
    if localDBs:
        for localDB in reversed(localDBs):
            cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.strategies = SingleIOV

    return cal
