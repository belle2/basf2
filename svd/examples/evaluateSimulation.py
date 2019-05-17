#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *

numEvents = 2000

main = create_path()

set_random_seed(1)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')

# gearbox
main.add_module('Gearbox')

# detector geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# event T0 jitter simulation
eventt0 = register_module('EventT0Generator')
eventt0.param('coreGaussWidth', 10.0)  # 10 sigma of core gaussian [ns]
# eventt0.param('fixedT0', nan)  If set, a fixed event t0 is used instead of simulating the bunch timing.
eventt0.param('tailGaussFraction', 0.0)  # 0 fraction (by area) of tail gaussian
eventt0.param('tailGaussWidth', 20.0)  # 20 sigma of tail gaussian [ns]
main.add_module(eventt0)

# detector simulation
main.add_module('FullSim')

# including the timing module
main.add_module("FullSimTiming", rootFile="EvtGenTiming.root", logLevel=LogLevel.INFO)

# SVD simulation
digitizer = register_module('SVDDigitizer')
digitizer.param('statisticsFilename', "digitizer.root")
digitizer.param('storeWaveforms', True)
main.add_module(digitizer)

main.add_module('RootOutput')
main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
