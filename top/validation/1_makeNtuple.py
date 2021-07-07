#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>EvtGenSimRec.root, EvtGenSimRec_B2Kpi.root</input>
  <output>TOPNtuple.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Makes a flat ntuple with TOP PID, track info and MC truth </description>
</header>
"""

import basf2 as b2

# ---------------------------------------------------------------
# Make a flat ntuple for validation of top package
# ---------------------------------------------------------------

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Input
roinput = b2.register_module('RootInput')
roinput.param('inputFileNames', ['../EvtGenSimRec_B2Kpi.root',
                                 '../EvtGenSimRec.root'])
main.add_module(roinput)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP is needed and B-field)
geometry = b2.register_module('Geometry')
geometry.param('components', ['MagneticField', 'TOP'])
main.add_module(geometry)

# Output: make flat ntuple from TOPLikelihoods, tracking info and MC truth
output = b2.register_module('TOPNtuple')
output.param('outputFileName', '../TOPNtuple.root')
main.add_module(output)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
