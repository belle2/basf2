#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# script to analyze raw histograms from TOP
#
# Usage: basf2 DQMHistAnalysisTOP_test.py input_files
#   input_files: one of the raw dqm files placed under /group/belle2/phase3/dqm/dqmsrv1/
#
# i.e basf2 DQMHistAnalysisTOP_test.py /group/belle2/phase3/dqm/dqmsrv1/e0018/dqmhisto/erecodqm_e0018r001313.root
# -----------------------------------------------------------------------------------------------


import basf2 as b2
import sys

argv = sys.argv
if len(argv) < 2:
    print('\nUsage: %s input_filename\n' % argv[0])
    exit(1)
inputFile = sys.argv[1]

# Set log level
b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Load histograms from file
main.add_module('DQMHistAnalysisInputRootFile',
                Experiment=0,
                RunList=[0],
                FileList=[inputFile],
                EventsList=[1],
                SelectHistograms=["DQMInfo/rtype", "DAQ/Nevent", "TOP/*"],
                EventInterval=1,
                NullHistogramMode=False)

# Plot all histograms from the file
main.add_module("DQMHistAutoCanvas")

# Analysis module
main.add_module('DQMHistAnalysisTOP')

# Output canvases to root file
main.add_module('DQMHistAnalysisOutputFile',
                HistoFile="output_dqmHistAnalysis.root",
                SaveHistos=False,
                SaveCanvases=True)

# Process all event at main path
b2.process(main)

# Print modules' statistics
print(b2.statistics)
