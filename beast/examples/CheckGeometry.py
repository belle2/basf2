#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file creates the Belle II detector
# geometry and checks for overlaps
######################################################

import basf2 as b2

evtmeta = b2.register_module('EventInfoSetter')

# Geometry parameter loader
gearbox = b2.register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase1.xml')

# Geometry builder
geometry = b2.register_module('Geometry')

# Overlap checker
overlapchecker = b2.register_module('OverlapChecker')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(evtmeta)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(overlapchecker)

# Process one event
b2.process(main)
