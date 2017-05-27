#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

# set_random_seed(101)
# set_log_level(LogLevel.INFO)

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Cosmics')
main.add_module('Gearbox')
main.add_module('Geometry', excludedComponents=['MagneticField'])

main.add_module('FullSim')

main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')
main.add_module('CDCDigitizer')
main.add_module('SetupGenfitExtrapolation')

main.add_module('TrackFinderMCTruthRecoTracks', WhichParticles='primary', RecoTracksStoreArrayName='CosmicRecoTracks')
main.add_module('DAFRecoFitter', recoTracksStoreArrayName='CosmicRecoTracks')

main.add_module('RootOutput')
main.add_module('Progress')

process(main)
print(statistics)
