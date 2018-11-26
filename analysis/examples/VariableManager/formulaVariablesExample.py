#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# An example/test of using the formula() metavariable
# note: it's the pion momentuim + the number of tracks in the event which is total nonsense
#
# James Kahn and Sam Cunliffe
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import os
import basf2
from modularAnalysis import inputMdstList
from stdCharged import stdPi

if os.path.isfile('mdst.root'):
    filename = 'mdst.root'
else:
    raise RuntimeError("Please copy an mdst file into this directory named mdst.root")

mypath = basf2.Path()  # create a path

# add input data and ParticleLoader modules to the path
inputMdstList('default', [filename], path=mypath)
stdPi('95eff', path=mypath)  # grab the standard pions list

# you can (and should) make this into something more sensible
nonsense_formula = 'formula(p+nTracks)'

# Add the VariablesToNtuple module explicitly
# this will write out one row per candidate in the pi+ list and evaluate
# the nonsense formula for all of them
mypath.add_module(
    'VariablesToNtuple',
    particleList='pi+:95eff',
    variables=['nTracks', 'p', nonsense_formula],
    fileName='formulaTest.root'
)

# process the data
basf2.process(mypath)
print(basf2.statistics)
