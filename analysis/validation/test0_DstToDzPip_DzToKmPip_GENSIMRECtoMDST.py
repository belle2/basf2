#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>../DstToDzPip_DzToKmPip_GENSIMRECtoMDST.mdst.root</output>
  <contact>Giulia Casarosa; giulia.casarosa@pi.infn.it</contact>
</header>
"""

######################################################
#
# Generate D* -> D0 pi; D0 -> K pi to get K/pi samples
# for charm validation (primary proper time resolution).
# no machine bacgkround
#
# Contributor: Giulia Casarosa
# April 20, 2017
#
######################################################

from basf2 import *
from modularAnalysis import generateContinuum
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from ROOT import Belle2
import glob

set_random_seed(1509)

main = create_path()

# e+e- -> ccbar -> D*+ anthing
# D*+ -> D0 pi+; D0 -> K- pi+
#
generateContinuum(1000, 'D*+', Belle2.FileSystem.findFile('/decfiles/dec/2610030000.dec'), path=main)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction
add_reconstruction(main)

# mdst output
add_mdst_output(main, True, '../DstToDzPip_DzToKmPip_GENSIMRECtoMDST.mdst.root')

# Go!
process(main)

# Print call statistics
print(statistics)
