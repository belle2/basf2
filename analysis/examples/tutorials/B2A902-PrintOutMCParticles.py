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
# This tutorial demonstrates how to print out the                        #
# contents of the StoreArray<MCParticle>.                                #
#                                                                        #
# This is useful for debugging purposes, e.g. to check if                #
# the MC files contain the desired processes/decays.                     #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# print out the contents of the StoreArray<MCParticle>
ma.printPrimaryMCParticles(path=my_path)

# The printed information can be stored in TTree via VariablesToNtuple module
# The branch __MCDecayString__ is automatically created if printMCParticles is called
ma.variablesToNtuple(decayString="", variables=[], path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
