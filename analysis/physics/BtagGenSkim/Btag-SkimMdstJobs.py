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

if len(sys.argv) != 3:
    sys.exit('Must provide enough arguments: [input directory] [0/1]')

inputDir = sys.argv[1]
recLong = int(sys.argv[2])

logDirectory = 'log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

rootDirectory = 'rootFiles/'
if not os.path.exists(rootDirectory):
    os.makedirs(rootDirectory)

if not inputDir.endswith('/'):
    inputDir = inputDir + '/'

# in each loop create a specific command string
for fileName in os.listdir(inputDir):

    if fileName.endswith('.root'):
        logName = fileName[:-5]

    logFile = logDirectory + 'Skimmed-' + logName + '.log'
    command = 'bsub -q l -o ' + logFile + ' basf2 Btag-SkimMdst.py ' \
        + inputDir + fileName + ' ' + rootDirectory + 'Skimmed-' + fileName + ' ' \
        + str(recLong)
    print command  # this prints out the command
    os.system(command)  # this evaluates it in the terminal
