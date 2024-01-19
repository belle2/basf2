#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = b2.create_path()

# Modules

# input root file with DQM histograms
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
inroot.param('SelectFolders', 'ARICH')
main.add_module(inroot)
main.add_module("DQMHistAutoCanvas")  # Plot all Histo from Input

# add DQM analysis module
arich = b2.register_module('DQMHistAnalysisMonObj')
main.add_module(arich)

# output created MonitoringObject to the root file
outroot = b2.register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
# uncomment to to store monitoring variables to run summary TTree
# outroot.param('TreeFile', 'run_tree.root')
main.add_module(outroot)


b2.print_path(main)
# Process all events
b2.process(main)
