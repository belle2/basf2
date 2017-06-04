#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2


reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('localdb/database.txt')

main = create_path()
main.add_module('RootInput')
main.add_module('Gearbox')
main.add_module('Geometry', excludedComponents=['MagneticField'])

main.add_module('SetupGenfitExtrapolation')
main.add_module('TrackFinderMCTruthRecoTracks', WhichParticles='primary', RecoTracksStoreArrayName='CosmicRecoTracks')
main.add_module('DAFRecoFitter', recoTracksStoreArrayName='CosmicRecoTracks')

main.add_module("MillepedeCollector", minPValue=0., tracks=['CosmicRecoTracks'], components=['VXDAlignment'])

main.add_module('RootOutput', branchNames=['EventMetaData'])
main.add_module('Progress')

process(main)
print(statistics)
