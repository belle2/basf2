#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, 4 Events
# Experiment 71, Run  4, 6 Events
# Experiment 73, Run 10, 2 Events
# Experiment 73, Run 20, 5 Events
# Experiment 73, Run 30, 3 Events
######################################################

from basf2 import Path, process

# Create main path
main = Path()
# EventInfoSetter - generate event numbers
main.add_module('EventInfoSetter',
                expList=[71, 71, 73, 73, 73],
                runList=[3, 4, 10, 20, 30],
                evtNumList=[4, 6, 2, 5, 3])
# EventInfoPrinter - show event meta info
main.add_module('EventInfoPrinter')
# Process all events
process(main)
