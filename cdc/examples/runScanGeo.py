#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import register_module, create_path, process

# Create main path
main = create_path()

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
evtruninfo = {'expList': [0], 'runList': [1], 'evtNumList': [1]}
eventinfosetter.param(evtruninfo)
main.add_module(eventinfosetter)

# Geometry parameter loader
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry builder
geometry = register_module('Geometry')
main.add_module(geometry)

# ScanGeometry module
scangeo = register_module('ScanCDCGeo')
main.add_module(scangeo)

# Process one event
process(main)
