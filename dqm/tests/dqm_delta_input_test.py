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
# b2.set_log_level(b2.LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Modules
main.add_module('DQMHistAnalysisInputTest', Events=30)
main.add_module('DQMHistAnalysisDeltaTest', useEpics=False)
# Process all events
b2.process(main)