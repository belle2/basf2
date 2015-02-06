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
# Use JS201-JobSubmissionScripts-simple.py
# to run several times.
############################################

from basf2 import *
from modularAnalysis import *
from reconstruction import add_mdst_output

import sys
import os

# input parameters: e.g. basf2 JS101-SteeringFile-EvtGen.py 100 decay.dec output.root
# this generates 100 files according to the decay.dec decay file and dumps the mdst into output.root

if len(sys.argv) != 4:
    sys.exit('Must provide enough arguments: [# of events] [decay file] [output file name]'
             )

noEvents = int(sys.argv[1])
decFile = sys.argv[2]
outputName = sys.argv[3]

directory = 'rootFiles/'
if not os.path.exists(directory):
    os.makedirs(directory)

# Generate Y(4S) events.
generateY4S(noEvents, decFile)

# If the simulation and reconstruction is not performed in the same job,
# then the Gearbox needs to be loaded with the loadGearbox() function.
loadGearbox()

add_mdst_output(analysis_main, True, directory + outputName)

# -------------------------------------------

# HERE IS WHERE YOU PUT YOUR ANALYSIS

printDataStore()

# -------------------------------------------

# process the path
process(analysis_main)

# print out the summary
print statistics
