#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# e+e- -> ccbar -> D*+ antyhing event generation
#
# This tutorial demonstrates how to generate
#
# e+e- -> ccbar -> D*+ anything
#
# events with EvtGen in BASF2, where the decay of D*+
# is specified by the given .dec file.
#
# The generated events are saved to the output ROOT file.
# In each event the generated particles (MCParticle objects)
# are stored in the StoreArray<MCParticle>.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import generateContinuum
from modularAnalysis import loadGearbox
from modularAnalysis import analysis_main
from reconstruction import add_mdst_output

# generation of 100 events according to the specified DECAY table
# e+e- -> ccbar -> D*+ anthing
# D*+ -> D0 pi+; D0 -> K- pi+
#
# generateContinuum function is defined in analysis/scripts/modularAnalysis.py
generateContinuum(100, 'D*+', 'B2A102-ccbarEventGeneration.dec')

# if simulation/reconstruction scripts are not added than one needs to load gearbox
loadGearbox()

add_mdst_output(analysis_main, True, 'B2A102-ccbarEventGeneration-evtgen.root')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
