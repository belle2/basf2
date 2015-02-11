#!/usr/bin/env python
# -*- coding: utf-8 -*-

############################################
# Submitting multiple jobs
############################################
# Author: The Belle II Colaboration
# Contributor: Matic Lubej (Jan 2015)
#
# This is a simple python script for the
# purpose of submitting multiple jobs. It
# creates a command string for each
# different job, prints it in the terminal
#  and evaluates it. It also creates
# a folder for the log files.
############################################

import os
import sys

if len(sys.argv) != 5:
    sys.exit('Must provide enough arguments: [# of jobs] [# of events per job] [decay file] [output file name]'
             )

noJobs = int(sys.argv[1])
noEvents = int(sys.argv[2])
decFile = sys.argv[3]
outputName = sys.argv[4]

logDirectory = 'log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

directory = 'rootFiles/'
if not os.path.exists(directory):
    os.makedirs(directory)

if outputName.endswith('.root'):
    outputName = outputName[:-5]

# in each loop create a specific command string
for x in range(0, noJobs):
    logFile = logDirectory + outputName + '-' + str(x) + '.log'
    command = 'bsub -q s -o ' + logFile \
        + ' basf2 JS101-SteeringFile-EvtGen.py ' + str(noEvents) + ' ' \
        + decFile + ' ' + directory + outputName + '-' + str(x) + '.root'
    print command  # this prints out the command
    os.system(command)  # this evaluates it in the terminal
