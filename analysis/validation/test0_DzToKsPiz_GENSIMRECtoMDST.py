#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>../DzToKsPiz_GENSIMRECtoMDST.mdst.root</output>
  <contact>Giulia Casarosa; giulia.casarosa@pi.infn.it</contact>
</header>
"""

######################################################
#
# Generate D0 -> Ks pi0
# for charm validation (primary reconstruction efficiency).
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

set_random_seed(150922)

main = create_path()

# e+e- -> ccbar -> D0 anything
# D0 -> Ks pi0
#
generateContinuum(10000, 'D0', Belle2.FileSystem.findFile('/decfiles/dec/2210022100.dec'), path=main)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction
add_reconstruction(main)

# mdst output
add_mdst_output(main, True, '../DzToKsPiz_GENSIMRECtoMDST.mdst.root')

# Go!
process(main)

# Print call statistics
print(statistics)
