#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################
# Example script to demonstrate the NtupleMaker module
# (inspired by myBtoDpi.py)
#
# Christian Oswald (oswald@physik.uni-bonn.de), Uni Bonn, 2013.
###########################################################

import sys

if len(sys.argv) != 3:
    # the program name and the two arguments
    # stop the program and print an error message
    sys.exit('Must provide two input parameters:[output_root_file_name] [#events_to_generate]'
             )

rootFileName = sys.argv[1]
nOfEvents = int(sys.argv[2])

import os

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from modularAnalysis import *  # analysis_main path

# ---------------------------------------------------------------
# EvtGen
# use specified user decay file
generateY4S(nOfEvents, os.environ['BELLE2_LOCAL_DIR'] +
            '/analysis/examples/exampleEvtgenDecayFiles/BtoDpi.dec')

# ---------------------------------------------------------------
# Offical simulation setup
add_simulation(analysis_main)

# ---------------------------------------------------------------
# Official reconstruction setup
add_reconstruction(analysis_main)

# ---------------------------------------------------------------
# Main part of this script starts here

# ----> Select kaons and pions
kaons = ('K+:all', '')
pions = ('pi+:all', '')

fillParticleLists([kaons, pions])

# ----> Reco D0
reconstructDecay('anti-D0:kpi -> K+:all pi-:all', '')

# ----> Reco B
reconstructDecay('B+:dpi -> anti-D0:kpi pi+:all', '')

# create and fill flat Ntuple with other information
toolsB = ['EventMetaData', 'B+']
toolsB += ['Kinematics', 'B+ -> [anti-D0 -> ^K+ ^pi-] ^pi+']
toolsB += ['MCKinematics', 'B+ -> [anti-D0 -> ^K+ ^pi-] ^pi+']

# nTuple OUTPUT
ntupleFile(rootFileName)
ntupleTree('test1', 'B+:dpi', toolsB)

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
