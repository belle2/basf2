#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2

from beamparameters import add_beamparameters
from simulation import add_simulation
from L1trigger import add_tsim

kkgeninput = basf2.register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

main = basf2.create_path()

main.add_module("EventInfoSetter")
# add_beamparameters(main, "Y4S")
main.add_module('Gearbox')
main.add_module('Geometry')

main.add_module(kkgeninput)

add_simulation(main)
add_tsim(main)

main.add_module("RootOutput")
main.add_module("Progress")

basf2.process(main)
print(basf2.statistics)
