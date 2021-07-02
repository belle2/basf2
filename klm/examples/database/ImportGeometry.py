#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Create KLM geometry payloads.
# Note that GeoConfiguration created by this script should not be used except
# for testing, because it contains the KLM only.

import basf2

basf2.set_log_level(basf2.LogLevel.INFO)

# Create main path
main = basf2.create_path()

# Event-info setter
eventinfosetter = basf2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# Gearbox
gearbox = basf2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = basf2.register_module('Geometry')
geometry.param('components', ['KLM'])
geometry.param('useDB', False)
geometry.param('createPayloads', True)
geometry.param('payloadIov', [0, 0, -1, -1])
main.add_module(geometry)

# Process the path
basf2.process(main)
