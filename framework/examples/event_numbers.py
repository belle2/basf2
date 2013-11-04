#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# EventInfoSetter - generate event numbers
eventinfosetter = register_module('EventInfoSetter')

evtrunlists = {'expList': [71, 71, 73, 73, 73], 'runList': [3, 4, 10, 20, 30],
              'evtNumList': [4, 6, 2, 5, 3]}
eventinfosetter.param(evtrunlists)

# EventInfoPrinter - show event meta info
eventinfo = register_module('EventInfoPrinter')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(eventinfo)

# Process all events
process(main)
