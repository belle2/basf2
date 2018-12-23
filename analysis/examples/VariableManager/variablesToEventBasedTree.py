#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToEventBasedTree module saves variables from the VariableManager
# to an event-based TTree
#
# Thomas Keck and Sam Cunliffe
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import os
import basf2
import modularAnalysis as ma  # a shorthand for the analysis tools namespace

if os.path.isfile('mdst.root'):
    filename = 'mdst.root'
else:
    raise RuntimeError("Please copy an mdst file into this directory named mdst.root")

mypath = basf2.Path()  # create a new path

# add input data and ParticleLoader modules to the path
ma.inputMdstList('default', [filename], path=mypath)
ma.fillParticleLists([('K-', 'kaonID > 0.2'), ('pi+', 'pionID > 0.2')], path=mypath)
ma.reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95', path=mypath)
ma.matchMCTruth('D0', path=mypath)

# This will write out one row per event in the TTree
# The branches of the TTree are arrays containing the variables for each D0 candidate in the event
# The event_variables are handled differently, the branches of these event_variables are just floats,
# you can use this to write out candidate independent information (aka event-based variables)
mypath.add_module('VariablesToEventBasedTree',
                  particleList='D0',
                  variables=['dM', 'isSignal', 'mcErrors', 'p', 'E',
                             'daughter(0, kaonID)', 'daughter(1, pionID)'],
                  event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])

# process the data
basf2.process(mypath)
print(basf2.statistics)
