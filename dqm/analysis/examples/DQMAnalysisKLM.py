#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

# Set log level.
basf2.set_log_level(basf2.LogLevel.INFO)

# Create main path.
main = basf2.create_path()

# Input module.
main.add_module('DQMHistAnalysisInputRootFile',
                Experiment=0,
                RunList=[0],
                EventsList=[1],
                EventInterval=1,
                FileList=['KLMDQM.root'],
                SelectHistograms=['KLM/*', 'DAQ/*'])

# KLM DQM analysis module.
main.add_module('DQMHistAnalysisKLM')

# Output module.
main.add_module('DQMHistAnalysisOutputFile',
                HistoFile='KLMDQMAnalysed.root',
                SaveHistos=False,
                SaveCanvases=True)

# Process main path.
basf2.process(main)

# Print modules' statistics.
print(basf2.statistics)
