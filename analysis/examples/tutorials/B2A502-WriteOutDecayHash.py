#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to write out the
# decay hash. This allows to store information on
# the reconstructed and original decay and use it
# in the offline analysis steps.
#
#   J/psi
#    |
#    +-> e- e+
#
# Notes:
#  1) No guarantee for collisions!
#
# Contributors: Moritz Gelb (June 2017)
#
######################################################

from basf2 import *
from modularAnalysis import *
from decayHash import DecayHashMap

# set the log level
set_log_level(LogLevel.WARNING)

# Bd_JpsiKL_ee Signal MC 7 file
inputFile = "/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/" \
            "prod00000628/s00/e0000/4S/r00000/signal/sub00/mdst_000001_prod00000628_task00000001.root"
inputMdstList('MC7', inputFile)


# reconstruct the decay
fillParticleList('e+', 'electronID > 0.2 and d0 < 2 and abs(z0) < 4', False)
fillParticleList('gamma', '', False)
reconstructDecay('J/psi -> e+ e-', '')

# generate the decay string
analysis_main.add_module('ParticleMCDecayString', listName='J/psi', fileName='hashmap_Jpsi.root')


# write out ntuples
var = ['M',
       'p',
       'E',
       'x', 'y', 'z',
       'extraInfo(DecayHash)',
       'extraInfo(DecayHashExtended)',
       ]

variablesToNTuple('J/psi', var, filename='Jpsi.root')

# process the events
process(analysis_main)

# print out the summary
print(statistics)
