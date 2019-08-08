#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToHistogram module saves variables from the VariableManager
# to TH1F and TH2F here is an example of how to use it.
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
