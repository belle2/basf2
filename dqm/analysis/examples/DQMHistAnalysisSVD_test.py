#!/usr/bin/env python3

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

argv = sys.argv
if len(argv) < 2:
    print(f'\nUsage: {argv[0]} input_filename\n')
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
                SelectHistograms=["DQMInfo/rtype",
                                  "SVDExpReco/*",
                                  "SVDUnpacker/DQMUnpackerHisto",
                                  "SVDClsTrk/SVDTRK_ClusterTimeV456",
                                  "SVDEfficiency/*"],
                EventInterval=1,
                NullHistogramMode=False,
                AutoCanvas=False)
# main.add_module("DQMHistAutoCanvas") # Plot all Histo from Input not needed

main.add_module('Gearbox')
main.add_module('Geometry')

# Analysis module to calculate occupancy of each sensor and check control plots
main.add_module('DQMHistAnalysisSVDGeneral')

# Analysis module to calculate efficiency of each sensor
main.add_module('DQMHistAnalysisSVDEfficiency')

# Output canvases to root file
main.add_module('DQMHistAnalysisOutputFile',
                HistoFile="output_dqmHistAnalysis.root",
                SaveHistos=False,
                SaveCanvases=True)

# Process all event at main path
b2.process(main)

# Print modules' statistics
print(b2.statistics)
