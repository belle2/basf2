#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The VariablesToNtuple module saves variables from the VariableManager
# to a candidate-based (or event-based) TTree
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

# Add the VariablesToNtuple module explicitly.
# This will write out one row per candidate in the D0 list
mypath.add_module('VariablesToNtuple',
                  particleList='D0',
                  variables=['dM', 'isSignal', 'mcErrors', 'p', 'E',
                             'daughter(0, kaonID)', 'daughter(1, pionID)'],
                  fileName='CandidateVariables.root')

# Add another instance of the VariablesToNtuple module.
# If the particle list is empty one row per event is written to the Ntuple,
# but all the variables to specify have to be event-based
mypath.add_module('VariablesToNtuple',
                  particleList='',
                  variables=['nTracks', 'isMC', 'year'],
                  fileName='EventVariables.root')

# One can also call the VariablesToNtuple module in the roe_path to store the ROE particles' variables.
# Build ROE and append a mask for cleanup
ma.buildRestOfEvent('D0', path=mypath)
cleanMask = ('cleanMask',  # mask name
             'nCDCHits > 0 and useCMSFrame(p)<=3.2',  # criteria on tracks
             'p >= 0.05 and useCMSFrame(p)<=3.2'  # criteria on ECL-clusters
             )
ma.appendROEMasks('D0', mask_tuples=[cleanMask], path=mypath)

# Make another path that is called in for_each loop over ROE objects
roe_path = basf2.Path()
ma.fillParticleList('pi+:inRoe', 'isInRestOfEvent > 0 and passesROEMask(cleanMask)', path=roe_path)
ma.variablesToNtuple(decayString='pi+:inRoe',
                     variables=['p', 'E'],
                     filename='CandidateVariables.root',
                     treename='roe',
                     signalSideParticleList='D0',  # index of D0 is stored in the branch '__signalSideCandidate__'
                     path=roe_path)
mypath.for_each('RestOfEvent', 'RestOfEvents', roe_path)


# you might also like to uncomment the following, and read the help for the
# convenient wrapper function:
# print(help(ma.variablesToNtuple))

# process the data
basf2.process(mypath)
print(basf2.statistics)
