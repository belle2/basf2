#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
# use_local_database('localdb/database.txt')

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

main = create_path()

main.add_module("EventInfoSetter")
beam.add_beamparameters(main, "Y4S")
main.add_module('Gearbox')
main.add_module('Geometry')

kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

main.add_module(kkgeninput)
sim.add_simulation(main)
main.add_module("Progress")
main.add_module("RootOutput")
process(main)
print(statistics)
