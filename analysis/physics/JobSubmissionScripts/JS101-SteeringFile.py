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

from basf2 import *
from modularAnalysis import generateY4S
from modularAnalysis import analysis_main
from modularAnalysis import loadGearbox
from reconstruction import add_mdst_output

import sys
import os

fileNo = int(sys.argv[1])

if len(sys.argv) != 2:
    sys.exit('Must provide an argument: file number')

directory = 'rootFiles/'

if not os.path.exists(directory):
    os.makedirs(directory)

# Generate 100 events with Y(4S) particle. generateY4S function is defined in analysis/scripts/modularAnalysis.py
generateY4S(100)

# If the simulation and reconstruction is not performed in the same job,
# then the Gearbox needs to be loaded with the loadGearbox() function.
loadGearbox()

add_mdst_output(analysis_main, True, directory + 'JS101-EvtGenData_'
                + str(fileNo) + '.root')

# process the path
process(analysis_main)

# print out the summary
print statistics
