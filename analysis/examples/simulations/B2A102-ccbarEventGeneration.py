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
# e+e- -> ccbar -> D*+ anything event generation                         #
#                                                                        #
# This tutorial demonstrates how to generate                             #
#                                                                        #
# e+e- -> ccbar -> D*+ anything                                          #
#                                                                        #
# events with EvtGen in basf2, where the decay of D*+                    #
# is specified by the given .dec file.                                   #
#                                                                        #
# The generated events are saved to the output ROOT file.                #
# In each event the generated particles (MCParticle objects)             #
# are stored in the StoreArray<MCParticle>.                              #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import setupEventInfo
from mdst import add_mdst_output
from generators import add_inclusive_continuum_generator

# Defining one path
my_path = b2.create_path()

# generation of 100 events according to the specified DECAY table
# e+e- -> ccbar -> D*+ anything
# D*+ -> D0 pi+; D0 -> K- pi+
#
setupEventInfo(100, path=my_path)
add_inclusive_continuum_generator(finalstate="ccbar",
                                  particles=["D*+"],
                                  userdecfile=b2.find_file('analysis/examples/simulations/B2A102-ccbarEventGeneration.dec'),
                                  path=my_path)

add_mdst_output(mc=True, filename='B2A102-ccbarEventGeneration-kkmc.root', path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
