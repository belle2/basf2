#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.INFO)

main = create_path()
main.add_module("RootInput", inputFileNames=["test_data/DST_exp1_run*.root"])
# HistoManager must be used right after RootInput
main.add_module('HistoManager', histoFileName="CollectorOutput.root", workDirName=".")
# Showing off the options usable by all Collector Modules:
# * preScale is a builtin random prescaling factor. This fraction of events are collected
# * maxEventsPerRun allows you to stop collecting once this many events have been collected in a run
# * granularity can be 'run' or 'all'. If it's 'run' the collector objects are separated into different
#   objects. If it's 'all' then a single object is used for all runs.
# main.add_module("CaTest", preScale=0.5, maxEventsPerRun=250, granularity="run")
main.add_module("CaTest")
# Notice that there is NO RootOutput module required anymore, although you could include it if you wanted
process(main)
print(statistics(statistics.TOTAL))
