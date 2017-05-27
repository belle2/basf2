#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)

"""
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('database.txt')
"""

from ROOT import Belle2

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

main = create_path()

main.add_module("EventInfoSetter")
beam.add_beamparameters(main, "Y4S")

ana.loadGearbox(main)
main.add_module('Geometry')
main.add_module('EvtGenInput')

sim.add_simulation(main)
reco.add_reconstruction(main, pruneTracks=False)

ana.fillParticleList('mu+:good', 'muid > 0.5 and useLabFrame(p) > 0.5', True, main)

main.add_module("Progress")
main.add_module("RootOutput")


# generate events
process(main)

# show call statistics
print(statistics)
