#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.INFO)

from sys import argv
# logging.package("calibration").log_level = LogLevel.DEBUG
# logging.package("calibration").debug_level = 100

if len(argv) == 2:
    data_dir = argv[1]
else:
    print("Usage: basf2 collector_only.py <data directory>")
    sys.exit(1)

main = create_path()
main.add_module("RootInput", inputFileNames=[data_dir + "/*.root"])
# HistoManager must be used right after RootInput
main.add_module('HistoManager', histoFileName="CollectorOutput.root", workDirName=".")
# Granularity can be 'run' or 'all'. If it's 'run' the collector objects are separated into different
# objects. If it's 'all' then a single object is used for all runs.
# main.add_module("CaTest", granularity="all")
main.add_module("CaTest", granularity="run")
# Notice that there is NO RootOutput module required anymore, although you could include it if you wanted
process(main)
# The Collector Module memory can look weirdly high with default statistics printing. If you want to view the memory
# statistics of a collector use this command, Otherwise just do the normal print(statistics)
print(statistics(statistics.TOTAL))
print(statistics)
