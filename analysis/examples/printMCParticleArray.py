#!/usr/bin/env python3

# Doxygen should skip this script
# @cond

"""
This script loads specified ROOT file and prints the content of the MCParticle StoreArray
for each event. To be used for debugging.

Execute script with:
  $> basf2 printMCParticleArray.py -i [input_ROOT_file]
"""

import basf2
import modularAnalysis as ma

path = basf2.Path()
ma.inputMdst(environmentType='default',
             filename=basf2.find_file('mdst14.root', 'validation', True),
             path=path)
ma.printMCParticles(path=path)
basf2.process(path)

# @endcond
