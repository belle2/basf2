#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import statistics
from ROOT import *
import sys
import os
import math


def statistics_plots(
    fileName='',
    timingMethods=[statistics.INIT, statistics.EVENT],
    memoryMethods=[statistics.EVENT],
    contact='',
    jobDesc='',
    ):
    """
    Add memory usage and execution time validation plots to the given root file.
    The current root file will be used if the fileName is empty (default).
    """

    # Open plot file
    save_dir = gDirectory
    plotFile = None
    if fileName:
        plotFile = TFile.Open(fileName, 'UPDATE')

    if not jobDesc:
        jobDesc = sys.argv[1]

    # Global timing
    methodName = {}
    hGlobalTiming = TH1D('GlobalTiming', 'Global Timing', 5, 0, 5)
    hGlobalTiming.SetStats(0)
    hGlobalTiming.GetXaxis().SetTitle('method')
    hGlobalTiming.GetYaxis().SetTitle('time/call [ms]')
    hGlobalTiming.GetListOfFunctions().Add(TNamed('Description',
            'The (average) time of the different basf2 execution phases for %s. The error bars show the rms of the time distributions.'
             % jobDesc))
    hGlobalTiming.GetListOfFunctions().Add(TNamed('Check',
            'There should be no significant and persistent increases in the the run time of the methods. Only cases where the increase compared to the reference or previous versions persists for at least two consecutive revisions should be reported since the measurements can be influenced by load from other processes on the execution host.'
            ))
    if contact:
        hGlobalTiming.GetListOfFunctions().Add(TNamed('Contact', contact))
    for (index, method) in statistics.EStatisticCounters.values.iteritems():
        methodName[method] = str(method)[0] \
            + str(method).lower()[1:].replace('_r', 'R')
        if index == 5:
            break
        hGlobalTiming.SetBinContent(index + 1,
                                    statistics.getGlobal().time_mean(method)
                                    * 1e-6)
        hGlobalTiming.SetBinError(index + 1,
                                  statistics.getGlobal().time_stddev(method)
                                  * 1e-6)
        hGlobalTiming.GetXaxis().SetBinLabel(index + 1, methodName[method])
    hGlobalTiming.Write()

    # Timing per module for the different methods
    modules = statistics.modules
    hModuleTiming = TH1D('ModuleTiming', 'Module Timing', len(modules), 0,
                         len(modules))
    hModuleTiming.SetStats(0)
    hModuleTiming.GetXaxis().SetTitle('module')
    hModuleTiming.GetYaxis().SetTitle('time/call [ms]')
    hModuleTiming.GetListOfFunctions().Add(TNamed('Check',
            'There should be no significant and persistent increases in the run time of a module. Only cases where the increase compared to the reference or previous versions persists for at least two consecutive revisions should be reported since the measurements can be influenced by load from other processes on the execution host.'
            ))
    if contact:
        hModuleTiming.GetListOfFunctions().Add(TNamed('Contact', contact))
    for method in timingMethods:
        hModuleTiming.SetTitle('Module %s Timing' % methodName[method])
        hModuleTiming.GetListOfFunctions().Add(TNamed('Description',
                'The (average) execution time of the %s method of modules for %s. The error bars show the rms of the time distributions.'
                 % (methodName[method], jobDesc)))
        index = 1
        for modstat in modules:
            hModuleTiming.SetBinContent(index, modstat.time_mean(method)
                    * 1e-6)
            hModuleTiming.SetBinError(index, modstat.time_stddev(method)
                                      * 1e-6)
            hModuleTiming.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        hModuleTiming.Write('%sTiming' % methodName[method])
        hModuleTiming.GetListOfFunctions().RemoveLast()

    # Memory usage profile
    memoryProfile = Belle2.PyStoreObj('MemoryProfile', 1)
    if memoryProfile:
        memoryProfile.obj().GetListOfFunctions().Add(TNamed('Description',
                'The memory usage vs. the event number for %s.' % jobDesc))
        memoryProfile.obj().GetListOfFunctions().Add(TNamed('Check',
                'The memory usage should be flat for high event numbers. If it keeps rising this is an idication of a memory leak.<br>There should also be no significant increases with respect to the reference (or previous revisions if no reference exists).'
                ))
        if contact:
            memoryProfile.obj().GetListOfFunctions().Add(TNamed('Contact',
                    contact))
        memoryProfile.obj().Write()

    # Memory usage per module for the different methods
    sqrtN = 1 / math.sqrt(statistics.getGlobal().calls() - 1)
    hModuleMemory = TH1D('ModuleMemory', 'Module Memory', len(modules), 0,
                         len(modules))
    hModuleMemory.SetStats(0)
    hModuleMemory.GetXaxis().SetTitle('module')
    hModuleMemory.GetYaxis().SetTitle('memory increase/call [kB]')
    hModuleMemory.GetListOfFunctions().Add(TNamed('Description',
            'The (average) increase in memory usage per call of the %s method of modules for %s.'
             % (methodName[method], jobDesc)))
    hModuleMemory.GetListOfFunctions().Add(TNamed('Check',
            'The increase in memory usage per call for each module should be consistent with zero or the reference.'
            ))
    if contact:
        hModuleMemory.GetListOfFunctions().Add(TNamed('Contact', contact))
    for method in memoryMethods:
        hModuleMemory.SetTitle('Module %s Memory' % methodName[method])
        index = 1
        for modstat in modules:
            hModuleMemory.SetBinContent(index, modstat.memory_mean(method))
            hModuleMemory.SetBinError(index, modstat.memory_stddev(method)
                                      * sqrtN)
            hModuleMemory.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        hModuleMemory.Write('%sMemory' % methodName[method])
        hModuleMemory.GetListOfFunctions().RemoveLast()

    if plotFile:
        plotFile.Close()
    save_dir.cd()


def event_timing_plot(
    dataFile,
    fileName='',
    maxTime=20.0,
    burnIn=1,
    contact='',
    jobDesc='',
    ):
    """
    Add a validation histogram of event execution time to the given root file.
    The current root file will be used if the fileName is empty (default).
    The data file has to contain the profile information created by the Profile
    module.
    """

    if not jobDesc:
        jobDesc = os.path.basename(sys.argv[0])

    # Get histogram with time vs event number
    save_dir = gDirectory
    data = TFile.Open(dataFile)
    entries = tree.GetEntries()
    tree.Draw('Entry$>>hEventTime(%d,-0.5,%d.5)' % (entries, entries - 1),
              'ProfileInfo.m_timeInSec', 'goff')
    hEventTime.SetDirectory(0)
    data.Close()
    save_dir.cd()

    # Open plot file
    plotFile = None
    if fileName:
        plotFile = TFile.Open(fileName, 'UPDATE')

    # Create and fill histogram with event execution time distribution
    stat = gStyle.GetOptStat()
    gStyle.SetOptStat(101110)
    hTiming = TH1D('Timing', 'Event Timing', 100, 0, maxTime)
    hTiming.UseCurrentStyle()
    hTiming.GetXaxis().SetTitle('time [s]')
    hTiming.GetYaxis().SetTitle('events')
    hTiming.GetListOfFunctions().Add(TNamed('Description',
                                     'The distribution of event execution times for %s.'
                                      % jobDesc))
    hTiming.GetListOfFunctions().Add(TNamed('Check',
                                     'The distribution should be consistent with the reference (or previous revisions if no reference exists).'
                                     ))
    if contact:
        hTiming.GetListOfFunctions().Add(TNamed('Contact', contact))
    for event in range(1 + burnIn, entries + 1):
        hTiming.Fill(hEventTime.GetBinContent(event)
                     - hEventTime.GetBinContent(event - 1))
    hTiming.Write()
    gStyle.SetOptStat(stat)

    if plotFile:
        plotFile.Close()
    save_dir.cd()


