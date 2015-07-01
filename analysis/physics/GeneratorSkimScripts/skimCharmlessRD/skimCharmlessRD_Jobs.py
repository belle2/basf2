#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

if len(sys.argv) != 6:
    sys.exit(
        'Must provide arguments: [# of jobs] [# of events/job] [output dir] [output filename without ".root"] [sim & rec ?: 1/0]')

nJobs = int(sys.argv[1])
nEvents = int(sys.argv[2])
outputDir = sys.argv[3]
outputName = sys.argv[4]
simrec = sys.argv[5]

# Log directory for log files
logDirectory = 'log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

# Clear log directory
os.system('rm -rf log/*')

# Submit jobs
for x in range(1, nJobs + 1):

    # Define log file directory and name
    logFile = logDirectory + 'charmlessRD-' + str(nEvents) + '-' + str(x) + '.log'
    # Define command string
    command = 'bsub -q s -o ' + logFile + ' basf2 skimCharmlessRD.py ' + \
        str(nEvents) + ' ' + outputDir + ' ' + outputName + '-' + str(x) + '.root' + ' ' + simrec
    # Print out the command
    print command
    # Evaluate the command
    os.system(command)
