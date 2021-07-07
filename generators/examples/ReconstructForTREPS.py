#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from modularAnalysis import fillParticleListFromMC, reconstructDecay, variablesToNtuple, inputMdst

# main path
main = basf2.create_path()

# load input mdst file
inputMdst('default', 'utrepsbpipi_100k.root', path=main)

# get pi+:MC from MCParticles
fillParticleListFromMC('pi+:MC', 'mcPrimary', path=main)

# reconstruct a virtual gamma for convenience
reconstructDecay('gamma:Virtual -> pi+:MC pi-:MC', '', path=main)

# store variables of gamma:Virtual to ntuple
variablesToNtuple('gamma:Virtual', ['M', 'E', 'daughter(0, cosTheta)', 'daughter(1, cosTheta)'], path=main)

# process
basf2.process(main)

# show call statistics
print(basf2.statistics)
