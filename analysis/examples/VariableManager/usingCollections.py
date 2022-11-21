#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The VariablesToNtuple (and VariablesToTree) module support collections of variables.
# You specify a collection instead of a variable name, and the collection will be automatically
# resolved to a set of variable names
# the Python module variables.collections defines some default collections, just import it
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import basf2
import variables
import variables.collections as vc  # collections of variables
import modularAnalysis as ma  # a shorthand for the analysis tools namespace
from variables import variables as vm  # shorthand name for the VariableManager instance

# You can also define collections yourself
vm.addCollection('MyCollection', variables.std_vector('daughter(0, kaonID)', 'daughter(1, pionID)'))

mypath = basf2.Path()  # create a new path

# add input data and ParticleLoader modules to the path
ma.inputMdstList([basf2.find_file('analysis/tests/mdst.root')], path=mypath)
ma.fillParticleLists([('K-', 'kaonID > 0.2'), ('pi+', 'pionID > 0.2')], path=mypath)
ma.reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95', path=mypath)
ma.matchMCTruth('D0', path=mypath)

# Add the VariablesToNtuple module explicitly
# this will write out one row per candidate in the D0 list
mypath.add_module('VariablesToNtuple',
                  particleList='D0',
                  variables=vc.kinematics + vc.mc_truth + ['MyCollection'],
                  fileName='CollectionVariables.root')

# you might also like to uncomment the following, and read the help for the
# convenient wrapper function:
# print(help(ma.variablesToNtuple))

# process the data
basf2.process(mypath)
print(basf2.statistics)
