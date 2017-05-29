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

if len(sys.argv) < 4:
    print('Usage: basf2 1_generate.py experiment_number run_number num_events')
    sys.exit(1)

experiment = int(sys.argv[1])
run = int(sys.argv[2])
nevents = int(sys.argv[3])

main = create_path()

main.add_module("EventInfoSetter", expList=[experiment], runList=[run], evtNumList=[nevents])
beam.add_beamparameters(main, "Y4S")

ana.loadGearbox(main)
main.add_module('Geometry')

# main.add_module('BeamParameters', createPayload=True)

kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

main.add_module("Progress")
main.add_module(kkgeninput)

sim.add_simulation(main)
reco.add_mc_reconstruction(main, pruneTracks=False)

ana.fillParticleList('mu+:good', 'muid > 0.1 and useLabFrame(p) > 2.', True, main)

ana.reconstructDecay('Z0:mumu -> mu-:good mu+:good', '', writeOut=True, path=main)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')

# ana.matchMCTruth('Z0:mumu', main)
# ana.matchMCTruth('mu+:good', main)

# ana.printVariableValues('Z0:mumu', ['E', 'deltaE', 'M', 'mcPDG'], path=main)

main.add_module("RootOutput")


# generate events
process(main)

# show call statistics
print(statistics)
