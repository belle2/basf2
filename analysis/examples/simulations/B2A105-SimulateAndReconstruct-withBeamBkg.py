#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to perform detector simulation and      #
# reconstruction (track finding+track fitting+ecl reconstruction+...)    #
# on a previously generated events with beam background mixing.          #
#                                                                        #
# In this example Beam Background is added. See                          #
# B2A104-SimulateAndReconstruct-withoutBeamBkg.py to check               #
# how to simulate+reconstruct without beam background.                   #
#                                                                        #
# The processed events are saved to the output ROOT file that            #
# now contain in addition to the generated particles                     #
# (MCParticle objects stored in the StoreArray<MCParticle>) also         #
# reconstructed MDST objects (Track/ECLCluster/KLMCluster/...).          #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output
import glob
import os

# create a path
my_path = b2.create_path()

# load input ROOT file
inputMdst(filename=b2.find_file('B2A101-Y4SEventGeneration-evtgen.root'), path=my_path)

# background files
# location of the files is obtained from a shell variable - check first if it is set
if 'BELLE2_BACKGROUND_DIR' not in os.environ:
    b2.B2FATAL(
        'BELLE2_BACKGROUND_DIR variable is not set. \n'
        'Please export (setenv) the variable to the location of BG overlay sample. \n'
        'Check https://confluence.desy.de/display/BI/Beam+background+samples to find them')
# get list of files and check the list length
bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
if len(bg) == 0:
    b2.B2FATAL('No files found in ', os.environ['BELLE2_BACKGROUND_DIR'])

# simulation
add_simulation(path=my_path, bkgfiles=bg)

# reconstruction
add_reconstruction(path=my_path)

# dump in MDST format
add_mdst_output(path=my_path,
                mc=True,
                filename='B2A101-Y4SEventGeneration-gsim-BKGx1.root')

# Show progress of processing
my_path.add_module('ProgressBar')

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
