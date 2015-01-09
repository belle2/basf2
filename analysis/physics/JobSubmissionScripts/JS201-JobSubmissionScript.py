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

logDirectory = 'log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

for x in range(0, 10):
    logFile = logDirectory + 'JS202-JobSubmissionScript-' + str(x) + '.log'
    command = 'bsub -q s -o ' + logFile + ' basf2 JS101-SteeringFile.py ' \
        + str(x)
    print command  # this prints out the command
    os.system(command)  # this evaluates it in the terminal
