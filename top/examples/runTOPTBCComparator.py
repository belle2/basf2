#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Usage: basf2 runTOPTBCComparator.py calSetList.txt
#
from basf2 import *
import sys

set_log_level(LogLevel.INFO)

# Create path
main = create_path()


# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
argvs = sys.argv
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)


# TOP's data quality module
histo = register_module("HistoManager")
main.add_module(histo)


# compare
comparator = register_module('TOPTBCComparator')
comparator.param('inputDirectorList', sys.argv[1])
main.add_module(comparator)


# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
