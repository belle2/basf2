#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2 as b2
from basf2 import conditions as b2conditions
from svd import add_svd_trgsummary
from simulation import add_simulation

numEvents = 2000

# SVD-ONLY SIMULATION
# b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = b2.create_path()

b2.set_random_seed(1)

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')

# Belle2 Simulation
add_simulation(main, simulateT0jitter=True, usePXDDataReduction=False, forceSetPXDDataReduction=True)

for m in main.modules():
    if m.name() == "SVDEventInfoSetter":
        m.param("useDB", True)
        m.param("daqMode", 3)
        m.param("relativeShift", 9)
        m.set_log_level(b2.LogLevel.DEBUG)
        m.set_debug_level(25)
    if m.name() == "SVDDigitizer":
        m.set_log_level(b2.LogLevel.INFO)
        m.set_debug_level(25)

'''
# SVD_ONLY SIMULATION
# gearbox
main.add_module('Gearbox')

# detector geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# event T0 jitter simulation
eventt0 = b2.register_module('EventT0Generator')
eventt0.param('coreGaussWidth', 10.0)  # 10 sigma of core gaussian [ns]
# eventt0.param('fixedT0', nan)  If set, a fixed event t0 is used instead of simulating the bunch timing.
eventt0.param('tailGaussFraction', 0.0)  # 0 fraction (by area) of tail gaussian
eventt0.param('tailGaussWidth', 20.0)  # 20 sigma of tail gaussian [ns]
# main.add_module(eventt0)

# detector simulation
main.add_module('FullSim')

# including the timing module
# main.add_module("FullSimTiming", rootFile="EvtGenTiming.root", logLevel=LogLevel.INFO)

# add_svd_trgsummary(main)

# SVD simulation
svdevtinfoset = b2.register_module("SVDEventInfoSetter")
# svdevtinfoset.param("useDB",True)
svdevtinfoset.param("daqMode", 3)
svdevtinfoset.param("relativeShift", 9)
svdevtinfoset.set_log_level(b2.LogLevel.DEBUG)
svdevtinfoset.set_debug_level(25)

main.add_module(svdevtinfoset)

digitizer = b2.register_module('SVDDigitizer')
digitizer.param('statisticsFilename', "digitizer_test2021_1_hwclock.root")
digitizer.param('storeWaveforms', True)
digitizer.param('signalsList', "digitizer_test2021_1_hwclock.txt")
digitizer.set_log_level(b2.LogLevel.DEBUG)
digitizer.set_debug_level(30)
main.add_module(digitizer)
'''
main.add_module('RootOutput')
main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
