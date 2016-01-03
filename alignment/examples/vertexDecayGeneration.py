#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.INFO)

from ROOT import Belle2

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

main = create_path()

main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)
beam.add_beamparameters(main, "Y4S")

ana.loadGearbox(main)
main.add_module('Geometry')

kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

main.add_module("Progress")
main.add_module(kkgeninput)

sim.add_simulation(main)
reco.add_mc_reconstruction(main)

ana.fillParticleList('mu+:good', 'muid > 0.1 and useLabFrame(p) > 2.', True, main)

ana.reconstructDecay('Z0:mumu -> mu-:good mu+:good', '', writeOut=True, path=main)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')

# ana.matchMCTruth('Z0:mumu', main)
# ana.matchMCTruth('mu+:good', main)

ana.printVariableValues('Z0:mumu', ['E', 'deltaE', 'M', 'mcPDG'], path=main)

main.add_module("RootOutput")


# generate events
process(main)

# show call statistics
print(statistics)
