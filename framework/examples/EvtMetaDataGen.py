#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, Event  0 to  3
# Experiment 71, Run  4, Event 10 to 15
# Experiment 73, Run 10, Event  0 to  1
# Experiment 73, Run 20, Event 30 to 34
# Experiment 73, Run 30, Event  1 to  5
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# EvtMetaGen - generate event meta data
evtmetagen = register_module('EvtMetaGen')

evtruninfo = {
    'ExpList': [71, 71, 73, 73, 73],
    'RunList': [3, 4, 10, 20, 30],
    'EvtStartList': [0, 10, 0, 30, 1],
    'EvtEndList': [3, 15, 1, 34, 5],
    }
evtmetagen.param(evtruninfo)

# EvtMetaInfo - show event meta info
evtmetainfo = register_module('EvtMetaInfo')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

# Process all events
process(main)
