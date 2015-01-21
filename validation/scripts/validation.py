#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import statistics
from ROOT import *
import math


def statistics_plots(fileName='', timingMethods=[statistics.INIT,
                     statistics.EVENT], memoryMethods=[statistics.EVENT]):
    """
    Add memory usage and execution time validation plots to the given root file.
    The current root file will be used if the fileName is empty (default).
    """

    if fileName:
        TFile.Open(fileName, 'UPDATE')

    # Global timing
    methodName = {}
    hGlobalTiming = TH1D('GlobalTiming', 'Global Timing', 5, 0, 5)
    hGlobalTiming.SetStats(0)
    hGlobalTiming.GetXaxis().SetTitle('method')
    hGlobalTiming.GetYaxis().SetTitle('time/call [ms]')
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
    for method in timingMethods:
        hModuleTiming.SetTitle('%s Timing' % methodName[method])
        index = 1
        for modstat in modules:
            hModuleTiming.SetBinContent(index, modstat.time_mean(method)
                    * 1e-6)
            hModuleTiming.SetBinError(index, modstat.time_stddev(method)
                                      * 1e-6)
            hModuleTiming.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        hModuleTiming.Write('%sTiming' % methodName[method])

    # Memory usage profile
    memoryProfile = Belle2.PyStoreObj('MemoryProfile', 1)
    if memoryProfile:
        memoryProfile.obj().Write()

    # Memory usage per module for the different methods
    sqrtN = 1 / math.sqrt(statistics.getGlobal().calls() - 1)
    hModuleMemory = TH1D('ModuleMemory', 'Module Memory', len(modules), 0,
                         len(modules))
    hModuleMemory.SetStats(0)
    hModuleMemory.GetXaxis().SetTitle('module')
    hModuleMemory.GetYaxis().SetTitle('memory increase/call [kB]')
    for method in memoryMethods:
        hModuleMemory.SetTitle('%s Memory' % methodName[method])
        index = 1
        for modstat in modules:
            hModuleMemory.SetBinContent(index, modstat.memory_mean(method))
            hModuleMemory.SetBinError(index, modstat.memory_stddev(method)
                                      * sqrtN)
            hModuleMemory.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        hModuleMemory.Write('%sMemory' % methodName[method])


