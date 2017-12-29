#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ######################################################
# This tutorial demonstrates how to perform detector simulation
# and reconstruction (track finding+track fitting+ecl reconstruction+...)
# on a previously generated events with beam background mixing.
#
# In this example Beam Background is added. See
# B2A103-SimulateAndReconstruct-withoutBeamBkg.py to check
# how to simulate+reconstruct without beam background.
#
# The processed events are saved to the output ROOT file that
# now contain in addition to the generated particles
# (MCParticle objects stored in the StoreArray<MCParticle>) also
# reconstructed MDST objects (Track/ECLCluster/KLMCluster/...).
# Contributors: A. Zupanc (June 2014)
#
# #####################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import analysis_main
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import glob

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    B2FATAL(
        'Required input file (B2A101-Y4SEventGeneration-evtgen.root) does not exist. \n'
        'Please run B2A101-Y4SEventGeneration.py tutorial script first.')

# load input ROOT file
inputMdst('None', 'B2A101-Y4SEventGeneration-evtgen.root')

# background files
# location of the files is obtained from a shell variable - check first if it is set
if 'BELLE2_BACKGROUND_DIR' not in os.environ:
    B2FATAL('BELLE2_BACKGROUND_DIR variable is not set. \n'
            'Please export (setenv) the variable to the location of BG overlay samples')
# get list of files and check the list length
bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
if len(bg) == 0:
    B2FATAL('No files found in ', os.environ['BELLE2_BACKGROUND_DIR'])

# simulation
add_simulation(analysis_main, bkgfiles=bg)

# reconstruction
add_reconstruction(analysis_main)

# dump in MDST format
add_mdst_output(analysis_main, True,
                'B2A101-Y4SEventGeneration-gsim-BKGx1.root')

# Show progress of processing
progress = register_module('ProgressBar')
analysis_main.add_module(progress)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
