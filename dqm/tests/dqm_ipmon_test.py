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
import b2test_utils as b2tu

with b2tu.clean_working_directory():
    # Set the log level to show only error and fatal messages
    # b2.set_log_level(b2.LogLevel.ERROR)
    # b2.set_log_level(b2.LogLevel.INFO)
    b2.set_log_level(b2.LogLevel.DEBUG)
    b2.set_debug_level(20)

    configs = [b2.find_file("dqm/tests/ipmon.json")]

    # Create main path
    main = b2.create_path()

    # Modules
    main.add_module('DQMHistAnalysisInputTest', Events=30, ConfigFiles=configs)
    main.add_module('DQMHistAnalysisIP', HistoDirectory="test", HistoName="ip_x", MonitorPrefix="ip")
    main.add_module('DQMHistAnalysisIP', HistoDirectory="test", HistoName="ip_y", MonitorPrefix="ip")
    main.add_module('DQMHistAnalysisIP', HistoDirectory="test", HistoName="ip_z", MonitorPrefix="ip")

    # Process all events
    b2.process(main)
