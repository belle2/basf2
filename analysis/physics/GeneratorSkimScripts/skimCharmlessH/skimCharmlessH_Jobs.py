#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

if len(sys.argv) != 5:
    sys.exit(
        'Must provide enough arguments: [# of jobs] [# of events per job] [output directory] [output file name without ".root"]')

nJobs = int(sys.argv[1])
nEvents = int(sys.argv[2])
outputDir = sys.argv[3]
outputName = sys.argv[4]

# Log directory for log files
logDirectory = 'log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

# Clear log directory
os.system('rm -rf log/*')

# Submit jobs
for x in range(1, nJobs + 1):

    # Define log file directory and name
    logFile = logDirectory + 'charmlessH-' + str(nEvents) + '-' + str(x) + '.log'
    # Define command string
    command = 'bsub -q s -o ' + logFile + ' basf2 skimCharmlessH.py ' + \
        str(nEvents) + ' ' + outputDir + ' ' + outputName + '-' + str(x) + '.root'
    # Print out the command
    print command
    # Evaluate the command
    os.system(command)
