#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# An example/test of using the formula() metavariable
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import basf2
from modularAnalysis import inputMdstList
from stdCharged import stdPi

mypath = basf2.Path()  # create a path

# add input data and ParticleLoader modules to the path
inputMdstList([basf2.find_file('analysis/tests/mdst.root')], path=mypath)
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
