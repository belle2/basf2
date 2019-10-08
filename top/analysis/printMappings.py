#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys

# ---------------------------------------------------------------------------
# Prints mapping of TOP electronic channels to pixels and to front end mapping
# e.g. scrods to slot/boardstacks
#
# Usage:  basf2 top/analysis/printMappings.py
#
# The filename of the main xml file can be specified as an optional argument
#   example: basf2 top/analysis/printMappings.py top/TOPSpareModule.xml
#            (hint: omit 'data' from the path)
# ---------------------------------------------------------------------------

# Suppress messages and warnings for all other packages
set_log_level(LogLevel.ERROR)

# set DEBUG level for top only to print mappings
logging.set_package('top', LogConfig(LogLevel.DEBUG, 100))

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
argvs = sys.argv
if len(argvs) > 1:
    fileName = argvs[1]
    gearbox.param('fileName', fileName)
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Process events
process(main)
