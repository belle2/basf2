#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# example steering to make root file with monitoring object
# includes arich and pxd monitoring object
# set to use local database

import basf2 as b2
import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)

b2.conditions.metadata_providers = ["conditions/metadata.sqlite"]
b2.conditions.override_globaltags(["master_2020-02-25"])
b2.conditions.payload_locations = ["conditions/"]

# Create main path
main = b2.create_path()

# Modules
# input root file with DQM histograms
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
inroot.param('Experiment', 1003)
main.add_module(inroot)


gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

geometry = b2.register_module('Geometry')
main.add_module(geometry)


# add DQM analysis module
arich = b2.register_module('DQMHistAnalysisARICHMonObj')
main.add_module(arich)
pxd1 = b2.register_module('DQMHistAnalysisPXDTrackCharge')
pxd1.param('RefHistoFile', "")
main.add_module(pxd1)

pxd2 = b2.register_module('DQMHistAnalysisPXDCM')
main.add_module(pxd2)

pxd3 = b2.register_module('DQMHistAnalysisPXDEff')
main.add_module(pxd3)

pxd4 = b2.register_module('DQMHistAnalysisPXDReduction')
main.add_module(pxd4)


# output created MonitoringObject to the root file
outroot = b2.register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
# uncomment to to store monitoring variables to run summary TTree
outroot.param('TreeFile', 'run_tree.root')
main.add_module(outroot)


b2.print_path(main)
# Process all events
b2.process(main)
