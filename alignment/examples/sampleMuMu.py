#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('localdb/database.txt')

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

main = create_path()

main.add_module("EventInfoSetter")
beam.add_beamparameters(main, "Y4S")

ana.loadGearbox(main)
main.add_module('Geometry')

# main.add_module('BeamParameters', createPayload=True)

kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

main.add_module(kkgeninput)

sim.add_simulation(main)
reco.add_reconstruction(main, pruneTracks=False)

ana.fillParticleList('mu+:qed', 'muid > 0.1 and useLabFrame(p) > 2.', True, main)

ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=main)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')

ana.matchMCTruth('Z0:mumu', main)
ana.matchMCTruth('mu+:good', main)

ana.printVariableValues('Z0:mumu', ['E', 'deltaE', 'M', 'mcPDG'], path=main)
ana.printVariableValues('mu+:qed', ['E', 'p', 'mcPDG'], path=main)
ana.printVariableValues('mu-:qed', ['E', 'p', 'mcPDG'], path=main)

main.add_module("Progress")
main.add_module("RootOutput")

process(main)
print(statistics)
