#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform detector simulation
# and reconstruction (track finding+track fitting+ecl reconstruction+...)
# on a previously generated events.
#
# See simulation examples for more details
#
# The processed events are saved to the output ROOT file that
# now contain in addition to the generated particles
# (MCParticle objects stored in the StoreArray<MCParticle>) also
# reconstructed MDST objects (Track/ECLCluster/KLMCluster/...).
# Contributors: A. Zupanc (June 2014) I. Komarov (2018)
#
######################################################
import basf2 as b2
import mdst as mdst
import simulation as si
import reconstruction as re
import modularAnalysis as ma

# check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/for_B2A102.root'):
    b2.B2FATAL("You need the example data installed. Run `b2install-example-data` in terminal for it.")

# create path
my_path = ma.analysis_main

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B2A102.root',
             path=my_path)

# In case of conflict with geometry, you may use this line instead:
# analysis_main.add_module("RootInput", inputFileName='B2A101-Y4SEventGeneration-evtgen.root')

# simulation
si.add_simulation(path=my_path)

# reconstruction
re.add_reconstruction(path=my_path)

# dump in MDST format
mdst.add_mdst_output(path=my_path,
                     mc=True,
                     filename='from_B2A102.root')

# Show progress of processing
progress = b2.register_module('ProgressBar')
my_path.add_module(progress)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
