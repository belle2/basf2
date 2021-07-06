#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Modules
input = b2.register_module('DQMHistAnalysisInput')
input.param('HistMemoryPath', argv[1])
main.add_module(input)

output = b2.register_module('DQMHistAnalysisOutputFile')
output.param('SaveCanvases', True)
output.param('SaveHistos', False)
main.add_module(output)

# Process all events
b2.process(main)
