#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# script to analyze raw histograms from DQM modules
#
# Usage: basf2 DQMHistAnalysisSVD_test.py input_files
#   input_files: one of the raw dqm files placed under /group/belle2/phase3/dqm/dqmsrv1/
#
# i.e basf2 DQMHistAnalysisSVD_test.py /group/belle2/phase3/dqm/dqmsrv1/e0018/dqmhisto/erecodqm_e0018r001313.root
# -----------------------------------------------------------------------------------------------


import basf2 as b2
import sys
import re

argv = sys.argv
if len(argv) < 2:
    print('\nUsage: %s input_filename\n' % argv[0])
    exit(1)
inputFile = sys.argv[1]
fileOut = sys.argv[2]

exp_nr = int(re.findall(r'\d+', inputFile)[0])
run_nr = int(re.findall(r'\d+', inputFile)[1])

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
                RunType='physics',
                EventInterval=1)

main.add_module('Gearbox')
main.add_module('Geometry')

# enable EPICS
main.add_module('DQMHistAnalysisEpicsEnable')

# Analysis module to calculate unpacker error
main.add_module('DQMHistAnalysisSVDUnpacker',
                samples3=False)

# Analysis module to calculate occupancy of each sensor and check control plots
main.add_module('DQMHistAnalysisSVDOccupancy',
                samples3=False)

# Analysis module to calculate cluster time on tracks
main.add_module('DQMHistAnalysisSVDClustersOnTrack',
                samples3=False)

# Analysis module to calculate efficiency of each sensor
main.add_module('DQMHistAnalysisSVDEfficiency',
                samples3=False)

# mirabelle
main.add_module('DQMHistAnalysisSVDOnMiraBelle')

# Save canvases based on histograms used to prepare monitoring variables and monitoring variables to root file
main.add_module('DQMHistAnalysisOutputMonObj',
                exp=exp_nr,
                run=run_nr)

# Dose analysis
# main.add_module('DQMHistAnalysisSVDDose')

# injection
# main.add_module('DQMHistInjection')

# Output canvases to root file
main.add_module('DQMHistAnalysisOutputFile',
                OutputFolder="./",
                Filename=fileOut,
                SaveHistos=False,
                SaveCanvases=True)

# Process all event at main path
b2.process(main, calculateStatistics=True)

# Print modules' statistics
print(b2.statistics)
