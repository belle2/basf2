#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('localdb/database.txt')

main = create_path()

main.add_module("EventInfoSetter")
beam.add_beamparameters(main, "Y4S")

ana.loadGearbox(main)
main.add_module('Geometry')
main.add_module('ParticleGun', pdgCodes=[13, -13])

sim.add_simulation(main)
reco.add_reconstruction(main, pruneTracks=False)

ana.fillParticleList('mu+:bbmu', 'muid > 0.1 and useLabFrame(p) > 0.5', True, main)

main.add_module("Progress")
main.add_module("RootOutput")


# generate events
process(main)

# show call statistics
print(statistics)
