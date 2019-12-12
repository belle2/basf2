#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run KKMC to generate tautau events
#
# Example steering file
########################################################

import basf2
from modularAnalysis import fillParticleListFromMC, reconstructDecay, variablesToNtuple, inputMdst

# main path
main = basf2.create_path()

inputMdst('default', 'utrepsbpipi_100k_B.root', path=main)

fillParticleListFromMC('pi+:MC', 'mcPrimary', path=main)
reconstructDecay('gamma:Virtual -> pi+:MC pi-:MC', '', path=main)

variablesToNtuple('gamma:Virtual', ['M', 'E', 'daughter(0, cosTheta)', 'daughter(1, cosTheta)'], path=main)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
