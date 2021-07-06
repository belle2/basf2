# !/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# script to test module which create monitoring variables for SVD
#
# Usage: basf2 DQMHistAnalysisSVDMiraBelle_test.py input_files
#   input_files: one of the raw dqm files placed under /group/belle2/phase3/dqm/dqmsrv1/
#
# i.e basf2 DQMHistAnalysisSVDMiraBelle_test.py /group/belle2/phase3/dqm/dqmsrv1/e0018/dqmhisto/erecodqm_e0018r001313.root
# Output file in format: mon_e0018r001313_online.root contains moniotring variables which can be pinted by script:
# dqm/analysis/examples/printDQMFile.C

import basf2 as b2
import os
import sys
import re

argv = sys.argv
if len(argv) < 2:
    print('\nUsage: %s input_filename\n' % argv[0])
    exit(1)
inputFile = sys.argv[1]

directory, fileName = os.path.split(inputFile)
exp_nr = int(re.findall(r'\d+', fileName)[0])
run_nr = int(re.findall(r'\d+', fileName)[1])

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
                SelectHistograms=["SVDExpReco/*",
                                  "SVDClsTrk/*",
                                  "SVDEfficiency/*"],
                EventInterval=1,
                NullHistogramMode=False,
                AutoCanvas=False)

# Run analysis module
main.add_module('DQMHistAnalysisSVDOnMiraBelle')

# Save canvases based on histograms used to prepare monitoring variables and monitoring variables to root file
main.add_module('DQMHistAnalysisOutputMonObj',
                exp=exp_nr,
                run=run_nr)

# Process the events
b2.print_path(main)
b2.process(main)

# print out the summary
print(b2.statistics)
