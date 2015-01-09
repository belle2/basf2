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
# generates 100 empty events and saves them.
############################################

import os
import sys

# specify the mcType (charged/mixed/..) and background (BGx0/BGx1)
mcType = sys.argv[1]
bkg = sys.argv[2]

# create a log directory
logDirectory = 'log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

# x and y to loop over all stream (s00-s49)
for x in range(0, 1):
    for y in range(0, 1):
        stream = 's' + str(x) + str(y)
        # z to run over all groups of files of 100 (300-399, 400-499, ...)
        for z in range(0, 10):
            logFile = logDirectory + '/data-' + mcType + '_mc35_' + bkg + '_' \
                + stream + '-' + str(z) + '.log'
            command = 'bsub -q l -o ' + logFile \
                + ' basf2 JS101-SteeringFile.py ' + mcType + ' ' + str(bkg) \
                + ' ' + stream + ' ' + str(z)
            print command
            os.system(command)

