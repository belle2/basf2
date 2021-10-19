#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The VariablesToHistogram module saves variables from the VariableManager
# to TH1F and TH2F here is an example of how to use it.
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import basf2
import modularAnalysis as ma  # a shorthand for the analysis tools namespace

mypath = basf2.Path()  # create a new path

# add input data and ParticleLoader modules to the path
ma.inputMdstList([basf2.find_file('analysis/tests/mdst.root')], path=mypath)
ma.fillParticleLists([('K-', 'kaonID > 0.2'), ('pi+', 'pionID > 0.2')], path=mypath)
ma.reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95', path=mypath)
ma.matchMCTruth('D0', path=mypath)

# add the VariablesToHistogram module explicitly
mypath.add_module('VariablesToHistogram',
                  particleList='D0',
                  variables=[('dM', 100, -2.0, 2.0),
                             ('isSignal', 2, -0.5, 1.5),
                             ('mcErrors', 1025, -0.5, 1024.5),
                             ('p', 1000, 0.0, 5.0),
                             ('E', 1000, 0.0, 5.0),
                             ('daughter(0, kaonID)', 100, 0.0, 1.0),
                             ('daughter(1, pionID)', 100, 0.0, 1.0)],
                  variables_2d=[('dM', 100, -2.0, 2.0, 'isSignal', 2, -0.5, 1.5),
                                ('p', 100, -2.0, 2.0, 'isSignal', 2, -0.5, 1.5),
                                ('dM', 100, -2.0, 2.0, 'mcErrors', 1025, -0.5, 1024.5)])

# you might also like to uncomment the following, and read the help for the
# convenient wrapper function:
# print(help(ma.variablesToHistogram))

# process the data
basf2.process(mypath)
print(basf2.statistics)
