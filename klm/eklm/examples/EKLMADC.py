#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# EKLM ADC study.

import basf2

# Set the log level to show only error and fatal messages
basf2.set_log_level(basf2.LogLevel.WARNING)

# EventInfoSetter - generate event meta data
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])

# XML reader
xmldata = basf2.register_module('Gearbox')

# EKLM ADC
eklmadc = basf2.register_module('EKLMADC')
eklmadc.param('Mode', 'Shape')
eklmadc.param('OutputFile', 'FPGA.root')
# For generation of the histograms for strips.
# eklmadc.param('Mode', 'Strips')
# eklmadc.param('OutputFile', 'EKLMADC.root')

# Create main path
main = basf2.create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(xmldata)
main.add_module(eklmadc)

# Run
basf2.process(main)
