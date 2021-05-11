#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Usage: basf2 runTOPTBCComparator.py calSetList.txt
#
import basf2 as b2
import sys

b2.set_log_level(b2.LogLevel.INFO)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
argvs = sys.argv
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# TOP's data quality module
histo = b2.register_module("HistoManager")
main.add_module(histo)

# compare
comparator = b2.register_module('TOPTBCComparator')
comparator.param('inputDirectorList', sys.argv[1])
main.add_module(comparator)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
