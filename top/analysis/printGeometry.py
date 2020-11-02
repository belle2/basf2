#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2
import sys

# ---------------------------------------------------------------------------
# Prints all geometry parameters of TOP
#
# Usage:  basf2 top/analysis/printGeometry.py
#
# Note: print-out is about 2000 lines long
#       pipe it to less or redirect the output to a file
#
# The filename of the main xml file can be specified as an optional argument
#   example: basf2 top/analysis/printGeometry.py top/TOPSpareModule.xml
#            (hint: omit 'data' from the path)
# ---------------------------------------------------------------------------

# Suppress messages and warnings for all other packages
b2.set_log_level(b2.LogLevel.ERROR)

# set DEBUG level for top only to print mappings
b2.logging.set_package('top', b2.LogConfig(b2.LogLevel.DEBUG, 10000))

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
argvs = sys.argv
if len(argvs) > 1:
    fileName = argvs[1]
    gearbox.param('fileName', fileName)
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Process events
b2.process(main)
