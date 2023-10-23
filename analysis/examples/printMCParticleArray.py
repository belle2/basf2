#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This script loads specified ROOT file and prints the content of the MCParticle StoreArray
for each event. To be used for debugging.

Execute script with:
  $> basf2 printMCParticleArray.py -i [input_ROOT_file]
"""

# Doxygen should skip this script
# @cond

import basf2
import modularAnalysis as ma

path = basf2.Path()
ma.inputMdst(filename=basf2.find_file('mdst14.root', 'validation', True),
             path=path)
ma.printMCParticles(path=path)

# The printed information can be stored in TTree via VariablesToNtuple module
# The branch __MCDecayString__ is automatically created if printMCParticles is called
ma.variablesToNtuple(decayString="", variables=[], path=path)

basf2.process(path)

# @endcond
