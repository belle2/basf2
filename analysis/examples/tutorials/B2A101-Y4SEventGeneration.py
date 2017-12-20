#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# Y(4S) -> BBbar event generation
#
# This tutorial demonstrates how to generate
#
# e+e- -> Y(4S) -> BBbar
#
# events with EvtGen in BASF2, where the decay of Y(4S)
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
from modularAnalysis import generateY4S
from modularAnalysis import loadGearbox
from reconstruction import add_mdst_output
from modularAnalysis import analysis_main
from ROOT import Belle2

# generation of 100 events according to the specified DECAY table
# Y(4S) -> Btag- Bsig+
# Btag- -> D0 pi-; D0 -> K- pi+
# Bsig+ -> mu+ nu_mu
#
# generateY4S function is defined in analysis/scripts/modularAnalysis.py
generateY4S(100, Belle2.FileSystem.findFile('analysis/examples/tutorials/B2A101-Y4SEventGeneration.dec'))

# If the simulation and reconstruction is not performed in the sam job,
# then the Gearbox needs to be loaded with the loadGearbox() function.
loadGearbox()

# dump generated events in MDST format to the output ROOT file
#
# add_mdst_output function is defined in reconstruction/scripts/reconstruction.py
add_mdst_output(analysis_main, True, 'B2A101-Y4SEventGeneration-evtgen.root')

# process all modules added to the analysis_main path
# (note: analysis_main is the default path created in the modularAnapys.py)
process(analysis_main)

# print out the summary
print(statistics)
