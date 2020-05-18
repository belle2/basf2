#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# example steering to make root file with monitoring object
# includes arich and pxd monitoring object
# set to use local database

from basf2 import *
from ROOT import Belle2
import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

conditions.metadata_providers = ["conditions/metadata.sqlite"]
conditions.override_globaltags(["master_2020-02-25"])
conditions.payload_locations = ["conditions/"]

# Create main path
main = create_path()

# Modules
# input root file with DQM histograms
inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
inroot.param('Experiment', 1003)
main.add_module(inroot)


gearbox = register_module('Gearbox')
main.add_module(gearbox)

geometry = register_module('Geometry')
main.add_module(geometry)


# add DQM analysis module
arich = register_module('DQMHistAnalysisARICHMonObj')
main.add_module(arich)
pxd1 = register_module('DQMHistAnalysisPXDTrackCharge')
pxd1.param('RefHistoFile', "")
main.add_module(pxd1)

pxd2 = register_module('DQMHistAnalysisPXDCM')
main.add_module(pxd2)

pxd3 = register_module('DQMHistAnalysisPXDEff')
main.add_module(pxd3)

pxd4 = register_module('DQMHistAnalysisPXDReduction')
main.add_module(pxd4)


# output created MonitoringObject to the root file
outroot = register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
# uncomment to to store monitoring variables to run summary TTree
outroot.param('TreeFile', 'run_tree.root')
main.add_module(outroot)


print_path(main)
# Process all events
process(main)
